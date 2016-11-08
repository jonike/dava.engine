#pragma once

#include "Base/RttiType.h"
#include "Base/RttiInheritance.h"
#include "Reflection/Private/StructureWrapperDefault.h"

namespace DAVA
{
class StructureWrapperClass final : public StructureWrapperDefault
{
public:
    StructureWrapperClass(const RttiType* classType, const ReflectedStructure* classStructure);

    /*
    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override;
    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const override;

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
    AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override;
    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override;
    */

    /*
     
     struct ClassBase
     {
     const RttiType* type;
     const ReflectedType* refType;
     
     RttiInheritance::CastOP castToBaseOP;
     ReflectedObject GetBaseObject(const ReflectedObject& obj) const;
     };

    template <typename T>
    void AddField(const char* fieldName, std::unique_ptr<ValueWrapper>&& vw)
    {
        ClassField clField;
        clField.type = ReflectedTypeDB::Get<T>();
        clField.vw = std::move(vw);
        fields.emplace_back(std::make_pair(fieldName, std::move(clField)));
    }

    template <typename T>
    void AddFieldFn(const char* fieldName, std::unique_ptr<ValueWrapper>&& vw)
    {
        ClassField clField;
        clField.type = FnRetTypeToFieldType<T>::Get();
        clField.vw = std::move(vw);
        fields.emplace_back(std::make_pair(fieldName, std::move(clField)));
    }

    template <typename F>
    void AddMethod(const char* methodName, const F& fn)
    {
        methods.emplace_back(std::make_pair(methodName, AnyFn(fn)));
    }

    void AddMeta(ReflectedMeta&& meta)
    {
        if (fields.size() > 0)
        {
            ClassField& clsFiled = fields.back().second;
            clsFiled.meta = std::make_unique<ReflectedMeta>(std::move(meta));
        }
    }
    */

private:
    struct FieldCacheEntry
    {
        const ReflectedStructure::Field* field;
        RttiInheritance::CastOP castToBaseOP;
    };

    Vector<FieldCacheEntry> fieldsCache;

    /*
    template <typename T>
    struct FnRetTypeToFieldType
    {
        static inline const ReflectedType* Get()
        {
            return ReflectedTypeDB::Get<std::nullptr_t>();
        }
    };

    template <typename T>
    struct FnRetTypeToFieldType<T*>
    {
        static inline const ReflectedType* Get()
        {
            return ReflectedTypeDB::Get<T>();
        }
    };

    template <typename T>
    struct FnRetTypeToFieldType<T&>
    {
        static inline const ReflectedType* Get()
        {
            return ReflectedTypeDB::Get<T>();
        }
    };

    const RttiType* thisType;

    Vector<std::pair<String, ClassField>> fields;
    Vector<std::pair<String, AnyFn>> methods;

    mutable bool basesInitialized = false;
    mutable Vector<ClassBase> bases;

    void InitBaseClasses() const;
     */
};

} // namespace DAVA
