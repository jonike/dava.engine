#include "Classes/SceneManager/Private/SceneRenderWidget.h"
#include "Classes/SceneManager/Private/SceneTabsModel.h"

#include "TArc/Controls/SceneTabbar.h"

#include "UI/Focus/UIFocusComponent.h"
#include "Engine/Qt/RenderWidget.h"
#include "Engine/EngineContext.h"
#include "Functional/Function.h"
#include "Base/StaticSingleton.h"
#include "Deprecated/ScenePreviewDialog.h"

SceneRenderWidget::SceneRenderWidget(DAVA::TArc::ContextAccessor* accessor_, DAVA::RenderWidget* renderWidget_)
    : accessor(accessor_)
    , renderWidget(renderWidget_)
{
    using namespace DAVA::TArc;
    activeSceneWrapper = accessor->CreateWrapper(DAVA::ReflectedType::Get<SceneData>());
    activeSceneWrapper.SetListener(this);

    DataContext* ctx = accessor->GetGlobalContext();
    ctx->CreateData(std::make_unique<SceneTabsModel>());

    SceneTabbar* tabBar = new SceneTabbar(accessor, DAVA::Reflection::Create(ctx->GetData<SceneTabsModel>()), this);
    tabBar->setTabsClosable(true);
    tabBar->setMovable(true);
    tabBar->setUsesScrollButtons(true);
    tabBar->setExpanding(false);

    setMinimumSize(renderWidget->minimumSize());

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(tabBar);
    layout->addWidget(renderWidget);
    layout->setMargin(1);
    layout->setSpacing(3);
    setLayout(layout);

    InitDavaUI();

    connections.AddConnection(renderWidget, &DAVA::RenderWidget::Resized, DAVA::MakeFunction(this, &SceneRenderWidget::OnRenderWidgetResized));
    connections.AddConnection(SceneSignals::Instance(), &SceneSignals::MouseOverSelection, DAVA::MakeFunction(this, &SceneRenderWidget::MouseOverSelection));

    tabBar->closeTab.Connect(this, &SceneRenderWidget::OnCloseTab);
}

SceneRenderWidget::~SceneRenderWidget()
{
    if (previewDialog != nullptr)
    {
        previewDialog->RemoveFromParent();
    }
    SafeRelease(previewDialog);
}

void SceneRenderWidget::ShowPreview(const DAVA::FilePath& scenePath)
{
    if (!previewDialog)
    {
        previewDialog = new ScenePreviewDialog();
    }

    if (scenePath.IsEqualToExtension(".sc2"))
    {
        previewDialog->Show(scenePath);
    }
    else
    {
        previewDialog->Close();
    }
}

void SceneRenderWidget::HidePreview()
{
    if (previewDialog && previewDialog->GetParent())
    {
        previewDialog->Close();
    }
}

void SceneRenderWidget::SetWidgetDelegate(IWidgetDelegate* widgetDelegate_)
{
    DVASSERT(widgetDelegate == nullptr);
    widgetDelegate = widgetDelegate_;
}

void SceneRenderWidget::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA::TArc;

    auto iter = std::find(fields.begin(), fields.end(), DAVA::Any(DAVA::String(SceneData::scenePropertyName)));
    if (iter == fields.end() && !fields.empty() && wrapper.HasData())
    {
        return;
    }

    DAVA::RefPtr<SceneEditor2> currentScene;
    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext != nullptr)
    {
        SceneData* data = activeContext->GetData<SceneData>();
        if (data != nullptr)
        {
            currentScene = data->GetScene();
        }
    }

    if (currentScene.Get() != nullptr)
    {
        if (dava3DView->GetParent() == nullptr)
        {
            const DAVA::List<DAVA::UIControl*>& children = davaUIScreen->GetChildren();
            if (children.empty())
            {
                davaUIScreen->AddControl(dava3DView.Get());
            }
            else
            {
                davaUIScreen->InsertChildBelow(dava3DView.Get(), children.front());
            }
        }

        dava3DView->SetScene(currentScene.Get());
        currentScene->SetViewportRect(dava3DView->GetRect());
    }
    else
    {
        dava3DView->SetScene(NULL);
        davaUIScreen->RemoveControl(dava3DView.Get());
    }
}

void SceneRenderWidget::InitDavaUI()
{
    dava3DView.Set(new DAVA::UI3DView(DAVA::Rect(dava3DViewMargin, dava3DViewMargin, 0, 0)));
    dava3DView->SetInputEnabled(true, true);
    dava3DView->GetOrCreateComponent<DAVA::UIFocusComponent>();
    dava3DView->SetName(DAVA::FastName("Scene Tab 3D View"));

    davaUIScreen = new DAVA::UIScreen();

    DAVA::EngineContext* engineCtx = accessor->GetEngineContext();

    engineCtx->uiScreenManager->RegisterScreen(davaUIScreenID, davaUIScreen.Get());
    engineCtx->uiScreenManager->SetScreen(davaUIScreenID);
}

void SceneRenderWidget::OnRenderWidgetResized(DAVA::uint32 w, DAVA::uint32 h)
{
    using namespace DAVA::TArc;

    accessor->GetEngineContext()->uiControlSystem->vcs->SetVirtualScreenSize(w, h);

    davaUIScreen->SetSize(DAVA::Vector2(w, h));
    dava3DView->SetSize(DAVA::Vector2(w - 2 * dava3DViewMargin, h - 2 * dava3DViewMargin));

    DataContext* ctx = accessor->GetActiveContext();
    if (ctx == nullptr)
    {
        return;
    }

    SceneData* data = ctx->GetData<SceneData>();
    if (data == nullptr)
    {
        return;
    }

    DAVA::RefPtr<SceneEditor2> scene = data->GetScene();
    if (scene.Get() == nullptr)
    {
        return;
    }

    scene->SetViewportRect(dava3DView->GetRect());
}

void SceneRenderWidget::OnCloseTab(DAVA::uint64 id)
{
    if (widgetDelegate)
    {
        widgetDelegate->CloseSceneRequest(id);
    }
}

void SceneRenderWidget::MouseOverSelection(SceneEditor2* scene, const SelectableGroup* objects)
{
    using namespace DAVA::TArc;

    static QCursor cursorMove(QPixmap(":/QtIcons/curcor_move.png"));
    static QCursor cursorRotate(QPixmap(":/QtIcons/curcor_rotate.png"));
    static QCursor cursorScale(QPixmap(":/QtIcons/curcor_scale.png"));

    DataContext* ctx = accessor->GetActiveContext();
    SceneData* data = ctx->GetData<SceneData>();

    if ((data->GetScene() == scene) && (objects != nullptr))
    {
        switch (scene->modifSystem->GetTransformType())
        {
        case Selectable::TransformType::Translation:
            renderWidget->setCursor(cursorMove);
            break;
        case Selectable::TransformType::Rotation:
            renderWidget->setCursor(cursorRotate);
            break;
        case Selectable::TransformType::Scale:
            renderWidget->setCursor(cursorScale);
            break;
        case Selectable::TransformType::Disabled:
        default:
            renderWidget->unsetCursor();
            break;
        }
    }
    else
    {
        renderWidget->unsetCursor();
    }
}
