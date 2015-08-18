#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

#include "SnapshotDiffViewerWidget.h"

#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsListModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BranchDiffTreeModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BlockListModel.h"

#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/BranchDiff.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"
#include "Qt/DeviceInfo/MemoryTool/SymbolsWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QLineEdit>
#include <QListView>

using namespace DAVA;

SnapshotDiffViewerWidget::SnapshotDiffViewerWidget(const MemorySnapshot* snapshot1_, const MemorySnapshot* snapshot2_, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , snapshot1(snapshot1_)
    , snapshot2(snapshot2_)
{
    DVASSERT(snapshot1 != nullptr && snapshot1->IsLoaded());
    DVASSERT(snapshot2 != nullptr && snapshot2->IsLoaded());
    Init();
}

SnapshotDiffViewerWidget::~SnapshotDiffViewerWidget()
{
    delete branchTreeModel;
    delete blockListModel1;
    delete blockListModel2;
}

void SnapshotDiffViewerWidget::Init()
{
    tab = new QTabWidget;

    InitSymbolsView();
    InitBranchView();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    setLayout(mainLayout);
}

void SnapshotDiffViewerWidget::InitSymbolsView()
{
    symbolWidget = new SymbolsWidget(*snapshot1->SymbolTable());
    QPushButton* buildTree = new QPushButton("Build tree");
    connect(buildTree, &QPushButton::clicked, this, &SnapshotDiffViewerWidget::SymbolView_OnBuldTree);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(buildTree);
    layout->addWidget(symbolWidget);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);
    tab->addTab(frame, "Symbols");
}

void SnapshotDiffViewerWidget::InitBranchView()
{
    branchTreeModel = new BranchDiffTreeModel(snapshot1, snapshot2);
    blockListModel1 = new BlockListModel;
    blockListModel2 = new BlockListModel;

    branchTree = new QTreeView;
    branchTree->setFont(QFont("Consolas", 10, 500));
    branchTree->setModel(branchTreeModel);

    blockList1 = new QListView;
    blockList1->setFont(QFont("Consolas", 10, 500));
    blockList1->setModel(blockListModel1);

    blockList2 = new QListView;
    blockList2->setFont(QFont("Consolas", 10, 500));
    blockList2->setModel(blockListModel2);

    QItemSelectionModel* selModel = branchTree->selectionModel();
    connect(selModel, &QItemSelectionModel::currentChanged, this, &SnapshotDiffViewerWidget::BranchView_SelectionChanged);
    //connect(blockList, &QTreeView::doubleClicked, this, &SnapshotDiffViewerWidget::BranchBlockView_DoubleClicked);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(blockList1);
    hlayout->addWidget(blockList2);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(branchTree);
    layout->addLayout(hlayout);

    QFrame* frame = new QFrame;
    frame->setLayout(layout);

    tab->addTab(frame, "Branches");
}

void SnapshotDiffViewerWidget::SymbolView_OnBuldTree()
{
    Vector<const String*> selection = symbolWidget->GetSelectedSymbols();
    if (!selection.empty())
    {
        branchTreeModel->PrepareModel(selection);
    }
}

void SnapshotDiffViewerWidget::BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    BranchDiff* branchDiff = static_cast<BranchDiff*>(current.internalPointer());

    if (branchDiff->left)
    {
        Vector<MMBlock*> blocks = branchDiff->left->GetMemoryBlocks();
        blockListModel1->PrepareModel(std::forward<Vector<MMBlock*>>(blocks));
    }
    else
        blockListModel1->ResetModel();

    if (branchDiff->right)
    {
        Vector<MMBlock*> blocks = branchDiff->right->GetMemoryBlocks();
        blockListModel2->PrepareModel(std::forward<Vector<MMBlock*>>(blocks));
    }
    else
        blockListModel2->ResetModel();
}

void SnapshotDiffViewerWidget::BranchBlockView_DoubleClicked(const QModelIndex& current)
{
    
}
