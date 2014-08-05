/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Classes/UI/hierarchytreecontrol.h"
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include "HierarchyTreeController.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "ItemsCommand.h"
#include "CommandsController.h"
#include "CopyPasteController.h"
#include "SubcontrolsHelper.h"

#define TREE_MIME_DATA 0
#define ITEM_ID 0, Qt::UserRole

HierarchyTreeControlMimeData::HierarchyTreeControlMimeData(const QList<QTreeWidgetItem*> items)
{
	for (QList<QTreeWidgetItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		QTreeWidgetItem* item = (*iter);
		QVariant data = item->data(ITEM_ID);
		HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
		
		Logger::Debug("HierarchyTreeNode::HIERARCHYTREENODEID %d", id);
		
		HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator it = std::find(this->items.begin(), this->items.end(), id);
		if (it == this->items.end())
			this->items.push_back(id);
	}
}

HierarchyTreeControlMimeData::~HierarchyTreeControlMimeData()
{
	
}

bool HierarchyTreeControlMimeData::IsDropEnable(const HierarchyTreeNode *parentItem) const
{
	const HierarchyTreePlatformNode* parentPlatform = dynamic_cast<const HierarchyTreePlatformNode*>(parentItem);
	const HierarchyTreeScreenNode* parentScreen = dynamic_cast<const HierarchyTreeScreenNode*>(parentItem);
	const HierarchyTreeControlNode* parentControl = dynamic_cast<const HierarchyTreeControlNode*>(parentItem);
	const HierarchyTreeRootNode* rootNode = dynamic_cast<const HierarchyTreeRootNode*>(parentItem);
	
	for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		HierarchyTreeNode::HIERARCHYTREENODEID id = (*iter);
		
		const HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(id);
		
		if (node->IsHasChild(parentItem))
			return false;
		
		if (parentPlatform)
		{
			//all items must be screen
			const HierarchyTreeScreenNode* screen = dynamic_cast<const HierarchyTreeScreenNode*>(node);
			if (!screen)
				return false;
		}
		else if (parentScreen || parentControl)
		{
			//all items must be control
			const HierarchyTreeControlNode* control = dynamic_cast<const HierarchyTreeControlNode*>(node);
			if (!control)
				return false;
		}
		else if (rootNode)
		{
			const HierarchyTreePlatformNode* platform = dynamic_cast<const HierarchyTreePlatformNode*>(node);
			if (!platform)
				return false;
		}
	}
	
	return true;
}

HierarchyTreeNode::HIERARCHYTREENODESIDLIST HierarchyTreeControlMimeData::GetItems() const
{
	return items;
}

HierarchyTreeControl::HierarchyTreeControl(QWidget *parent) :
    QTreeWidget(parent),
    expandNodeItem(NULL),
    expandCheckMousePos(false)
{
	setAcceptDrops(true);
	setAutoScroll(true);
	setDropIndicatorShown(true);
    
    expandTimer = new QTimer();
    expandTimer->setSingleShot(true);
    
    static const int expandInterval = 1000; //ms
    expandTimer->setInterval(expandInterval);
    connect(expandTimer, SIGNAL(timeout()), this, SLOT(OnExpandTimer()));
}

HierarchyTreeControl::~HierarchyTreeControl()
{
    StopExpandTimer();
    disconnect(expandTimer, SIGNAL(timeout()), this, SLOT(OnExpandTimer()));
    SafeDelete(expandTimer);
}

void HierarchyTreeControl::contextMenuEvent(QContextMenuEvent * event)
{
	emit ShowCustomMenu(event->globalPos());
}

Vector<int32> HierarchyTreeControl::GetPositionKey(QTreeWidgetItem* item) const
{
    Vector<int32> positionKey;

    for(int i = 0; i < topLevelItemCount(); ++i)
	{
		if (item == topLevelItem(i))
		{
	        positionKey.push_back(i + 1);
            break;
		}
	}

    QTreeWidgetItem* curItem = item;
    QTreeWidgetItem* curItemParent = item->parent();
    while (curItemParent)
    {
        int idx = curItemParent->indexOfChild(curItem) + 1;
        positionKey.push_back(idx);
        
        curItem = curItemParent;
        curItemParent = curItemParent->parent();
    }

	return positionKey;
}

bool HierarchyTreeControl::SortByInternalIndex(const SortedItems &first, const SortedItems &second)
{
    const Vector<int32>& firstKey = first.positionKey;
    const Vector<int32>& secondKey = second.positionKey;

    if (firstKey.size() != secondKey.size())
    {
        return firstKey.size() < secondKey.size();
    }

    // Both items are on the same level of hierarchy, so compare pos-by-pos.
    // Note - The array was built in the "child-to-parent position" way, compare from the end.
    uint32 levelsCount = firstKey.size();
    for (uint32 i = 0; i < levelsCount; i ++)
    {
        if (firstKey[i] == secondKey[i])
        {
            // Compare the next hierarchy level.
            continue;
        }

        return (firstKey[i] < secondKey[i]);
    }

	return false;
}

QMimeData* HierarchyTreeControl::mimeData(const QList<QTreeWidgetItem*> items) const
{
	std::list<SortedItems> sortedItems;
	
	for (QList<QTreeWidgetItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		QTreeWidgetItem* item = (*iter);
		sortedItems.push_back(SortedItems(item, GetPositionKey(item)));
	}
	
	sortedItems.sort(SortByInternalIndex);
	QList<QTreeWidgetItem* > qSortedItems;
	for (std::list<SortedItems>::iterator iter = sortedItems.begin(); iter != sortedItems.end(); ++iter)
	{
		qSortedItems.push_back(iter->item);
	}
	
	QMimeData* data = QTreeWidget::mimeData(qSortedItems);
	data->setUserData(TREE_MIME_DATA, new HierarchyTreeControlMimeData(qSortedItems));
	return data;
}

void HierarchyTreeControl::dropEvent(QDropEvent *event)
{
	QTreeWidgetItem* item = itemAt(event->pos());
	if (!item)
		return;
	
	// What are we dropping?
	const ControlMimeData* controlMimeData = dynamic_cast<const ControlMimeData*>(event->mimeData());
	if (controlMimeData)
	{
		HandleDropControlMimeData(event, controlMimeData);
		return;
	}
	
	const HierarchyTreeControlMimeData* hierarchyMimeData = dynamic_cast<const HierarchyTreeControlMimeData*>(event->mimeData()->userData(TREE_MIME_DATA));
	if (hierarchyMimeData)
	{
		HandleDropHierarchyMimeData(event, hierarchyMimeData);
		return;
	}
}

void HierarchyTreeControl::HandleDropControlMimeData(QDropEvent *event, const ControlMimeData* mimeData)
{
    HierarchyTreeNode::HIERARCHYTREENODEID insertInTo = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode::HIERARCHYTREENODEID insertAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	if (!GetMoveItemID(event, insertInTo, insertAfter))
		return;
	
    HierarchyTreeNode* parentNode = HierarchyTreeController::Instance()->GetTree().GetNode(insertInTo);
    HierarchyTreeNode* insertAfterNode = HierarchyTreeController::Instance()->GetTree().GetNode(insertAfter);
	if (dynamic_cast<HierarchyTreePlatformNode*>(parentNode) ||
		dynamic_cast<HierarchyTreeAggregatorControlNode*>(parentNode))
	{
		return;
	}
    
	if (!parentNode)
	{
        return;
	}
    
    StopExpandTimer();
    
    CreateControlCommand* cmd = new CreateControlCommand(mimeData->GetControlId(), parentNode, insertAfterNode);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void HierarchyTreeControl::HandleDropHierarchyMimeData(QDropEvent *event, const HierarchyTreeControlMimeData* mimeData)
{
	HierarchyTreeNode::HIERARCHYTREENODEID insertInTo = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode::HIERARCHYTREENODEID insertAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	if (!GetMoveItemID(event, insertInTo, insertAfter))
		return;
	
	HierarchyTreeNode* parentNode = HierarchyTreeController::Instance()->GetTree().GetNode(insertInTo);
	if (!parentNode)
	{
		parentNode = (HierarchyTreeNode*) HierarchyTreeController::Instance()->GetTree().GetRootNode();
		insertInTo = parentNode->GetId();
	}
	
	if (!mimeData->IsDropEnable(parentNode))
	{
		return;
	}
		
	//Copy current selected item(s) if ctrl key is pressed during drag
	if (event->keyboardModifiers() == Qt::ControlModifier)
	{
		CopyPasteController::Instance()->CopyControls(HierarchyTreeController::Instance()->GetActiveControlNodes());
		CopyPasteController::Instance()->Paste(parentNode);
	}
	else //Otherwise move item(s)
	{
		HierarchyTreeNode::HIERARCHYTREENODESIDLIST items = mimeData->GetItems();
		ChangeNodeHeirarchy* cmd = new ChangeNodeHeirarchy(insertInTo, insertAfter, items);
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}
 
void HierarchyTreeControl::dragEnterEvent(QDragEnterEvent *event)
{
	QTreeWidget::dragEnterEvent(event);
	
	if (!event->mimeData())
		return;

	// Check what is arriving - Library Control or another Hierarchy Tree node.
	const ControlMimeData* controlMimeData = dynamic_cast<const ControlMimeData*>(event->mimeData());
	if (controlMimeData)
	{
		HandleDragEnterControlMimeData(event, controlMimeData);
		return;
	}
	
	const HierarchyTreeControlMimeData* hierarchyMimeData = dynamic_cast<const HierarchyTreeControlMimeData*>(event->mimeData()->userData(TREE_MIME_DATA));
	if (hierarchyMimeData)
	{
		HandleDragEnterHierarchyMimeData(event, hierarchyMimeData);
		return;
	}
	
	// Not ours...
	event->ignore();
}

void HierarchyTreeControl::dragLeaveEvent(QDragLeaveEvent */*event*/)
{
    StopExpandTimer();
}

void HierarchyTreeControl::HandleDragEnterControlMimeData(QDragEnterEvent *event, const ControlMimeData* /*mimeData*/)
{
    if (currentItem())
    {
        setCurrentItem(NULL);
    }

    event->accept();
}

void HierarchyTreeControl::HandleDragEnterHierarchyMimeData(QDragEnterEvent *event, const HierarchyTreeControlMimeData* mimeData)
{
	HierarchyTreeNode * selectedTreeNode = HierarchyTreeController::Instance()->GetTree().GetNode(*(mimeData->GetItems().begin()));
	HierarchyTreeControlNode * selectedControlNode =  dynamic_cast<HierarchyTreeControlNode*>(selectedTreeNode);

	HierarchyTreeAggregatorNode * aggregatorNode =  dynamic_cast<HierarchyTreeAggregatorNode*>(selectedControlNode);
	if (aggregatorNode)
	{
		// Don't allow to drop anything to aggregator.
		return;
	}

	if(selectedControlNode)
	{
		if(SubcontrolsHelper::ControlIsSubcontrol(selectedControlNode->GetUIObject()))
		{
			return;
		}
	}

	event->accept();
}

void HierarchyTreeControl::dragMoveEvent(QDragMoveEvent *event)
{
	QTreeWidget::dragMoveEvent(event);
	event->ignore();

	if (!event->mimeData())
	{
		return;
	}

	// What are we dragging?
	const ControlMimeData* controlMimeData = dynamic_cast<const ControlMimeData*>(event->mimeData());
	if (controlMimeData)
	{
		HandleDragMoveControlMimeData(event, controlMimeData);
		return;
	}
	
	const HierarchyTreeControlMimeData* hierarchyMimeData = dynamic_cast<const HierarchyTreeControlMimeData*>(event->mimeData()->userData(TREE_MIME_DATA));
	if (hierarchyMimeData)
	{
		HandleDragMoveHierarchyMimeData(event, hierarchyMimeData);
		return;
	}
}

void HierarchyTreeControl::HandleDragMoveControlMimeData(QDragMoveEvent *event, const ControlMimeData* /*mimeData*/)
{
    DropIndicatorPosition position = dropIndicatorPosition();
    Logger::Warning("POSITION TYPE^ %i", position);

	// Where we are in tree?
	QTreeWidgetItem* item = itemAt(event->pos());
	if (!item)
	{
		HierarchyTreeController::Instance()->ResetSelectedControl();
		return;
	}

	HierarchyTreeNode::HIERARCHYTREENODEID insertInto = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	QVariant data = item->data(ITEM_ID);
	insertInto = data.toInt();
	
	// Handle specific types of nodes.
	HierarchyTreeNode* nodeToInsertControlTo = HierarchyTreeController::Instance()->GetTree().GetNode(insertInto);
	if (dynamic_cast<HierarchyTreePlatformNode*>(nodeToInsertControlTo) ||
		dynamic_cast<HierarchyTreeAggregatorControlNode*>(nodeToInsertControlTo))
	{
		// Don't allow to drop the controls directly to Platform or Aggregator.
		HierarchyTreeController::Instance()->ResetSelectedControl();
		return;
	}

    // Don't allow to drop the control to the screen which isn't loaded.
    HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(nodeToInsertControlTo);
    if (screenNode && !screenNode->IsLoaded())
    {
        return;
    }

    scrollTo(indexAt(event->pos()));
    StartExpandTimer(item, true);

	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(nodeToInsertControlTo);
	if (controlNode)
	{
		// Don't reselect the same control, if it is already selected.
		if (!HierarchyTreeController::Instance()->IsControlSelected(controlNode))
		{
			HierarchyTreeController::Instance()->ResetSelectedControl();
			HierarchyTreeController::Instance()->SelectControl(controlNode, HierarchyTreeController::DeferredExpandWithMouseCheck);
		}
	}

	event->accept();
}

void HierarchyTreeControl::HandleDragMoveHierarchyMimeData(QDragMoveEvent *event, const HierarchyTreeControlMimeData* mimeData)
{
	HierarchyTreeNode::HIERARCHYTREENODEID insertInTo = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode::HIERARCHYTREENODEID insertAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	if (!GetMoveItemID(event, insertInTo, insertAfter))
		return;
	
	HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(insertInTo);
	if (!node)
	{
		node = (HierarchyTreeNode*) HierarchyTreeController::Instance()->GetTree().GetRootNode();
	}

    // Don't allow to drop something to the screen which isn't loaded.
    HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(node);
    if (screenNode && !screenNode->IsLoaded())
    {
        return;
    }

	HierarchyTreeAggregatorControlNode* aggregatorControlNode = dynamic_cast<HierarchyTreeAggregatorControlNode*>(node);
	if (aggregatorControlNode)
	{
		// Don't allow to drop controls to aggregator controls.
		return;
	}

	if (mimeData->IsDropEnable(node))
	{
		event->accept();
	}
}

bool HierarchyTreeControl::GetMoveItemID(QDropEvent *event, HierarchyTreeNode::HIERARCHYTREENODEID &insertInTo, HierarchyTreeNode::HIERARCHYTREENODEID &insertAfter)
{
	DropIndicatorPosition position = dropIndicatorPosition();
	
	QTreeWidgetItem* item = itemAt(event->pos());
	if (!item)
		return false;

	if (item == currentItem())
		return false;

	insertInTo = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	
	switch (position)
	{
		case OnViewport:
		{
			return false;
		}break;
		case OnItem:
		{
			QVariant data = item->data(ITEM_ID);
			insertInTo = data.toInt();
			insertAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
		} break;
		case AboveItem:
		{
			QTreeWidgetItem* parent = item->parent();
			if (parent)
				insertInTo = parent->data(ITEM_ID).toInt();
			QTreeWidgetItem* above = itemAbove(item);
			if (!above)
				return false;
			insertAfter = above->data(ITEM_ID).toInt();
		} break;
		case BelowItem:
		{
			QTreeWidgetItem* parent = item->parent();
			if (parent)
				insertInTo = parent->data(ITEM_ID).toInt();
			insertAfter = item->data(ITEM_ID).toInt();
		}break;
	}
	
	if (currentItem() && currentItem()->data(ITEM_ID) == insertAfter)
		return false;

	return true;
}

void HierarchyTreeControl::StartExpandTimer(QTreeWidgetItem* nodeItem, bool needCheckMousePos)
{
    expandTimer->stop();
    expandNodeItem = nodeItem;
    expandCheckMousePos = needCheckMousePos;
    expandTimer->start();
}


void HierarchyTreeControl::StopExpandTimer()
{
    expandNodeItem = NULL;
    expandTimer->stop();
}

void HierarchyTreeControl::OnExpandTimer()
{
    if (!expandNodeItem)
    {
        // Nothing to expand.
        return;
    }

    if (!expandCheckMousePos)
    {
        // Just expand the item selected, without any checks.
        ExpandItemAndScrollTo(expandNodeItem);
        return;
    }
    
    // Perform the check - what is under mouse cursor.
    QModelIndex modelIndex = indexAt(mapFromGlobal(QCursor::pos()));
    if (!modelIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* selectedItem = itemFromIndex(modelIndex);
    if (selectedItem == expandNodeItem)
    {
        ExpandItemAndScrollTo(selectedItem);
    }
}

void HierarchyTreeControl::ExpandItemAndScrollTo(QTreeWidgetItem* item)
{
    item->setExpanded(true);
    scrollToItem(item);

    QTreeWidgetItem* parentItem = item->parent();
    while (parentItem)
    {
        parentItem->setExpanded(true);
        parentItem = parentItem->parent();
    }
}
