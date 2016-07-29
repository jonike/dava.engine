#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Entity;
class VegetationRenderObject;
}

class SceneEditor2;
class EditorVegetationSystem : public DAVA::SceneSystem
{
public:
    EditorVegetationSystem(DAVA::Scene* scene);
    ~EditorVegetationSystem() override;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void GetActiveVegetation(DAVA::Vector<DAVA::VegetationRenderObject*>& activeVegetationObjects);

private:
    DAVA::Vector<DAVA::VegetationRenderObject*> vegetationObjects;
};
