#ifndef __DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA 
{

class RenderComponent : public Component
{
public:
    RenderComponent(RenderObject * _object = 0);
    virtual ~RenderComponent();
    
    IMPLEMENT_COMPONENT_TYPE(RENDER_COMPONENT);
    virtual Component * Clone(SceneNode * toEntity);

    void SetRenderObject(RenderObject * object);
    RenderObject * GetRenderObject();
    
    /**
     \brief This function should be implemented in each node that have data nodes inside it.
     */
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);

    
private:
    RenderObject * renderObject;
    
	void InsterDataNode(DataNode *node, Set<DataNode*> & dataNodes);


public:
    INTROSPECTION_EXTEND(RenderComponent, Component,
        MEMBER(renderObject, "renderObject", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};


};

#endif //__DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
