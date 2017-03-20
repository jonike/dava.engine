#include "UI/Find/FindInDocumentController.h"
#include "UI/Find/Finder/Finder.h"
#include "UI/Find/Widgets/FindInDocumentWidget.h"
#include "UI/mainwindow.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DocumentsModule/DocumentsModule.h"

#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>

using namespace DAVA;

FindInDocumentController::FindInDocumentController(DocumentsModule* documentsModule_, MainWindow* mainWindow, FindInDocumentWidget* findInDocumentWidget_)
    : QObject()
    , documentsModule(documentsModule_)
    , findInDocumentWidget(findInDocumentWidget_)
{
    TArc::UI* ui = documentsModule->GetUI();

    TArc::ContextAccessor* accessor = documentsModule->GetAccessor();

    TArc::FieldDescriptor packageFieldDescr;
    packageFieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
    packageFieldDescr.fieldName = FastName(DocumentData::packagePropertyName);

    const auto updater =
    [](const Any& fieldValue) -> Any {
        return fieldValue.CanCast<PackageNode*>() && fieldValue.Cast<PackageNode*>() != nullptr;
    };

    TArc::QtAction* findInDocumentAction = new TArc::QtAction(accessor, QObject::tr("Find in Document"), this);
    findInDocumentAction->setShortcut(QKeySequence::Find);
    findInDocumentAction->SetStateUpdationFunction(TArc::QtAction::Enabled, packageFieldDescr, updater);

    TArc::QtAction* findNextAction = new TArc::QtAction(accessor, QObject::tr("Find Next"), this);
    findNextAction->setShortcut(QKeySequence::FindNext);
    findNextAction->SetStateUpdationFunction(TArc::QtAction::Enabled, packageFieldDescr, updater);

    TArc::QtAction* findPreviousAction = new TArc::QtAction(accessor, QObject::tr("Find Previous"), this);
    findPreviousAction->setShortcut(QKeySequence::FindPrevious);
    findPreviousAction->SetStateUpdationFunction(TArc::QtAction::Enabled, packageFieldDescr, updater);

    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnFindFilterReady, this, &FindInDocumentController::SetFilter);
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnFindNext, this, &FindInDocumentController::SelectNextFindResult);
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnFindPrevious, this, &FindInDocumentController::SelectPreviousFindResult);
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnFindAll, this, &FindInDocumentController::FindAll);
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnStopFind, this, &FindInDocumentController::HideFindInDocumentWidget);

    QObject::connect(findInDocumentAction, &QAction::triggered, this, &FindInDocumentController::ShowFindInDocumentWidget);
    QObject::connect(findNextAction, &QAction::triggered, this, &FindInDocumentController::SelectNextFindResult);
    QObject::connect(findPreviousAction, &QAction::triggered, this, &FindInDocumentController::SelectPreviousFindResult);

    TArc::ActionPlacementInfo placementInfo(TArc::CreateMenuPoint("Find", TArc::InsertionParams(TArc::InsertionParams::eInsertionMethod::AfterItem)));
    ui->AddAction(QEGlobal::windowKey, placementInfo, findInDocumentAction);
    ui->AddAction(QEGlobal::windowKey, placementInfo, findNextAction);
    ui->AddAction(QEGlobal::windowKey, placementInfo, findPreviousAction);

    findInDocumentWidget->hide();

    editedRootControlsFieldBinder.reset(new TArc::FieldBinder(accessor));
    TArc::FieldDescriptor editedRootControlsFieldDescriptor(ReflectedTypeDB::Get<DocumentData>(), FastName(DocumentData::editedRootControlsPropertyName));
    editedRootControlsFieldBinder->BindField(editedRootControlsFieldDescriptor, MakeFunction(this, &FindInDocumentController::OnEditedRootControlsChanged));
}

void FindInDocumentController::ShowFindInDocumentWidget()
{
    TArc::ContextAccessor* accessor = documentsModule->GetAccessor();
    TArc::DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext != nullptr)
    {
        findInDocumentWidget->Reset();
        findInDocumentWidget->show();
        findInDocumentWidget->setFocus();
    }
}

void FindInDocumentController::HideFindInDocumentWidget()
{
    findInDocumentWidget->hide();
}

void FindInDocumentController::SelectNextFindResult()
{
    MoveSelection(+1);
}

void FindInDocumentController::SelectPreviousFindResult()
{
    MoveSelection(-1);
}

void FindInDocumentController::FindAll()
{
    documentsModule->InvokeOperation(QEGlobal::FindInDocument.ID, context.filter);
}

void FindInDocumentController::SetFilter(std::shared_ptr<FindFilter> filter)
{
    context.filter = filter;
    context.results.clear();
    context.currentSelection = -1;

    Finder finder(filter, nullptr);

    QObject::connect(&finder, &Finder::ItemFound,
                     [this](const FindItem& item)
                     {
                         for (const String& path : item.GetControlPaths())
                         {
                             context.results.push_back(path);
                         }
                     });

    TArc::ContextAccessor* accessor = documentsModule->GetAccessor();
    TArc::DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext != nullptr)
    {
        DocumentData* data = activeContext->GetData<DocumentData>();

        const SortedControlNodeSet& editedRootControls = data->GetEditedRootControls();

        finder.Process(data->GetPackageNode(), editedRootControls);
    }
}

void FindInDocumentController::MoveSelection(int32 step)
{
    if (!context.results.empty())
    {
        context.currentSelection += step;

        if (context.currentSelection < 0)
        {
            context.currentSelection = static_cast<int32>(context.results.size() - 1);
        }
        else if (context.currentSelection >= context.results.size())
        {
            context.currentSelection = 0;
        }

        TArc::ContextAccessor* accessor = documentsModule->GetAccessor();
        TArc::DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* data = activeContext->GetData<DocumentData>();

        const QString& path = data->GetPackageAbsolutePath();
        const QString& name = QString::fromStdString(context.results[context.currentSelection]);
        documentsModule->InvokeOperation(QEGlobal::SelectControl.ID, path, name);
    }
}

void FindInDocumentController::OnEditedRootControlsChanged(const DAVA::Any& value)
{
    HideFindInDocumentWidget();

    context.results.clear();
    context.currentSelection = -1;
}
