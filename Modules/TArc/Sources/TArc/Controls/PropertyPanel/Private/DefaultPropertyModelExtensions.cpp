#include "TArc/Controls/PropertyPanel/Private/DefaultPropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"

#include "TArc/Controls/PropertyPanel/Private/ObjectsPool.h"
#include "TArc/Controls/PropertyPanel/Private/TextComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/BoolComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/EnumComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/FlagsComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/NumberComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/FilePathComponentValue.h"
#include "TArc/Utils/ReflectionHelpers.h"

#include <Debug/DVAssert.h>

namespace DAVA
{
namespace TArc
{

void DefaultChildCheatorExtension::ExposeChildren(const std::shared_ptr<const PropertyNode>& node, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    DVASSERT(node->field.ref.IsValid());

    if (node->propertyType == PropertyNode::SelfRoot || node->propertyType == PropertyNode::RealProperty)
    {
        UnorderedSet<String> groups;
        ForEachField(node->field.ref, [&](Reflection::Field&& field)
                     {
                         if (field.ref.HasMeta<M::HiddenField>() == false)
                         {
                             const M::Group* groupMeta = field.ref.GetMeta<M::Group>();
                             if (groupMeta == nullptr)
                             {
                                 children.push_back(allocator->CreatePropertyNode(std::move(field)));
                             }
                             else
                             {
                                 if (groups.count(groupMeta->groupName) == 0)
                                 {
                                     Reflection::Field groupField = node->field;
                                     groupField.key = groupMeta->groupName;
                                     children.push_back(allocator->CreatePropertyNode(std::move(groupField), PropertyNode::GroupProperty, groupMeta->groupName));
                                     groups.insert(groupMeta->groupName);
                                 }
                             }
                         }
                     });
    }
    else if (node->propertyType == PropertyNode::GroupProperty)
    {
        String groupName = node->cachedValue.Cast<String>();
        ForEachField(node->field.ref, [&](Reflection::Field&& field) {
            const M::Group* groupMeta = field.ref.GetMeta<M::Group>();
            if (groupMeta != nullptr && groupMeta->groupName == groupName)
            {
                children.push_back(allocator->CreatePropertyNode(std::move(field)));
            }
        });
    }

    return ChildCreatorExtension::ExposeChildren(node, children);
}

class DefaultAllocator : public IChildAllocator
{
public:
    DefaultAllocator();
    ~DefaultAllocator() override = default;
    std::shared_ptr<PropertyNode> CreatePropertyNode(Reflection::Field&& reflection, int32_t type = PropertyNode::RealProperty) override;
    std::shared_ptr<PropertyNode> CreatePropertyNode(Reflection::Field&& reflection, int32_t type, const Any& value) override;

private:
    ObjectsPool<PropertyNode, SingleThreadStrategy> pool;
};

DefaultAllocator::DefaultAllocator()
    : pool(10000 /*batch size*/, 10 /*initial batches count*/)
{
}

std::shared_ptr<PropertyNode> DefaultAllocator::CreatePropertyNode(Reflection::Field&& field, int32_t type)
{
    if (field.ref.IsValid())
        return CreatePropertyNode(std::move(field), type, field.ref.GetValue());

    return CreatePropertyNode(std::move(field), type, Any());
}

std::shared_ptr<PropertyNode> DefaultAllocator::CreatePropertyNode(Reflection::Field&& field, int32_t type, const Any& value)
{
    std::shared_ptr<PropertyNode> result = pool.RequestObject();
    result->propertyType = type;
    result->field = std::move(field);
    result->cachedValue = value;

    return result;
}

std::shared_ptr<IChildAllocator> CreateDefaultAllocator()
{
    return std::make_shared<DefaultAllocator>();
}

ReflectedPropertyItem* DefaultMergeValueExtension::LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const
{
    DVASSERT(node->field.ref.IsValid());

    ReflectedPropertyItem* result = nullptr;
    const ReflectedType* valueType = node->field.ref.GetValueObject().GetReflectedType();

    for (const std::unique_ptr<ReflectedPropertyItem>& item : items)
    {
        DVASSERT(item->GetPropertyNodesCount() > 0);
        std::shared_ptr<const PropertyNode> etalonNode = item->GetPropertyNode(0);
        const ReflectedType* etalonItemType = etalonNode->field.ref.GetValueObject().GetReflectedType();

        if (valueType == etalonItemType && etalonNode->field.key == node->field.key)
        {
            result = item.get();
            break;
        }
    }

    return result;
}

DefaultEditorComponentExtension::DefaultEditorComponentExtension(UI* ui_)
    : ui(ui_)
{
}

std::unique_ptr<BaseComponentValue> DefaultEditorComponentExtension::GetEditor(const std::shared_ptr<const PropertyNode>& node) const
{
    if (node->propertyType == PropertyNode::RealProperty)
    {
        if (node->field.ref.HasMeta<M::Enum>() ||
            node->field.ref.HasMeta<M::ValueEnumeratorField>())
        {
            return std::make_unique<EnumComponentValue>();
        }
        else if (node->field.ref.HasMeta<M::Flags>())
        {
            return std::make_unique<FlagsComponentValue>();
        }

        static UnorderedMap<const Type*, Function<std::unique_ptr<BaseComponentValue>()>> simpleCreatorsMap =
        {
          std::make_pair(Type::Instance<String>(), &std::make_unique<TextComponentValue>),
          std::make_pair(Type::Instance<FastName>(), &std::make_unique<TextComponentValue>),
          std::make_pair(Type::Instance<const char*>(), &std::make_unique<TextComponentValue>),
          std::make_pair(Type::Instance<bool>(), &std::make_unique<BoolComponentValue>),
          std::make_pair(Type::Instance<float32>(), &std::make_unique<NumberComponentValue<float32>>),
          std::make_pair(Type::Instance<float64>(), &std::make_unique<NumberComponentValue<float64>>),
          std::make_pair(Type::Instance<int8>(), &std::make_unique<NumberComponentValue<int8>>),
          std::make_pair(Type::Instance<uint8>(), &std::make_unique<NumberComponentValue<uint8>>),
          std::make_pair(Type::Instance<int16>(), &std::make_unique<NumberComponentValue<int16>>),
          std::make_pair(Type::Instance<uint16>(), &std::make_unique<NumberComponentValue<uint16>>),
          std::make_pair(Type::Instance<int32>(), &std::make_unique<NumberComponentValue<int32>>),
          std::make_pair(Type::Instance<uint32>(), &std::make_unique<NumberComponentValue<uint32>>)
        };

        const Type* valueType = node->cachedValue.GetType()->Decay();
        auto iter = simpleCreatorsMap.find(valueType);
        if (iter != simpleCreatorsMap.end())
        {
            return iter->second();
        }
        else if (valueType == Type::Instance<FilePath>())
        {
            FilePathComponentValue::Params p;
            p.ui = ui;
            return std::make_unique<FilePathComponentValue>(p);
        }
    }

    return EditorComponentExtension::GetEditor(node);
}
} // namespace TArc
} // namespace DAVA
