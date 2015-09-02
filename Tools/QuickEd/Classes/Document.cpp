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


#include "Document.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"

#include "Ui/QtModelPackageCommandExecutor.h"
#include "EditorCore.h"
#include "UI/IconHelper.h"
#include "UI/MainWindow.h"
#include "QtTools/DavaGLWidget/DavaGLWidget.h"

Document::Document(PackageNode *_package, QObject *parent)
    : QObject(parent)
    , package(SafeRetain(_package))
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
    , selectionSystem(this)
    , canvasSystem(this)
    , hudSystem(this)
    , cursorSystem(this)
    , transformSystem(this)
{
    systems << &selectionSystem << &canvasSystem << &hudSystem << &cursorSystem << &transformSystem;
    inputListeners << &hudSystem << &selectionSystem << &transformSystem;
    selectionSystem.SelectionWasChanged.Connect(this, &Document::OnSelectionWasChanged);
    selectionSystem.SelectionWasChanged.Connect(&canvasSystem, &CanvasSystem::OnSelectionWasChanged);
    selectionSystem.SelectionWasChanged.Connect(&hudSystem, &HUDSystem::OnSelectionWasChanged);
    selectionSystem.SelectionWasChanged.Connect(&transformSystem, &TransformSystem::OnSelectionWasChanged);
    hudSystem.AddListener(&cursorSystem);
    hudSystem.AddListener(&transformSystem);
    hudSystem.SelectionRectChanged.Connect(&selectionSystem, &SelectionSystem::SelectByRect);
    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::RefreshAllControlProperties);
}

Document::~Document()
{
    for (auto context : contexts)
    {
        delete context.second;
    }
}

using namespace DAVA;

void Document::Detach()
{
    for (auto system : systems)
    {
        system->Detach();
    }
    emit SelectedNodesChanged(SelectedNodes(), selectedNodes);
}

void Document::Attach()
{
    for (auto system : systems)
    {
        system->Attach();
    }
    emit SelectedNodesChanged(selectedNodes, SelectedNodes());
}

CanvasSystem* Document::GetCanvasSystem()
{
    return &canvasSystem;
}

HUDSystem* Document::GetHUDSystem()
{
    return &hudSystem;
}

const FilePath &Document::GetPackageFilePath() const
{
    return package->GetPath();
}

void Document::RefreshLayout()
{
    package->RefreshPackageStylesAndLayout(true);
}

WidgetContext* Document::GetContext(QObject* requester) const
{
    auto iter = contexts.find(requester);
    if (iter!= contexts.end())
    {
        return iter->second;
    }
    return nullptr;
}

void Document::SetContext(QObject* requester, WidgetContext* widgetContext)
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        DVASSERT_MSG(false, "document already have this context");
        delete iter->second;
        contexts.erase(iter);
    }
    contexts.insert(std::pair<QObject*, WidgetContext*>(requester, widgetContext));
}

void Document::OnSelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected)
{
    SelectedNodes reallySelected(selected.begin(), selected.end());
    SelectedNodes reallyDeselected(deselected.begin(), deselected.end());
    for (auto control : deselected)
    {
        if (selectedNodes.find(control) == selectedNodes.end())
        {
            reallyDeselected.erase(control);
        }
    }
    SubstractSets(reallyDeselected, selectedNodes);

    for (auto control : selected)
    {
        if (selectedNodes.find(control) != selectedNodes.end())
        {
            reallySelected.erase(control);
        }
    }
    UniteSets(reallySelected, selectedNodes);
    SetSelectedNodes(reallySelected, reallyDeselected);
}

bool Document::OnInput(UIEvent *currentInput)
{
    QListIterator<InputInterface*> it(inputListeners);
    it.toBack();
    bool forUpdate = false;
    while (it.hasPrevious())
    {
        if (it.previous()->OnInput(currentInput, forUpdate))
        {
            forUpdate = true;
        }
    }
    return false;
}

void Document::GetControlNodesByPos(DAVA::Vector<ControlNode*> &controlNodes, const DAVA::Vector2& pos) const
{
    auto controlsNode = package->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        auto tmpNode = controlsNode->Get(index);
        DVASSERT(nullptr != tmpNode);
        GetControlNodesByPosImpl(controlNodes, pos, tmpNode);
    }
}

void Document::GetControlNodesByRect(Set<ControlNode*>& controlNodes, const Rect& rect) const
{
    auto controlsNode = package->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        auto tmpNode = controlsNode->Get(index);
        DVASSERT(nullptr != tmpNode);
        GetControlNodesByRectImpl(controlNodes, rect, tmpNode);
    }
}

void Document::GetControlNodesByPosImpl(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos, ControlNode* node) const
{
    int count = node->GetCount();
    auto control = node->GetControl();
    if (control->IsPointInside(pos) && control->GetVisible() && control->GetVisibleForUIEditor())
    {
        controlNodes.push_back(node);
    }
    for (int i = 0; i < count; ++i)
    {
        GetControlNodesByPosImpl(controlNodes, pos, node->Get(i));
    }
}

void Document::GetControlNodesByRectImpl(Set<ControlNode*>& controlNodes, const Rect& rect, ControlNode* node) const
{
    int count = node->GetCount();
    auto control = node->GetControl();
    if (control->GetVisible() && control->GetVisibleForUIEditor() && rect.RectContains(control->GetGeometricData().GetAABBox()))
    {
        controlNodes.insert(node);
    }
    for (int i = 0; i < count; ++i)
    {
        GetControlNodesByRectImpl(controlNodes, rect, node->Get(i));
    }
}

AbstractProperty* Document::GetPropertyByName(const ControlNode *node, const String &name) const
{
    RootProperty *propertiesRoot = node->GetRootProperty();
    int propertiesCount = propertiesRoot->GetCount();
    for (int index = 0; index < propertiesCount; ++index)
    {
        auto rootProperty = propertiesRoot->GetProperty(index);
        if (nullptr != rootProperty)
        {
            int sectionCount = rootProperty->GetCount();
            for (int prop = 0; prop < sectionCount; ++prop)
            {
                AbstractProperty *valueProperty = rootProperty->GetProperty(prop);
                if (nullptr != valueProperty && valueProperty->GetName() == name)
                {
                    return valueProperty;
                }
            }
        }
    }
    return nullptr;
}

void Document::SelectControlByPos(const Vector<ControlNode*> &nodesUnderPoint, const Vector2 &point)
{
    auto view = EditorCore::Instance()->GetMainWindow()->GetGLWidget();
    QPoint globalPos = view->mapToGlobal(QPoint(point.x, point.y));
    QMenu menu;
    QList<QAction*> actions;
    QAction *defaultAction = nullptr;
    for (auto it = nodesUnderPoint.rbegin(); it != nodesUnderPoint.rend(); ++it)
    {
        ControlNode *controlNode = *it;
        QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
        QIcon icon(IconHelper::GetIconPathForClassName(className));
        QAction *action = new QAction(icon, QString::fromStdString(controlNode->GetName()), &menu);
        menu.addAction(action);
        void* ptr = static_cast<void*>(*it);
        action->setData(QVariant::fromValue(ptr));
        actions << action;
        if (defaultAction == nullptr)
        {
            if (selectedNodes.find(*it) != selectedNodes.end())
            {
                defaultAction = action;
            }
        }
    }
    menu.setDefaultAction(defaultAction);
    QAction *selectedAction = menu.exec(globalPos);
    if (nullptr != selectedAction)
    {
        SelectedNodes deselected = selectedNodes;
        SelectedNodes selected;
        void *ptr = selectedAction->data().value<void*>();
        ControlNode *selectedNode = static_cast<ControlNode*>(ptr);
        selected.insert(selectedNode);
        SetSelectedNodes(selected, deselected);
    }
}

void Document::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
}

void Document::OnSelectedNodesChanged(const SelectedNodes &selected, const SelectedNodes &deselected)
{
    SetSelectedNodes(selected, deselected);
}

void Document::SetSelectedNodes(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectedNodes reallySelected = selected;
    SelectedNodes reallyDeselected = deselected;
    for (auto node : deselected)
    {
        if (selectedNodes.find(node) == selectedNodes.end())
        {
            reallyDeselected.erase(node);
        }
    }
    SubstractSets(reallyDeselected, selectedNodes);

    for (auto node : selected)
    {
        if (selectedNodes.find(node) != selectedNodes.end())
        {
            reallySelected.erase(node);
        }
    }
    UniteSets(reallySelected, selectedNodes);
    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        SelectedControls selectedControls;
        SelectedControls deselectedControls;
        for (auto node : reallySelected)
        {
            ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
            if (nullptr != controlNode)
            {
                selectedControls.insert(controlNode);
            }
        }
        for (auto node : reallyDeselected)
        {
            ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
            if (nullptr != controlNode)
            {
                deselectedControls.insert(controlNode);
            }
        }
        if (!selectedControls.empty() || !deselectedControls.empty())
        {
            selectionSystem.OnSelectionWasChanged(selectedControls, deselectedControls);
        }
        emit SelectedNodesChanged(reallySelected, reallyDeselected);
    }
}
