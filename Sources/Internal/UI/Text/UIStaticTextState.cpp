#include "UIStaticTextState.h"
#include "UIStaticTextComponent.h"

#include "UI/UIControl.h"
#include "UI/Layouts/LayoutFormula.h"
#include "Math/Vector.h"

#include "Utils/Utils.h"
#include "Base/ObjectFactory.h"
#include "Utils/StringFormat.h"
#include "FileSystem/LocalizationSystem.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Animation/LinearAnimation.h"
#include "Utils/StringUtils.h"
#include "Render/2D/TextBlockSoftwareRender.h"
#include "Render/RenderHelper.h"
#include "UI/UIControlSystem.h"
#include "Job/JobManager.h"
#include "Utils/UTF8Utils.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Math\Math2D.h"

namespace DAVA
{


UIStaticTextState::UIStaticTextState(UIControl* control_, UIStaticTextComponent* component_)
{
    control = control_;
    component = component_;
    Rect rect = control->GetRect();
    control->SetInputEnabled(false, false);
    textBlock = TextBlock::Create(Vector2(rect.dx, rect.dy));

    textBg = new UIControlBackground();
    textBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    textBg->SetColorInheritType(component->GetColorInheritType());
    textBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());

    shadowBg = new UIControlBackground();
    shadowBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    shadowBg->SetColorInheritType(component->GetColorInheritType());
    shadowBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());
}

UIStaticTextState::~UIStaticTextState()
{
    SafeRelease(textBlock);
    SafeRelease(shadowBg);
    SafeRelease(textBg);
}

void UIStaticTextState::ApplyComponentData()
{
    DVASSERT(control == component->GetControl(), "Invalid control poiner!");

    if (component->IsModified())
    {
        component->SetModified(false);

        textBg->SetColorInheritType(component->GetColorInheritType());
        textBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());
        textBg->SetColor(component->GetColor());

        shadowBg->SetColorInheritType(component->GetColorInheritType());
        shadowBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());
        shadowBg->SetColor(component->GetShadowColor());

        textBlock->SetRectSize(control->size);

        switch (component->GetFitting())
        {
        default:
        case UIStaticTextComponent::eTextFitting::FITTING_NONE:
            textBlock->SetFittingOption(0);
            break;
        case UIStaticTextComponent::eTextFitting::FITTING_ENLARGE:
            textBlock->SetFittingOption(TextBlock::eFitType::FITTING_ENLARGE);
            break;
        case UIStaticTextComponent::eTextFitting::FITTING_REDUCE:
            textBlock->SetFittingOption(TextBlock::eFitType::FITTING_REDUCE);
            break;
        case UIStaticTextComponent::eTextFitting::FITTING_FILL:
            textBlock->SetFittingOption(TextBlock::eFitType::FITTING_REDUCE | TextBlock::eFitType::FITTING_ENLARGE);
            break;
        case UIStaticTextComponent::eTextFitting::FITTING_POINTS:
            textBlock->SetFittingOption(TextBlock::eFitType::FITTING_POINTS);
            break;
        }

        textBlock->SetText(UTF8Utils::EncodeToWideString(component->GetText()), component->GetRequestedTextRectSize());

        String fontName = component->GetFontName();
        if (!fontName.empty())
        {
            Font* font = FontManager::Instance()->GetFont(fontName);
            if (textBlock->GetFont() != font)
            {
                textBlock->SetFont(font);
            }
        }

        switch (component->GetMultiline())
        {
        default:
        case UIStaticTextComponent::eTextMultiline::MULTILINE_DISABLED:
            textBlock->SetMultiline(false, false);
            break;
        case UIStaticTextComponent::eTextMultiline::MULTILINE_ENABLED:
            textBlock->SetMultiline(true, false);
            break;
        case UIStaticTextComponent::eTextMultiline::MULTILINE_ENABLED_BY_SYMBOL:
            textBlock->SetMultiline(true, true);
            break;
        }

        textBlock->SetAlign(component->GetAlign());
        textBlock->SetUseRtlAlign(component->GetUseRtlAlign());
        textBlock->SetForceBiDiSupportEnabled(component->IsForceBiDiSupportEnabled());

        if (textBlock->NeedCalculateCacheParams())
        {
            control->SetLayoutDirty();
        }

    }
}
};
