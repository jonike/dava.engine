#ifndef __DAVAENGINE_UI_IGNORE_LAYOUT_COMPONENT_H__
#define __DAVAENGINE_UI_IGNORE_LAYOUT_COMPONENT_H__

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UIControl;

class UIIgnoreLayoutComponent : public UIBaseComponent<UIIgnoreLayoutComponent>
{
    DAVA_VIRTUAL_REFLECTION(UIIgnoreLayoutComponent, UIComponent);

public:
    UIIgnoreLayoutComponent() = default;
    UIIgnoreLayoutComponent(const UIIgnoreLayoutComponent& src);

protected:
    virtual ~UIIgnoreLayoutComponent() = default;

private:
    UIIgnoreLayoutComponent& operator=(const UIIgnoreLayoutComponent&) = delete;

public:
    UIIgnoreLayoutComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool enabled_);

private:
    bool enabled = true;
};
}


#endif //__DAVAENGINE_UI_IGNORE_LAYOUT_COMPONENT_H__
