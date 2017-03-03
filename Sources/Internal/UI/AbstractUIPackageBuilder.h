#ifndef __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "UI/Styles/UIStyleSheetSelectorChain.h"

namespace DAVA
{
class UIPackage;
class UIControl;
class UIStyleSheet;
class UIComponent;
class UIControlBackground;
class YamlNode;
class AbstractUIPackageBuilder;

class AbstractUIPackageLoader
{
public:
    virtual ~AbstractUIPackageLoader();
    virtual bool LoadPackage(const FilePath& packagePath, AbstractUIPackageBuilder* builder) = 0;
    virtual bool LoadControlByName(const FastName& name, AbstractUIPackageBuilder* builder) = 0;
};

class AbstractUIPackageBuilder
{
public:
    class UIControlWithTypeInfo
    {
    public:
        UIControlWithTypeInfo() = default;
        UIControlWithTypeInfo(UIControl* control_);
        UIControlWithTypeInfo(const InspInfo* typeInfo);

        UIControl* GetControl() const;
        const InspInfo* GetTypeInfo() const;

    private:
        UIControl* control = nullptr;
        const InspInfo* typeInfo = nullptr;
    };

    enum eControlPlace
    {
        TO_PROTOTYPES,
        TO_CONTROLS,
        TO_PREVIOUS_CONTROL
    };

    AbstractUIPackageBuilder();
    virtual ~AbstractUIPackageBuilder();

    virtual void BeginPackage(const FilePath& packagePath) = 0;
    virtual void EndPackage() = 0;

    virtual bool ProcessImportedPackage(const String& packagePath, AbstractUIPackageLoader* loader) = 0;
    virtual void ProcessStyleSheet(const Vector<UIStyleSheetSelectorChain>& selectorChains, const Vector<UIStyleSheetProperty>& properties) = 0;

    virtual UIControlWithTypeInfo BeginControlWithClass(const FastName& controlName, const String& className) = 0;
    virtual UIControlWithTypeInfo BeginControlWithCustomClass(const FastName& controlName, const String& customClassName, const String& className) = 0;
    virtual UIControlWithTypeInfo BeginControlWithPrototype(const FastName& controlName, const String& packageName, const FastName& prototypeName, const String* customClassName, AbstractUIPackageLoader* loader) = 0;
    virtual UIControlWithTypeInfo BeginControlWithPath(const String& pathName) = 0;
    virtual UIControlWithTypeInfo BeginUnknownControl(const FastName& controlName, const YamlNode* node) = 0;
    virtual void EndControl(eControlPlace controlPlace) = 0;

    virtual void BeginControlPropertiesSection(const String& name) = 0;
    virtual void EndControlPropertiesSection() = 0;

    virtual const InspInfo* BeginComponentPropertiesSection(uint32 componentType, uint32 componentIndex) = 0;
    virtual void EndComponentPropertiesSection() = 0;

    virtual void ProcessProperty(const InspMember* member, const VariantType& value) = 0;
};
}

#endif // __DAVAENGINE_UI_ABSTRACT_PACKAGE_LOADER_H__
