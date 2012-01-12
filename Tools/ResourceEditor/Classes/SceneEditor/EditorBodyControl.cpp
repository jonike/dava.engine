#include "EditorBodyControl.h"

#include "ControlsFactory.h"

#include "OutputManager.h"
#include "OutputPanelControl.h"

#include "../BeastProxy.h"

#include "../SceneNodeUserData.h"

EditorBodyControl::EditorBodyControl(const Rect & rect)
    :   UIControl(rect)
	, beastManager(0)
{
    scene = NULL;
    
    selectedSceneGraphNode = NULL;
    selectedDataGraphNode = NULL;
    savedTreeCell = 0;
    
    for(int32 i = 0; i < EDNID_COUNT; ++i)
    {
        dataNodes[i] = NULL;
    }
    
    ControlsFactory::CusomizeBottomLevelControl(this);


    CreateLeftPanel();
    

    scene3dView = new UI3DView(Rect(
                            ControlsFactory::LEFT_SIDE_WIDTH + SCENE_OFFSET, 
                            SCENE_OFFSET, 
                            rect.dx - ControlsFactory::LEFT_SIDE_WIDTH - ControlsFactory::RIGHT_SIDE_WIDTH - 2 * SCENE_OFFSET, 
                            rect.dy - 2 * SCENE_OFFSET - OUTPUT_PANEL_HEIGHT));
    scene3dView->SetDebugDraw(true);
    scene3dView->SetInputEnabled(false);
    AddControl(scene3dView);
    
    
    
    CreateScene(true);

    outputPanel = new OutputPanelControl(scene, Rect(
                                              ControlsFactory::LEFT_SIDE_WIDTH, 
                                              rect.dy - OUTPUT_PANEL_HEIGHT, 
                                              rect.dx - ControlsFactory::LEFT_SIDE_WIDTH - ControlsFactory::RIGHT_SIDE_WIDTH, 
                                              OUTPUT_PANEL_HEIGHT));
    ControlsFactory::CustomizePanelControl(outputPanel);
    AddControl(outputPanel);

    
    
    CreatePropertyPanel();
	
	CreateModificationPanel();
}


EditorBodyControl::~EditorBodyControl()
{
    ReleaseModificationPanel();
    
    ReleasePropertyPanel();

    SafeRelease(outputPanel);
    
    ReleaseScene();
  
    SafeRelease(scene3dView);

    ReleaseLeftPanel();
}

void EditorBodyControl::CreateLeftPanel()
{
    Rect fullRect = GetRect();
    
    Rect leftRect = Rect(0, 0, ControlsFactory::LEFT_SIDE_WIDTH, fullRect.dy);
    leftPanelSceneGraph = ControlsFactory::CreatePanelControl(leftRect);
    AddControl(leftPanelSceneGraph);
    
    Rect sceneGraphRect = leftRect;
    sceneGraphRect.dy -= (ControlsFactory::BUTTON_HEIGHT * 4);
    sceneGraphTree = new UIHierarchy(sceneGraphRect);
    ControlsFactory::CusomizeListControl(sceneGraphTree);
    ControlsFactory::SetScrollbar(sceneGraphTree);
    sceneGraphTree->SetCellHeight(CELL_HEIGHT);
    sceneGraphTree->SetDelegate(this);
    sceneGraphTree->SetClipContents(true);
    leftPanelSceneGraph->AddControl(sceneGraphTree);
    
    int32 y = sceneGraphRect.dy;
    UIButton * refreshSceneGraphButton = ControlsFactory::CreateButton(Rect(
                                                                      0, y, ControlsFactory::LEFT_SIDE_WIDTH,ControlsFactory::BUTTON_HEIGHT), 
                                                                 L"Refresh");
    y += ControlsFactory::BUTTON_HEIGHT;
    
    UIButton * lookAtButton = ControlsFactory::CreateButton(Rect(
                                                                 0, y, ControlsFactory::LEFT_SIDE_WIDTH,ControlsFactory::BUTTON_HEIGHT), 
                                                            L"Look At Object");
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * removeNodeButton = ControlsFactory::CreateButton(Rect(
                                                                     0, y, ControlsFactory::LEFT_SIDE_WIDTH, ControlsFactory::BUTTON_HEIGHT), 
                                                                L"Remove Object");
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * enableDebugFlagsButton = ControlsFactory::CreateButton(Rect(
                                                                           0, y, ControlsFactory::LEFT_SIDE_WIDTH, ControlsFactory::BUTTON_HEIGHT), 
                                                                      L"Debug Flags");
    
    refreshSceneGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRefreshSceneGraph));
    lookAtButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnLookAtButtonPressed));
    removeNodeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRemoveNodeButtonPressed));
    enableDebugFlagsButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnEnableDebugFlagsPressed));
    
    leftPanelSceneGraph->AddControl(refreshSceneGraphButton);
    leftPanelSceneGraph->AddControl(lookAtButton);
    leftPanelSceneGraph->AddControl(removeNodeButton);
    leftPanelSceneGraph->AddControl(enableDebugFlagsButton);
    
    SafeRelease(refreshSceneGraphButton);
    SafeRelease(lookAtButton);
    SafeRelease(removeNodeButton);
    SafeRelease(enableDebugFlagsButton);
    
    
    
    Rect dataGraphRect = leftRect;
    dataGraphRect.dy -= (ControlsFactory::BUTTON_HEIGHT);
    leftPanelDataGraph = ControlsFactory::CreatePanelControl(leftRect);
    dataGraphTree = new UIHierarchy(dataGraphRect);
    ControlsFactory::CusomizeListControl(dataGraphTree);
    ControlsFactory::SetScrollbar(dataGraphTree);
    dataGraphTree->SetCellHeight(CELL_HEIGHT);
    dataGraphTree->SetDelegate(this);
    dataGraphTree->SetClipContents(true);
    leftPanelDataGraph->AddControl(dataGraphTree);
    UIButton * refreshDataGraphButton = ControlsFactory::CreateButton(Rect(
                                                                      0, dataGraphRect.dy, ControlsFactory::LEFT_SIDE_WIDTH,ControlsFactory::BUTTON_HEIGHT), 
                                                                 L"Refresh");
    refreshDataGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRefreshDataGraph));
    leftPanelDataGraph->AddControl(refreshDataGraphButton);
    SafeRelease(refreshDataGraphButton);
}

void EditorBodyControl::ReleaseLeftPanel()
{
    SafeRelease(sceneGraphTree);
    SafeRelease(leftPanelSceneGraph);
    
    SafeRelease(dataGraphTree);
    SafeRelease(leftPanelDataGraph);
}


void EditorBodyControl::CreateScene(bool withCameras)
{
    scene = new EditorScene();
    // Camera setup
    cameraController = new WASDCameraController(40);
    
    if(withCameras)
    {
        Camera * cam = new Camera(scene);
        cam->SetName("editor-camera");
        cam->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
        cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
        
        cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
        
        scene->AddNode(cam);
        scene->AddCamera(cam);
        scene->SetCurrentCamera(cam);
        cameraController->SetCamera(cam);
        
        SafeRelease(cam);
        
        Camera * cam2 = new Camera(scene);
        cam2->SetName("editor-top-camera");
        cam2->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL);
        cam2->SetUp(Vector3(1.0f, 0.0f, 0.0f));
        cam2->SetPosition(Vector3(0.0f, 0.0f, 200.0f));
        cam2->SetTarget(Vector3(0.0f, 250.0f, 0.0f));
        
        cam2->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
        
        scene->AddNode(cam2);
        scene->AddCamera(cam2);
        
        SafeRelease(cam2);
    }
    
    scene3dView->SetScene(scene);
}

void EditorBodyControl::ReleaseScene()
{
    //TODO: need to release root nodes?
    
    SafeRelease(scene);
    SafeRelease(cameraController);
}


static const wchar_t * mods[3] = { L"M", L"R", L"S"};
static const wchar_t * axises[3] = { L"X", L"Y", L"Z"};

#define BUTTON_W 20 
#define BUTTON_B 5 

void EditorBodyControl::CreateModificationPanel(void)
{
	modState = MOD_MOVE;
	modAxis = AXIS_X;
	
	modificationPanel = ControlsFactory::CreatePanelControl(Rect(scene3dView->GetRect(true).x, 5, 120, 45));
    modificationPanel->GetBackground()->SetColor(Color(1.0, 1.0, 1.0, 0.2));

	for (int i = 0; i < 3; i++)
	{
		btnMod[i] = ControlsFactory::CreateButton(Rect((BUTTON_W + BUTTON_B) * i, 0, BUTTON_W, BUTTON_W), mods[i]);
		btnMod[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnModificationPressed));
		modificationPanel->AddControl(btnMod[i]);

		btnAxis[i] = ControlsFactory::CreateButton(Rect((BUTTON_W + BUTTON_B) * i, BUTTON_W + BUTTON_B, BUTTON_W, BUTTON_W), axises[i]);
		btnAxis[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnModificationPressed));
		modificationPanel->AddControl(btnAxis[i]);
	}
	UIStaticText * st = new UIStaticText(Rect(55, 0, 80, BUTTON_W));
    st->SetFont(ControlsFactory::GetFontLight());
	st->SetText(L"w, e, r");
    modificationPanel->AddControl(st);

	st = new UIStaticText(Rect(55, BUTTON_W + BUTTON_B, 80, BUTTON_W));
    st->SetFont(ControlsFactory::GetFontLight());
	st->SetText(L"5, 6, 7, 8");
    modificationPanel->AddControl(st);
	
	
	UpdateModState();
}

void EditorBodyControl::ReleaseModificationPanel()
{
	for (int i = 0; i < 3; i++)
	{
		SafeRelease(btnMod[i]);
		SafeRelease(btnAxis[i]);
	}
	SafeRelease(modificationPanel);
}

void EditorBodyControl::OnModificationPressed(BaseObject * object, void * userData, void * callerData)
{
	for (int i = 0; i < 3; i++)
	{
		if (object == btnMod[i])
		{
			modState = (eModState)i;
		}
		if (object == btnAxis[i])
		{
			modAxis = (eModAxis)i;
		}
	}
	UpdateModState();
}


void EditorBodyControl::UpdateModState(void)
{
	for (int i = 0; i < 3; i++)
	{
		btnMod[i]->SetState(UIControl::STATE_NORMAL);
		btnAxis[i]->SetState(UIControl::STATE_NORMAL);
	}
	btnMod[modState]->SetState(UIControl::STATE_SELECTED);

	switch (modAxis) 
	{
	case AXIS_X:
	case AXIS_Y:
	case AXIS_Z:
		btnAxis[modAxis]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_XY:
		btnAxis[AXIS_X]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Y]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_YZ:
		btnAxis[AXIS_Y]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Z]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_XZ:
		btnAxis[AXIS_X]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Z]->SetState(UIControl::STATE_SELECTED);
		break;
	default:
		break;
	}
}


void EditorBodyControl::CreatePropertyPanel()
{
    Rect fullRect = GetRect();
    Rect propertyPanelRect = Rect(fullRect.dx - ControlsFactory::RIGHT_SIDE_WIDTH, 0, ControlsFactory::RIGHT_SIDE_WIDTH, size.y);
    rightPanel = ControlsFactory::CreatePanelControl(propertyPanelRect);
    AddControl(rightPanel);

    refreshButton = ControlsFactory::CreateButton(Rect(
                                            0, propertyPanelRect.dy - ControlsFactory::BUTTON_HEIGHT, 
                                            propertyPanelRect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                            L"Refresh");
    refreshButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditorBodyControl::OnRefreshPressed));
    
    rightPanel->AddControl(refreshButton);
    
    
    propertyPanelRect.x = propertyPanelRect.y = 0;
    propertyPanelRect.dy -= ControlsFactory::BUTTON_HEIGHT;

    nodesPropertyPanel = new NodesPropertyControl(propertyPanelRect, false);
    nodesPropertyPanel->SetDelegate(this);
}

void EditorBodyControl::ReleasePropertyPanel()
{
    SafeRelease(refreshButton);

    SafeRelease(nodesPropertyPanel);
    
    SafeRelease(rightPanel);
}

bool EditorBodyControl::IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode)
{
    if(forHierarchy == sceneGraphTree)
    {
        if (forNode) 
        {
            SceneNode *node = (SceneNode*)forNode;
            if(node->GetSolid())
            {
                return false;
            }
            else
            {
                return node->GetChildrenCount() > 0;
            }
        }
        
        return scene->GetChildrenCount() > 0;
    }
    else if(forHierarchy == dataGraphTree)
    {
        if (forNode) 
        {
            return ((DataNode*)forNode)->GetChildrenCount() > 0;
        }
        
        for(int32 i = 0; i < EDNID_COUNT; ++i)
        {
            if(dataNodes[i]) return true;
        }
    }
    
    return false;
}

int32 EditorBodyControl::ChildrenCount(UIHierarchy *forHierarchy, void *forParent)
{
    if(forHierarchy == sceneGraphTree)
    {
        if (forParent) 
        {
            SceneNode *node = (SceneNode*)forParent;
            if(node->GetSolid())
            {
                return 0;
            }
            else
            {
                return node->GetChildrenCount();
            }

        }
        
        return scene->GetChildrenCount();
    }
    else if(forHierarchy == dataGraphTree)
    {
        if (forParent) 
        {
            return ((DataNode*)forParent)->GetChildrenCount();
        }
        
        int32 count = 0;
        for(int32 i = 0; i < EDNID_COUNT; ++i)
        {
            if(dataNodes[i]) ++count;
        }

        return count;
    }

    return 0;
}

void * EditorBodyControl::ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index)
{
    if(forHierarchy == sceneGraphTree)
    {
        if (forParent) 
        {
            return ((SceneNode*)forParent)->GetChild(index);
        }
        
        return scene->GetChild(index);
    }
    else if(forHierarchy == dataGraphTree)
    {
        if (forParent) 
        {
            return ((DataNode*)forParent)->GetChild(index);
        }
        
        int32 newIndex = 0;
        for(int32 i = 0; i < EDNID_COUNT; ++i)
        {
            if(dataNodes[i])
            {
                if(index == newIndex)
                {
                    return dataNodes[i];
                }
                
                ++newIndex;
            }
        }
    }
    
    return NULL;
}

UIHierarchyCell * EditorBodyControl::CellForNode(UIHierarchy *forHierarchy, void *node)
{
    UIHierarchyCell *c = NULL;
    if(forHierarchy == sceneGraphTree)
    {
        c = forHierarchy->GetReusableCell("SceneGraph cell"); //try to get cell from the reusable cells store
        if(!c)
        { //if cell of requested type isn't find in the store create new cell
            c = new UIHierarchyCell(Rect(0, 0, ControlsFactory::LEFT_SIDE_WIDTH, CELL_HEIGHT), "SceneGraph cell");
        }
        
        //fill cell whith data
        SceneNode *n = (SceneNode *)node;
        c->text->SetText(StringToWString(n->GetName()));
        if(n == selectedSceneGraphNode)
        {
            c->SetSelected(true, false);
            savedTreeCell = c;
        }
        else
        {
            c->SetSelected(false, false);
        }
    }
    else if(forHierarchy == dataGraphTree)
    {
        c = forHierarchy->GetReusableCell("DataGraph cell"); //try to get cell from the reusable cells store
        if(!c)
        { //if cell of requested type isn't find in the store create new cell
            c = new UIHierarchyCell(Rect(0, 0, ControlsFactory::LEFT_SIDE_WIDTH, CELL_HEIGHT), "DataGraph cell");
        }
        
        //fill cell whith data
        DataNode *n = (DataNode *)node;
        c->text->SetText(StringToWString(n->GetName()));
        if(n == selectedDataGraphNode)
        {
            c->SetSelected(true, false);
        }
        else
        {
            c->SetSelected(false, false);
        }
    }
    
    ControlsFactory::CustomizeExpandButton(c->openButton);
    ControlsFactory::CustomizeSceneGraphCell(c);

    return c;//returns cell
}

void EditorBodyControl::OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell)
{
    savedTreeCell = selectedCell;

    if(forHierarchy == sceneGraphTree)
    {
        UIHierarchyNode * hNode = selectedCell->GetNode();
        SceneNode * node = dynamic_cast<SceneNode*>((BaseObject*)hNode->GetUserNode());
        if (node)
        {
            selectedSceneGraphNode = node;
            scene->SetSelection(node);
            
            UpdatePropertyPanel();
            DebugInfo();
        }
    }
    else if(forHierarchy == dataGraphTree)
    {
        UIHierarchyNode * hNode = selectedCell->GetNode();
        DataNode * node = dynamic_cast<DataNode*>((BaseObject*)hNode->GetUserNode());
        if (node)
        {
            selectedDataGraphNode = node;
//            scene->SetSelection(node);
//            
            UpdatePropertyPanel();
//            DebugInfo();
        }
    }
    
    //select 
    List<UIControl*> children = forHierarchy->GetVisibleCells();
    for(List<UIControl*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        UIControl *ctrl = (*it);
        ctrl->SetSelected(false, false);
    }
    
    selectedCell->SetSelected(true, false);
}

void EditorBodyControl::DebugInfo()
{
    MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode*>(selectedSceneGraphNode);
    if(mesh)
    {
        AABBox3 bbox = mesh->GetBoundingBox();
        AABBox3 transformedBox;
        bbox.GetTransformedBox(mesh->GetWorldTransform(), transformedBox);

        OutputManager::Instance()->Log(StringToWString(mesh->GetName()) + 
                                       Format(L": Min: (%0.2f, %0.2f, %0.2f)", 
                                            transformedBox.min.x, transformedBox.min.y, transformedBox.min.z) + 
                                       
                                       Format(L"; Max: (%0.2f, %0.2f, %0.2f)", 
                                              transformedBox.max.x, transformedBox.max.y, transformedBox.max.z));
    }
}

void EditorBodyControl::UpdatePropertyPanel()
{
    if(selectedSceneGraphNode || selectedDataGraphNode)
    {
        if(!nodesPropertyPanel->GetParent())
        {
            rightPanel->AddControl(nodesPropertyPanel);
        }
        RefreshProperties();
    }
    else
    {
        if(nodesPropertyPanel->GetParent())
        {
            rightPanel->RemoveControl(nodesPropertyPanel);
        }
    }
}

SceneNode * getHighestProxy(SceneNode* curr)
{
	int32 cc = curr->GetChildrenCount();
	if (cc == 0)
		return getHighestProxy(curr->GetParent());
	if (cc > 1)
		return 0;
	if (cc == 1)
    {
        SceneNode * result = getHighestProxy(curr->GetParent());
	    if (result == 0)
            return curr;
        else return result;
        
    }
	
    return NULL;
}

void EditorBodyControl::Input(DAVA::UIEvent *event)
{    
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {
        if(event->tid == DVKEY_ESCAPE)
        {
            if(UIControlSystem::Instance()->GetFocusedControl() == this)
            {
                ResetSelection();
            }
        }

        if (event->keyChar == '1')
            cameraController->SetSpeed(40);
        if (event->keyChar == '2')
            cameraController->SetSpeed(80);
        if (event->keyChar == '3')
            cameraController->SetSpeed(160);
        if (event->keyChar == '4')
            cameraController->SetSpeed(320);
        
        Camera * newCamera = 0;
        if (event->keyChar == 'z')newCamera = scene->GetCamera(0);
        if (event->keyChar == 'x')newCamera = scene->GetCamera(1);
        if (event->keyChar == 'c')newCamera = scene->GetCamera(2);
        if (event->keyChar == 'v')newCamera = scene->GetCamera(3);
        if (event->keyChar == 'b')newCamera = scene->GetCamera(4);
        if (newCamera)
        {
            scene->SetCurrentCamera(newCamera);
            scene->SetClipCamera(scene->GetCamera(0));
        }
		
        if (event->keyChar == 'w') modState = MOD_MOVE;
		if (event->keyChar == 'e') modState = MOD_ROTATE;
		if (event->keyChar == 'r') modState = MOD_SCALE;
        if (event->keyChar == '5') modAxis = AXIS_X;
		if (event->keyChar == '6') modAxis = AXIS_Y;
		if (event->keyChar == '7') modAxis = AXIS_Z;
		if (event->keyChar == '8') 
		{
			if (modAxis < AXIS_XY)
				modAxis = AXIS_XY;
			else
				modAxis = (eModAxis)(AXIS_XY + ((modAxis + 1 - AXIS_XY) % 3));
		}
		UpdateModState();
	}   
	
	//selection with second mouse button 
	if (event->phase == UIEvent::PHASE_BEGAN && event->tid == UIEvent::BUTTON_2)
	{
		Camera * cam = scene->GetCurrentCamera();
		const Rect & rect = scene3dView->GetLastViewportRect();
		Vector3 from = cam->GetPosition();
		Vector3 to = cam->UnProject(event->point.x, event->point.y, 0, rect);
		to -= from;
		to *= 1000.f;
		to += from;
		scene->TrySelection(from, to);
        
        SelectNodeAtTree(scene->GetSelection());
	}	
	
    SceneNode * selection = scene->GetSelection();
	if (selection != 0 && event->tid == UIEvent::BUTTON_1)
	{
		if (event->phase == UIEvent::PHASE_BEGAN)
		{
			inTouch = true;	
			touchStart = event->point;
			
			proxy = getHighestProxy(selection);
			if (proxy == 0)
				proxy = selection;
			
			startTransform = proxy->GetLocalTransform();
			startWT = proxy->GetWorldTransform();
			
			SceneNodeUserData * userData = (SceneNodeUserData*)selection->userData;
			if (userData)
				userData->bulletObject->SetUpdateFlag(false);
			
			//calculate koefficient for moving
			Camera * cam = scene->GetCurrentCamera();
			const Vector3 & camPos = cam->GetPosition();
			const Matrix4 & wt = proxy->GetWorldTransform();
			Vector3 objPos = Vector3(0,0,0) * wt;
			
			Matrix4 inv;
			Matrix4 worldTransform = proxy->GetWorldTransform();
			worldTransform._03 = 0.0f;
			worldTransform._13 = 0.0f;
			worldTransform._23 = 0.0f;
			worldTransform._33 = 1.0f;
			worldTransform._30 = 0.0f;
			worldTransform._31 = 0.0f;
			worldTransform._32 = 0.0f;
			
			//				bool res = worldTransform.GetInverse(inv);				
			
			
			//				float32 transformK = /*((Vector3(0,0,0) * inv) - */(Vector3(0,0,1) * worldTransform).Length();
			Vector3 dir = objPos - camPos;
			moveKf = dir.Length() * 0.003;
			
			//				Logger::Debug(L"transformK = %f", transformK);			
			//				Logger::Debug(L"moveKf = %f", moveKf);				
			//				//moveKf /= transformK;
			//				Logger::Debug(L"result = %f", moveKf);
			//				Logger::Debug(L"inv = %d", res);
		}	
		if (event->phase == UIEvent::PHASE_DRAG)
		{
//				PrepareModMatrix(event->point.x - touchStart.x, event->point.y - touchStart.y);
//				const Matrix4 & worldTransform = proxy->GetWorldTransform();
//
//				Matrix4 worldTransformInverse;
//
//				bool result = ((Matrix4&)worldTransform).GetInverse(worldTransformInverse);
//				if (result)
//					proxy->SetLocalTransform(worldTransform * currTransform * worldTransformInverse);				
//				else 
//					Logger::Debug(L"Error matrix calculation");
				
				PrepareModMatrix(event->point.x - touchStart.x, event->point.y - touchStart.y);
				proxy->SetLocalTransform(currTransform);
            
            nodesPropertyPanel->UpdateFieldsForCurrentNode();
            
		}
		if (event->phase == UIEvent::PHASE_ENDED)
		{
				inTouch = false;
				SceneNodeUserData * userData = (SceneNodeUserData*)selection->userData;
				if (userData)
					userData->bulletObject->SetUpdateFlag(true);
		}
	}
	else
	{
		cameraController->SetSelection(selection);
		cameraController->Input(event);
	}
	UIControl::Input(event);
}

static Vector3 vect[3] = {Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};

void EditorBodyControl::PrepareModMatrix(float32 winx, float32 winy)
{	
	Matrix4 modification;
	modification.Identity();
	
	if (modState == MOD_MOVE)
	{
		switch (modAxis) 
		{
			case AXIS_X:
			case AXIS_Y:
				modification.CreateTranslation(vect[modAxis] * winx * axisSign[modAxis] * moveKf);
				break;
			case AXIS_Z:
				modification.CreateTranslation(vect[modAxis] * winy * axisSign[AXIS_Z] * moveKf);
				break;
			case AXIS_XY:
				modification.CreateTranslation((vect[AXIS_X] * winx * axisSign[AXIS_X] + vect[AXIS_Y] * winy * axisSign[AXIS_Y]) * moveKf);
				break;
			case AXIS_YZ:
				modification.CreateTranslation((vect[AXIS_Y] * winx * axisSign[AXIS_Y] + vect[AXIS_Z] * winy * axisSign[AXIS_Z]) * moveKf);
				break;
			case AXIS_XZ:
				modification.CreateTranslation((vect[AXIS_X] * winx * axisSign[AXIS_X] + vect[AXIS_Z] * winy * axisSign[AXIS_Z]) * moveKf);
				break;
			default:
				break;
		}
	}
	else if (modState == MOD_ROTATE)
	{
		Matrix4 d;
		Matrix4 translate1, translate2;

		Vector3 v = startWT.GetTranslationVector();
		translate1.CreateTranslation(-v);
		translate2.CreateTranslation(v);
		
//        SceneNode * selection = scene->GetSelection();
//		translate1.CreateTranslation(-selection->GetWorldTransform().GetTranslationVector());
//		translate2.CreateTranslation(selection->GetWorldTransform().GetTranslationVector());
		
		switch (modAxis) 
		{
			case AXIS_X:
			case AXIS_Y:
				modification.CreateRotation(vect[modAxis], winy / 100.0f);
				break;
			case AXIS_Z:
				modification.CreateRotation(vect[modAxis], winx / 100.0f);
				break;
			case AXIS_XY:
				modification.CreateRotation(vect[AXIS_X], winx / 100.0f);
				d.CreateRotation(vect[AXIS_Y], winy / 100.0f);
				modification *= d;
				break;
			case AXIS_YZ:
				modification.CreateRotation(vect[AXIS_Y], winx / 100.0f);
				d.CreateRotation(vect[AXIS_Z], winy / 100.0f);
				modification *= d;
				break;
			case AXIS_XZ:
				modification.CreateRotation(vect[AXIS_X], winx / 100.0f);
				d.CreateRotation(vect[AXIS_Z], winy / 100.0f);
				modification *= d;
				break;
			default:
				break;
		}
		modification = (translate1 * modification) * translate2;
	}
	else if (modState == MOD_SCALE)
	{
//		modification.CreateScale(Vector3(1,1,1) + vect[modAxis] * dist/100);
		modification.CreateScale(Vector3(1,1,1) + Vector3(1,1,1) * (winx/100.0f));
	}
	currTransform =  startTransform * modification;
}


void EditorBodyControl::DrawAfterChilds(const UIGeometricData &geometricData)
{
	UIControl::DrawAfterChilds(geometricData);
	SceneNode * selection = scene->GetSelection();
	if (selection)
	{
//		RenderHelper::SetClip();
		
		const Rect & rect = scene3dView->GetLastViewportRect();
		Matrix4 wt = selection->GetWorldTransform();
		Vector3 offs = wt.GetTranslationVector();
		Camera * cam = scene->GetCurrentCamera(); 
		Vector2 start = cam->GetOnScreenPosition(offs, rect);
		Vector2 end;
	
		const Vector3 & vc = cam->GetPosition();
		float32 kf = ((vc - offs).Length() - cam->GetZNear()) * 0.2;
		
		for(int i = 0; i < 3; i++)
		{
			if (modAxis == i
				|| (i == AXIS_X && (modAxis == AXIS_XY || modAxis == AXIS_XZ)) 
				|| (i == AXIS_Y && (modAxis == AXIS_XY || modAxis == AXIS_YZ)) 
				|| (i == AXIS_Z && (modAxis == AXIS_XZ || modAxis == AXIS_YZ)))
			{
				RenderManager::Instance()->SetColor(0, 1.0f, 0, 1.0f);					
			}
			else 
			{
				RenderManager::Instance()->SetColor(1.0f, 0, 0, 1.0f);	
			}

			Vector3 v = offs + vect[i] * kf;
			end = cam->GetOnScreenPosition(v, rect);
			RenderHelper::Instance()->DrawLine(start, end);

		
			if (i == AXIS_X 
				|| (i == AXIS_Y && modAxis == AXIS_Y)
				|| (i == AXIS_Y && modAxis == AXIS_YZ)
				)
			{
				axisSign[i] = (start.x > end.x) ? -1.0f: 1.0f;
			}
			else if (i == AXIS_Y && modAxis == AXIS_XY)
			{
				axisSign[i] = (start.y > end.y) ? -1.0f: 1.0f;				
			}
			else if (i == AXIS_Z)
			{
				axisSign[i] = (start.y > end.y) ? -1.0f: 1.0f;
			}
		}
		RenderManager::Instance()->ResetColor();
	}
}

void EditorBodyControl::Update(float32 timeElapsed)
{
	SceneNode * selection = scene->GetSelection();
	if (selection && modificationPanel->GetParent() == 0)
	{
		AddControl(modificationPanel);
	}
	else if (selection == 0 && modificationPanel->GetParent() != 0)
	{
		RemoveControl(modificationPanel);
	}
	
    UIControl::Update(timeElapsed);
}

void EditorBodyControl::OnLookAtButtonPressed(BaseObject * obj, void *, void *)
{
    MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode*>(selectedSceneGraphNode);
    if (mesh)
    {
        AABBox3 bbox = mesh->GetBoundingBox();
        AABBox3 transformedBox;
        bbox.GetTransformedBox(mesh->GetWorldTransform(), transformedBox);
        Vector3 center = transformedBox.GetCenter();
        scene->GetCurrentCamera()->SetTarget(center);
    }
}

void EditorBodyControl::OnRemoveNodeButtonPressed(BaseObject * obj, void *, void *)
{
    if (selectedSceneGraphNode)
    {
        SceneNode * parentNode = selectedSceneGraphNode->GetParent();
        if (parentNode)
        {
            parentNode->RemoveNode(selectedSceneGraphNode);
            
            selectedSceneGraphNode = NULL;
            savedTreeCell = NULL;
            UpdatePropertyPanel();

            sceneGraphTree->Refresh();
        }
    }
}

void EditorBodyControl::OnEnableDebugFlagsPressed(BaseObject * obj, void *, void *)
{
    if (selectedSceneGraphNode)
    {
        if (selectedSceneGraphNode->GetDebugFlags() & SceneNode::DEBUG_DRAW_ALL)
        {
            selectedSceneGraphNode->SetDebugFlags(0, true);
        }else
        {
            selectedSceneGraphNode->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL, true);
        }
    }
}

void EditorBodyControl::OpenScene(const String &pathToFile, bool editScene)
{
    if(editScene)
    {
        SceneNode *rootNode = scene->GetRootNode(pathToFile);
        mainFilePath = pathToFile;
        rootNode->SetSolid(false);
        scene->AddNode(rootNode);
    }
    else
    {
        SceneNode *rootNode = scene->GetRootNode(pathToFile)->Clone();
        rootNode->SetSolid(true);
        scene->AddNode(rootNode);
    }
    
    if (scene->GetCamera(0))
    {
        scene->SetCurrentCamera(scene->GetCamera(0));
        cameraController->SetCamera(scene->GetCamera(0));
    }
    sceneGraphTree->Refresh();
    RefreshDataGraph();
}

const String &EditorBodyControl::GetFilePath()
{
    return mainFilePath;
}

void EditorBodyControl::WillAppear()
{
    selectedSceneGraphNode = NULL;
    selectedDataGraphNode = NULL;
    savedTreeCell = NULL;

    
    nodesPropertyPanel->SetWorkingScene(scene);
    
    sceneGraphTree->Refresh();
    RefreshDataGraph();
}

void EditorBodyControl::ShowProperties(bool show)
{
    if(show && !rightPanel->GetParent())
    {
        AddControl(rightPanel);
        
        ChangeControlWidthRight(scene3dView, -ControlsFactory::RIGHT_SIDE_WIDTH);
        ChangeControlWidthRight(outputPanel, -ControlsFactory::RIGHT_SIDE_WIDTH);
    }
    else if(!show && rightPanel->GetParent())
    {
        RemoveControl(rightPanel);
        
        ChangeControlWidthRight(scene3dView, ControlsFactory::RIGHT_SIDE_WIDTH);
        ChangeControlWidthRight(outputPanel, ControlsFactory::RIGHT_SIDE_WIDTH);
    }
}

bool EditorBodyControl::PropertiesAreShown()
{
    return (rightPanel->GetParent() != NULL);
}

void EditorBodyControl::ShowSceneGraph(bool show)
{
    ResetSelection();
    
    if(show && !leftPanelSceneGraph->GetParent())
    {
        AddControl(leftPanelSceneGraph);

        ChangeControlWidthLeft(scene3dView, ControlsFactory::LEFT_SIDE_WIDTH);
        ChangeControlWidthLeft(outputPanel, ControlsFactory::LEFT_SIDE_WIDTH);
        
        sceneGraphTree->Refresh();
    }
    else if(!show && leftPanelSceneGraph->GetParent())
    {
        RemoveControl(leftPanelSceneGraph);
        
        ChangeControlWidthLeft(scene3dView, -ControlsFactory::LEFT_SIDE_WIDTH);
        ChangeControlWidthLeft(outputPanel, -ControlsFactory::LEFT_SIDE_WIDTH);
    }
}

bool EditorBodyControl::SceneGraphAreShown()
{
    return (leftPanelSceneGraph->GetParent() != NULL);
}

void EditorBodyControl::ShowDataGraph(bool show)
{
    ResetSelection();

    if(show && !leftPanelDataGraph->GetParent())
    {
        AddControl(leftPanelDataGraph);
        
        ChangeControlWidthLeft(scene3dView, ControlsFactory::LEFT_SIDE_WIDTH);
        ChangeControlWidthLeft(outputPanel, ControlsFactory::LEFT_SIDE_WIDTH);
        
        RefreshDataGraph();
    }
    else if(!show && leftPanelDataGraph->GetParent())
    {
        RemoveControl(leftPanelDataGraph);
        
        ChangeControlWidthLeft(scene3dView, -ControlsFactory::LEFT_SIDE_WIDTH);
        ChangeControlWidthLeft(outputPanel, -ControlsFactory::LEFT_SIDE_WIDTH);
    }
}

bool EditorBodyControl::DataGraphAreShown()
{
    return (leftPanelDataGraph->GetParent() != NULL);
}


void EditorBodyControl::UpdateLibraryState(bool isShown, int32 width)
{
    if(isShown)
    {
        ShowProperties(false);
        
        ChangeControlWidthRight(scene3dView, -width);
        ChangeControlWidthRight(outputPanel, -width);
    }
    else
    {
        ChangeControlWidthRight(scene3dView, ControlsFactory::RIGHT_SIDE_WIDTH);
        ChangeControlWidthRight(outputPanel, ControlsFactory::RIGHT_SIDE_WIDTH);
    }
}

void EditorBodyControl::BeastProcessScene()
{
	beastManager = BeastProxy::Instance()->CreateManager();

	KeyedArchive *keyedArchieve = new KeyedArchive();
	keyedArchieve->Load("~doc:/ResourceEditorOptions.archive");
	String path = keyedArchieve->GetString("LastSavedPath", "/") +"/DataSource/3d/lightmaps/";
	BeastProxy::Instance()->SetLightmapsDirectory(beastManager, path);

	BeastProxy::Instance()->ParseScene(beastManager, scene);
	BeastProxy::Instance()->CreateSkyLight(beastManager);
	BeastProxy::Instance()->SetCamera(beastManager, scene->GetCurrentCamera());
	BeastProxy::Instance()->WindowedRender(beastManager);
}

EditorScene * EditorBodyControl::GetScene()
{
    return scene;
}

void EditorBodyControl::AddNode(SceneNode *node)
{
    scene->AddNode(node);
    sceneGraphTree->Refresh();
    RefreshDataGraph();
}

void EditorBodyControl::ChangeControlWidthRight(UIControl *c, float32 width)
{
    Rect r = c->GetRect();
    r.dx += width;
    c->SetRect(r);
}

void EditorBodyControl::ChangeControlWidthLeft(UIControl *c, float32 width)
{
    Rect r = c->GetRect();
    r.dx -= width;
    r.x += width;
    c->SetRect(r);
}

void EditorBodyControl::NodesPropertyChanged()
{
    if(selectedSceneGraphNode)
    {
        nodesPropertyPanel->WriteTo(selectedSceneGraphNode);
        savedTreeCell->text->SetText(StringToWString(selectedSceneGraphNode->GetName()));
    }
}

void EditorBodyControl::OnRefreshPressed(BaseObject * obj, void *, void *)
{
    NodesPropertyChanged();
}

void EditorBodyControl::Refresh()
{
    sceneGraphTree->Refresh();
    RefreshDataGraph();
}

void EditorBodyControl::SelectNodeAtTree(DAVA::SceneNode *node)
{
    if(savedTreeCell)
    {
        savedTreeCell->SetSelected(false, false);
    }

    selectedSceneGraphNode = node;
    if(node)
    {
        List<void *> nodesForSearch;
        
        SceneNode *nd = node;
        while(nd)
        {
            nodesForSearch.push_front(nd);
            nd = nd->GetParent();
        }
        
        sceneGraphTree->OpenNodes(nodesForSearch);
    }
    else
    {
        sceneGraphTree->Refresh();
    }
    
    UpdatePropertyPanel();
}

void EditorBodyControl::RefreshProperties()
{
    if(selectedSceneGraphNode)
    {
        nodesPropertyPanel->ReadFrom(selectedSceneGraphNode);
    }
    else if(selectedDataGraphNode)
    {
        nodesPropertyPanel->ReadFrom(selectedDataGraphNode);
    }
}

void EditorBodyControl::ResetSelection()
{
    scene->SetSelection(NULL);
    SelectNodeAtTree(NULL);
}

void EditorBodyControl::RefreshDataGraph()
{
    dataNodes[EDNID_MATERIAL] = scene->GetMaterials();
    dataNodes[EDNID_MESH] = scene->GetStaticMeshes();
    dataNodes[EDNID_SCENE] = scene->GetScenes();
    
    dataGraphTree->Refresh();
}

void EditorBodyControl::OnRefreshSceneGraph(BaseObject * obj, void *, void *)
{
    sceneGraphTree->Refresh();
}

void EditorBodyControl::OnRefreshDataGraph(BaseObject * obj, void *, void *)
{
    RefreshDataGraph();
}
