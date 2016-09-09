#pragma once

#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
enum BillboardType : uint32
{
    BILLBOARD_SPHERICAL,
    BILLBOARD_CYLINDRICAL
};

class BillboardRenderObject : public RenderObject
{
public:
    BillboardRenderObject();

    void RecalcBoundingBox() override;
    void PrepareToRender(Camera* camera) override;
    void BindDynamicParameters(Camera* camera) override;

    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    uint32 GetBillboardType() const;
    void SetBillboardType(uint32 type);

private:
    Matrix4 billboardTransform = Matrix4::IDENTITY;
    uint32 billboardType = BillboardType::BILLBOARD_SPHERICAL;

public:
    INTROSPECTION_EXTEND(BillboardRenderObject, RenderObject,
                         MEMBER(billboardType, InspDesc("billboardType", GlobalEnumMap<BillboardType>::Instance()), I_SAVE | I_VIEW | I_EDIT))
};

inline uint32 BillboardRenderObject::GetBillboardType() const
{
    return billboardType;
}
}
