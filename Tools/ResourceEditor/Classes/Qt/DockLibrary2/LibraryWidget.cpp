/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "LibraryWidget.h"
#include "LibraryFilteringModel.h"
#include "LibraryFileSystemModel.h"

#include "Main/mainwindow.h"
#include "Project/ProjectManager.h"
#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/DAEConvertAction.h"


#include <QToolBar>
#include <QLineEdit>
#include <QComboBox>
#include <QTreeView>
#include <QHeaderView>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QProcess>
#include <QFileSystemModel>
#include <QMenu>
#include <QAction>
#include <QStringList>

Q_DECLARE_METATYPE( QFileInfo )



struct FileType
{
    FileType() {}

    FileType(const QString &n)
    {
        name = n;
    }

    
    FileType(const QString &n, const QString &f)
    {
        name = n;
        filter << f;
    }

    FileType(const QString &n, const QString &f1, const QString &f2)
    {
        name = n;
        filter << f1;
        filter << f2;
    }

    QString name;
    QStringList filter;
};

QVector<FileType> fileTypeValues;

LibraryWidget::LibraryWidget(QWidget *parent /* = 0 */)
	: QWidget(parent)
{
    SetupFileTypes();
    SetupToolbar();
    SetupView();
    SetupLayout();

    
    spacer = NULL;
    
    
    ViewAsList();
    OnFilesTypeChanged(0);
}

LibraryWidget::~LibraryWidget()
{
    if(spacer)
    {
        delete spacer;
        spacer = NULL;
    }
}

void LibraryWidget::SetupFileTypes()
{
    FileType allFiles("All files");
    allFiles.filter << "*.dae";
    allFiles.filter << "*.sc2";
    allFiles.filter << "*.png";
    allFiles.filter << "*.tex";
    
    fileTypeValues.push_back(allFiles);
    fileTypeValues.push_back(FileType("Models", "*.dae", "*.sc2"));
    fileTypeValues.push_back(FileType("Textures", "*.png", "*.tex"));
    fileTypeValues.push_back(FileType("DAE", "*.dae"));
    fileTypeValues.push_back(FileType("PNG", "*.png"));
    fileTypeValues.push_back(FileType("SC2", "*.sc2"));
    fileTypeValues.push_back(FileType("TEX", "*.tex"));
}

void LibraryWidget::SetupSignals()
{
    QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectClosed()), this, SLOT(ProjectClosed()));
    
    QObject::connect(filesView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));
    QObject::connect(filesView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
}

void LibraryWidget::SetupToolbar()
{
    toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setMovable(false);

    searchFilter = new QLineEdit(toolbar);
    searchFilter->setToolTip("Enter text to search something at tree");
    searchFilter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    
    filesTypeFilter = new QComboBox(toolbar);
    filesTypeFilter->setEditable(false);
    filesTypeFilter->setMinimumWidth(100);
    filesTypeFilter->setMaximumWidth(100);
    for(int i = 0; i < fileTypeValues.size(); ++i)
    {
        filesTypeFilter->addItem(fileTypeValues[i].name);
    }
    filesTypeFilter->setCurrentIndex(0);
    
    
    QIcon resetIcon(QString::fromUtf8(":/QtIcons/reset.png"));
    QAction *actionResetFilter = new QAction(resetIcon, "Reset search filter", toolbar);

    QIcon asListIcon(QString::fromUtf8(":/QtIconsTextureDialog/view_list.png"));
    actionViewAsList = new QAction(asListIcon, "View as list", toolbar);
    actionViewAsList->setCheckable(true);
    actionViewAsList->setChecked(true);

    QIcon asDetailedIcon(QString::fromUtf8(":/QtIcons/all.png"));
    actionViewDetailed = new QAction(asDetailedIcon, "View detailed", toolbar);
    actionViewDetailed->setCheckable(true);
    actionViewDetailed->setChecked(false);

    QObject::connect(searchFilter, SIGNAL(editingFinished()), this, SLOT(SetFilter()));
    QObject::connect(actionResetFilter, SIGNAL(triggered()), this, SLOT(ResetFilter()));
    QObject::connect(filesTypeFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFilesTypeChanged(int)));
    QObject::connect(actionViewAsList, SIGNAL(triggered()), this, SLOT(ViewAsList()));
    QObject::connect(actionViewDetailed, SIGNAL(triggered()), this, SLOT(ViewDetailed()));
    
    toolbar->addWidget(searchFilter);
    toolbar->addAction(actionResetFilter);
    toolbar->addSeparator();
    toolbar->addWidget(filesTypeFilter);
    
    toolbar->addAction(actionViewAsList);
    toolbar->addAction(actionViewDetailed);
}

void LibraryWidget::SetupView()
{
    filesModel = new LibraryFileSystemModel(this);
    proxyModel = new LibraryFilteringModel(this);

    filesView = new QTreeView(this);
    filesView->setContextMenuPolicy(Qt::CustomContextMenu);
    filesView->header()->setVisible(false);
    filesView->setDragDropMode(QAbstractItemView::DragOnly);
	filesView->setDragEnabled(true);
    filesView->setUniformRowHeights(true);
    
    filesView->setModel(NULL);
    
    QObject::connect(filesModel, SIGNAL(ModelLoaded()), this, SLOT(OnModelLoaded()));
    
    waitBar = new QProgressBar(this);
    waitBar->setMinimumHeight(20);
    waitBar->setMinimum(0);
    waitBar->setMaximum(0);
    
    notFoundMessage = new QLabel("Nothing found", this);
    notFoundMessage->setMinimumHeight(20);
    notFoundMessage->setMaximumHeight(20);
    notFoundMessage->setAlignment(Qt::AlignCenter);
}

void LibraryWidget::SetupLayout()
{
    // put tab bar and davawidget into vertical layout
	layout = new QVBoxLayout();
	layout->addWidget(toolbar);
	layout->addWidget(waitBar);
	layout->addWidget(notFoundMessage);
	layout->addWidget(filesView);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);
    
    waitBar->setVisible(false);
    notFoundMessage->setVisible(false);
    filesView->setVisible(true);
}


void LibraryWidget::ViewAsList()
{
    viewMode = VIEW_AS_LIST;
    
    HideDetailedColumnsAtFilesView(true);
    
    actionViewAsList->setChecked(true);
    actionViewDetailed->setChecked(false);
}

void LibraryWidget::ViewDetailed()
{
    viewMode = VIEW_DETAILED;
    
    // Magic trick for MacOS: call funciton twice
    HideDetailedColumnsAtFilesView(false);
    HideDetailedColumnsAtFilesView(false);
    //EndOftrick

    actionViewAsList->setChecked(false);
    actionViewDetailed->setChecked(true);
}

void LibraryWidget::HideDetailedColumnsAtFilesView(bool hide)
{
    int columns = (hide) ? 1 : proxyModel->columnCount();
    int width = filesView->geometry().width() / columns;
    
    if(!hide)
    {
        filesView->setColumnWidth(0, width);
    }
    
    for(int i = 1; i < proxyModel->columnCount(); ++i)
	{
        filesView->setColumnHidden(i, hide);
        filesView->setColumnWidth(i, width);
	}
}


void LibraryWidget::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(0 == selected.count()) return;

    const QItemSelection realSelection = proxyModel->mapSelectionToSource(selected);
    const QModelIndex index = realSelection.indexes().first();

    QFileInfo fileInfo = filesModel->fileInfo(index);

    if(0 == fileInfo.suffix().compare("sc2", Qt::CaseInsensitive))
    {
        ShowPreview(fileInfo.filePath());
    }
    else
    {
        HidePreview();
    }
}


void LibraryWidget::ShowContextMenu(const QPoint & point)
{
    HidePreview();

    const QModelIndex index = proxyModel->mapToSource(filesView->indexAt(point));
    
	if(!index.isValid()) return;
    
    QFileInfo fileInfo = filesModel->fileInfo(index);
    if(!fileInfo.isFile()) return;

    QMenu contextMenu(this);
    QVariant fileInfoAsVariant = QVariant::fromValue<QFileInfo>(fileInfo);

    DAVA::FilePath pathname = fileInfo.absoluteFilePath().toStdString();
    if(pathname.IsEqualToExtension(".sc2"))
    {
        QAction * actionAdd = contextMenu.addAction("Add Model", this, SLOT(OnAddModel()));
        QAction * actionEdit = contextMenu.addAction("Edit Model", this, SLOT(OnEditModel()));
        
        actionAdd->setData(fileInfoAsVariant);
        actionEdit->setData(fileInfoAsVariant);
    }
    else if(pathname.IsEqualToExtension(".dae"))
    {
        QAction * actionConvert = contextMenu.addAction("Convert", this, SLOT(OnConvertDae()));
        QAction * actionConvertGeometry = contextMenu.addAction("Convert geometry", this, SLOT(OnConvertGeometry()));
        
        actionConvert->setData(fileInfoAsVariant);
        actionConvertGeometry->setData(fileInfoAsVariant);
    }
    else if(pathname.IsEqualToExtension(".tex"))
    {
        QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEditTextureDescriptor()));
        actionEdit->setData(fileInfoAsVariant);
    }
    else if(pathname.IsEqualToExtension(".png"))
    {
        QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEditTextureDescriptor()));
        actionEdit->setData(fileInfoAsVariant);
    }
    
    
    contextMenu.addSeparator();
    QAction * actionRevealAt = contextMenu.addAction("Reveal at folder", this, SLOT(OnRevealAtFolder()));
    actionRevealAt->setData(fileInfoAsVariant);

    
    contextMenu.exec(filesView->mapToGlobal(point));
}

void LibraryWidget::SetFilter()
{
    QString filter = searchFilter->text();
    
    proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));

    if(filesView->model())
    {
        filesView->setRootIndex(proxyModel->mapFromSource(filesModel->index(rootPathname)));
        if(!filter.isEmpty())
        {
            for(int i = 0; i < proxyModel->rowCount(); ++i)
            {
                ExpandUntilFilterAccepted(proxyModel->index(i, 0));
            }
        }
        SwitchTreeAndLabel();
    }
}

void LibraryWidget::ResetFilter()
{
    searchFilter->setText("");
    SetFilter();
}


void LibraryWidget::OnFilesTypeChanged(int typeIndex)
{
    filesModel->SetExtensionFilter(fileTypeValues[typeIndex].filter);
    
    if(filesView->model())
    {
        filesView->setRootIndex(proxyModel->mapFromSource(filesModel->index(rootPathname)));
        SwitchTreeAndLabel();
    }
}


void LibraryWidget::ProjectOpened(const QString &path)
{
    rootPathname = path + "/DataSource/3d/";
    
    filesView->setModel(NULL);
    proxyModel->SetModel(NULL);

    if(filesView->isVisible())
    {
        AddSpacer();
    }
    
    waitBar->setVisible(true);
    notFoundMessage->setVisible(false);
    filesView->setVisible(false);
    
    filesModel->Load(rootPathname);
}

void LibraryWidget::ProjectClosed()
{
    ResetFilter();
    
    rootPathname = "";
    filesView->setRootIndex(proxyModel->mapFromSource(filesModel->index(rootPathname)));
    filesView->collapseAll();

    if(filesView->isVisible() == false)
    {
        RemoveSpacer();
        
        waitBar->setVisible(false);
        notFoundMessage->setVisible(false);
        filesView->setVisible(true);
    }
}

void LibraryWidget::OnModelLoaded()
{
    QDir rootDir(rootPathname);
    
    proxyModel->SetModel(filesModel);
    filesView->setModel(proxyModel);
    filesView->setRootIndex(proxyModel->mapFromSource(filesModel->index(rootPathname)));

    RemoveSpacer();
    waitBar->setVisible(false);
    notFoundMessage->setVisible(false);
    filesView->setVisible(true);
    
    if(VIEW_AS_LIST == viewMode)
    {
        ViewAsList();
    }
    else
    {
        ViewDetailed();
    }
}

void LibraryWidget::SwitchTreeAndLabel()
{
    if(proxyModel->rowCount())
    {
        if(filesView->isVisible() == false)
        {
            notFoundMessage->setVisible(false);
            filesView->setVisible(true);
            RemoveSpacer();
        }
    }
    else
    {
        if(filesView->isVisible())
        {
            notFoundMessage->setVisible(true);
            filesView->setVisible(false);
            AddSpacer();
        }
    }
}

void LibraryWidget::OnAddModel()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    SceneEditor2 *scene = QtMainWindow::Instance()->GetCurrentScene();
    if(NULL != scene)
    {
        QtMainWindow::Instance()->WaitStart("Add object to scene", fileInfo.absoluteFilePath());
        
        scene->structureSystem->Add(fileInfo.absoluteFilePath().toStdString());
        
        QtMainWindow::Instance()->WaitStop();
    }
}

void LibraryWidget::OnEditModel()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->OpenScene(fileInfo.absoluteFilePath());
}

void LibraryWidget::OnConvertDae()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->WaitStart("DAE to SC2 conversion", fileInfo.absoluteFilePath());
    
    Command2 *daeCmd = new DAEConvertAction(fileInfo.absoluteFilePath().toStdString());
    daeCmd->Redo();
    delete daeCmd;
    
    QtMainWindow::Instance()->WaitStop();
}

void LibraryWidget::OnConvertGeometry()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->WaitStart("DAE to SC2 conversion of geometry", fileInfo.absoluteFilePath());
    
    Command2 *daeCmd = new DAEConvertWithSettingsAction(fileInfo.absoluteFilePath().toStdString());
    daeCmd->Redo();
    delete daeCmd;
    
    QtMainWindow::Instance()->WaitStop();
}

void LibraryWidget::OnEditTextureDescriptor()
{
    
}

void LibraryWidget::OnRevealAtFolder()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
#if defined (Q_WS_MAC)
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \""+fileInfo.absoluteFilePath()+"\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#elif defined (Q_WS_WIN)
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    QProcess::startDetached("explorer", args);
#endif//

}

void LibraryWidget::HidePreview() const
{
    SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
    widget->HideScenePreview();
}

void LibraryWidget::ShowPreview(const QString & pathname) const
{
    SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
    widget->ShowScenePreview(pathname.toStdString());
}

bool LibraryWidget::ExpandUntilFilterAccepted(const QModelIndex &proxyIndex)
{
    bool childExpanded = false;
    for(int i = 0; i < proxyModel->rowCount(proxyIndex); ++i)
    {
        childExpanded |= ExpandUntilFilterAccepted(proxyModel->index(i, 0, proxyIndex));
    }

    bool wasExpanded = childExpanded;
    if(filesModel->IsAccepted(proxyModel->mapToSource(proxyIndex)))
    {
        QModelIndex index = proxyIndex.parent();
        while(index.isValid())
        {
            filesView->expand(index);
            
            index = index.parent();
        }
        
        wasExpanded = true;
    }
    if(!childExpanded)
    {
        filesView->collapse(proxyIndex);
    }
    
    return wasExpanded;
}

void LibraryWidget::AddSpacer()
{
    DVASSERT(spacer == NULL);
    
    spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addSpacerItem(spacer);
}

void LibraryWidget::RemoveSpacer()
{
    DVASSERT(spacer);
    
    layout->removeItem(spacer);
    delete spacer;
    spacer = NULL;
}