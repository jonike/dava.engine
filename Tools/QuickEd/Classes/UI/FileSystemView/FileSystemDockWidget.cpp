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


#include "FileSystemDockWidget.h"

#include "ui_FileSystemDockWidget.h"
#include <DAVAEngine.h>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QDirIterator>

#include "QtTools/FileDialog/FileDialog.h"

namespace
{
const QString yamlExtensionString = ".yaml";
const QString defaultDialogLabel = "Enter new folder name:";
class FileSystemModel : public QFileSystemModel
{

public:
    FileSystemModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & idx, const QVariant & value, int role = Qt::EditRole) override;
};
} //unnamed namespace


FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{

}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant FileSystemModel::data(const QModelIndex & index, int role) const
{
    QVariant data = QFileSystemModel::data(index, role);
    if (index.isValid() && role == Qt::EditRole && !isDir(index) && data.canConvert<QString>())
    {
        return data.toString().remove(QRegularExpression(yamlExtensionString + "$"));
    }
    return data;
}

bool FileSystemModel::setData(const QModelIndex & idx, const QVariant & value, int role)
{
    if (value.canConvert<QString>())
    {
        QString name = value.toString();
        if (!name.endsWith(yamlExtensionString))
        {
            return QFileSystemModel::setData(idx, name + yamlExtensionString, role);
        }
    }
    return QFileSystemModel::setData(idx, value, role);
}

FileSystemDockWidget::FileSystemDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::FileSystemDockWidget())
    , model(new FileSystemModel(this))
{
    ui->setupUi(this);
    ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    model->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList filters;
    filters << "*" + yamlExtensionString;
    model->setNameFilters(filters);
    model->setNameFilterDisables(false);
    model->setReadOnly(false);
    
    connect(ui->treeView, &QTreeView::doubleClicked, this, &FileSystemDockWidget::onDoubleClicked);
    connect(ui->filterLine, &QLineEdit::textChanged, this, &FileSystemDockWidget::setFilterFixedString);


    newFolderAction = new QAction(tr("Create folder"), this);
    connect(newFolderAction, &QAction::triggered, this, &FileSystemDockWidget::onNewFolder);

    newFileAction = new QAction(tr("Create file"), this);
    connect(newFileAction, &QAction::triggered, this, &FileSystemDockWidget::onNewFile);

    deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
    deleteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(deleteAction, &QAction::triggered, this, &FileSystemDockWidget::onDeleteFile);
    
#if defined Q_OS_WIN
    QString actionName = tr("Show in explorer");
#else if defined Q_OS_MAC
    QString actionName = tr("Show in finder");
#endif //Q_OS_WIN //Q_OS_MAC
    showInSystemExplorerAction = new QAction(actionName, this);
    connect(showInSystemExplorerAction, &QAction::triggered, this, &FileSystemDockWidget::OnShowInExplorer);
    
    renameAction = new QAction(tr("Rename"), this);
    connect(renameAction, &QAction::triggered, this, &FileSystemDockWidget::OnRename);
    
    openFileAction = new QAction(tr("Open File"), this);
    openFileAction->setShortcuts({ QKeySequence(Qt::Key_Return), QKeySequence(Qt::Key_Enter)});
    openFileAction->setShortcutContext(Qt::WidgetShortcut);
    connect(openFileAction, &QAction::triggered, this, &FileSystemDockWidget::OnOpenFile);

    ui->treeView->addAction(newFolderAction);
    ui->treeView->addAction(newFileAction);
    ui->treeView->addAction(deleteAction);
    ui->treeView->addAction(showInSystemExplorerAction);
    ui->treeView->addAction(renameAction);
    ui->treeView->addAction(openFileAction);
}

FileSystemDockWidget::~FileSystemDockWidget() = default;

void FileSystemDockWidget::SetProjectDir(const QString &path)
{
    if (ui->treeView->selectionModel())
    {
        disconnect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
    }
    QDir dir(path);
    dir.cdUp();
    QString p = dir.path() + "/Data/UI";
    QModelIndex rootIndex = model->setRootPath(p);
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(rootIndex);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    
    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(OnSelectionChanged(const QItemSelection &, const QItemSelection &)));
    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
}

void FileSystemDockWidget::RefreshActions( const QModelIndexList &indexList )
{
    bool canCreateDir = true;
    bool canRemove = !indexList.empty();
    bool canOpen = false;
    if (indexList.size() == 1)
    {
        QModelIndex selectedIndex = indexList.front();
        bool isDir = model->isDir(selectedIndex);
        canCreateDir = isDir;
        canRemove = CanRemove(selectedIndex);
        canOpen = !isDir;
        deleteAction->setText(isDir ? "Remove folder" : "Remove file");
    }
    else
    {
        deleteAction->setText("Remove selected items");
        for (const auto &index : indexList)
        {
            canRemove &= CanRemove(index);
        }
    }
    newFolderAction->setEnabled(canCreateDir);
    deleteAction->setEnabled(canRemove);
    openFileAction->setEnabled(canOpen);
    openFileAction->setVisible(canOpen);
}

bool FileSystemDockWidget::CanRemove(const QModelIndex& index) const
{
    if (!model->isDir(index))
    {
        return true;
    }
    QDir dir(model->filePath(index));
    QDirIterator dirIterattor(dir, QDirIterator::Subdirectories);
    while (dirIterattor.hasNext())
    {
        if (dirIterattor.next().endsWith(yamlExtensionString))
        {
            return false;
        }
    }
    return true;
}

bool FileSystemDockWidget::ValidateInputDialogText(QInputDialog* dialog, const QString& text)
{
    const auto &selected = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    QString path = model->filePath(selected.front()) + "/";

    const QObjectList &children = dialog->children();
    auto iter = std::find_if(children.begin(), children.end(), [](const QObject* obj)
    {
        return qobject_cast<const QLineEdit*>(obj) != nullptr;
    });
    if (iter == children.end())
    {
        Logger::Warning("create folder inpud dialog: can not find lineedit");
        return false;
    }
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(*iter);
    iter = std::find_if(children.begin(), children.end(), [](const QObject* obj)
    {
        return qobject_cast<const QDialogButtonBox*>(obj) != nullptr;
    });
    if (iter == children.end())
    {
        Logger::Warning("create folder inpud dialog: can not find button box");
        return false;
    }
    QDialogButtonBox *buttonBox = qobject_cast<QDialogButtonBox*>(*iter);

    QPalette palette(lineEdit->palette());
    bool enabled = true;
    if (QFileInfo::exists(path + text))
    {
        dialog->setLabelText(defaultDialogLabel + "\nthis folder already exists");
        palette.setColor(QPalette::Text, Qt::red);
        enabled = false;
    }
    else
    {
        dialog->setLabelText(defaultDialogLabel);
        palette.setColor(QPalette::Text, Qt::black);
        enabled = true;
    }
    lineEdit->setPalette(palette);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(enabled);
    return enabled;
}

void FileSystemDockWidget::OnSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
    RefreshActions(selected.indexes());
}

void FileSystemDockWidget::onDoubleClicked(const QModelIndex &index)
{
    if(!model->isDir(index))
    {
        emit OpenPackageFile(model->filePath(index));
    }
}

void FileSystemDockWidget::setFilterFixedString( const QString &filterStr )
{
    QStringList filters;
    filters << QString("*%1*" + yamlExtensionString).arg(filterStr);
    model->setNameFilters(filters);
}

void FileSystemDockWidget::onNewFolder()
{
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("New folder"));
    dialog.setLabelText(defaultDialogLabel);
    dialog.setTextValue(tr("New folder"));
    dialog.setTextEchoMode(QLineEdit::Normal);
    dialog.setInputMethodHints(Qt::ImhUrlCharactersOnly);

    connect(&dialog, &QInputDialog::textValueChanged, this, &FileSystemDockWidget::OnInputDialogTextChanged);
    dialog.okButtonText(); //force ensure layout of dialog
    
    if (!ValidateInputDialogText(&dialog, dialog.textValue()))
    {
        int i = 1; 
        do
        {
            dialog.setTextValue(tr("new folder (%1)").arg(i++));

        } while (!ValidateInputDialogText(&dialog, dialog.textValue()));
    }

    int ret = dialog.exec();
    QString folderName = dialog.textValue();
    if (ret == QDialog::Accepted && !folderName.isEmpty())
    {
        if (QFileInfo::exists(folderName))
        {
            
        }
        auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
        DVASSERT(selectedIndexes.empty() || selectedIndexes.size() == 1);
        QModelIndex currIndex = selectedIndexes.empty() ? ui->treeView->rootIndex() : selectedIndexes.front();
        model->mkdir(currIndex, folderName);
    }
    auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    RefreshActions(selectedIndexes);
}

void FileSystemDockWidget::onNewFile()
{
    auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selectedIndexes.empty() || selectedIndexes.size() == 1);
    QModelIndex currIndex = selectedIndexes.empty() ? ui->treeView->rootIndex() : selectedIndexes.front();

    QString folderPath = model->filePath(currIndex);
    QString strFile = FileDialog::getSaveFileName(this, tr("Create new file"), folderPath, "*" + yamlExtensionString);
    if (strFile.isEmpty())
    {
        return;
    }
    QFileInfo fileInfo(strFile);
    if (fileInfo.suffix().toLower() != QString(yamlExtensionString).remove('.'))
    {
        strFile += yamlExtensionString;
    }

    QFile file(strFile);
    file.open(QIODevice::WriteOnly);
    file.close();
    RefreshActions(selectedIndexes);
}

void FileSystemDockWidget::onDeleteFile()
{
    const QModelIndexList &indexes = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(indexes.size() == 1);
    auto index = indexes.front();
    bool isDir = model->isDir(index);
    QString text = tr("Delete ") + (isDir ? "folder" : "file") + " \"" + model->fileName(index) + "\"" + (isDir ? " and its content" : "") + "?";
    if (QMessageBox::Yes == QMessageBox::question(this, text, text, QMessageBox::Yes | QMessageBox::No))
    {
        if (!model->remove(index))
        {
            DAVA::Logger::Error("can not remove file %s", model->isDir(index) ? "folder" : "file", model->fileName(index).toUtf8().data());
        }
}
    RefreshActions(indexes);
}

void FileSystemDockWidget::OnShowInExplorer()
{
    const QModelIndexList &indexes = ui->treeView->selectionModel()->selectedIndexes();
    if(indexes.size() != 1)
    {
        return;
    }
    QString pathIn = model->fileInfo(indexes.first()).absoluteFilePath();
#ifdef Q_OS_MAC
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \""+pathIn+"\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#endif
#ifdef Q_OS_WIN
    QString param;
    param = QLatin1String("/select,");
    param += QDir::toNativeSeparators(pathIn);
    QString command = QString("explorer") + " " + param;
    QProcess::startDetached(command);
#endif
}

void FileSystemDockWidget::OnRename()
{
    const auto &selected = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    ui->treeView->edit(selected.first());
}

void FileSystemDockWidget::OnOpenFile()
{
    const auto &selected = ui->treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    onDoubleClicked(selected.first());
}

void FileSystemDockWidget::OnInputDialogTextChanged(const QString& text)
{
    QInputDialog *dialog = qobject_cast<QInputDialog*>(sender());
    ValidateInputDialogText(dialog, text);
}
