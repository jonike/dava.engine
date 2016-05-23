#ifndef __MATERIALS_MODEL_H__
#define __MATERIALS_MODEL_H__

#include "Render/Material/NMaterial.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QString>

class QMimeData;
class QStandardItem;
class SceneEditor2;
class MaterialItem;
class Command2;
class SelectableGroup;
struct TextureInfo;

class MaterialModel
: public QStandardItemModel
{
    Q_OBJECT

public:
    enum Columns
    {
        TITLE_COLUMN,
        LOD_COLUMN,
        SWITCH_COLUMN,
    };

public:
    MaterialModel(QObject* parent = 0);
    virtual ~MaterialModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    MaterialItem* itemFromIndex(const QModelIndex& index) const;

    void SetScene(SceneEditor2* scene);
    SceneEditor2* GetScene();
    void SetSelection(const SelectableGroup* group);
    DAVA::NMaterial* GetMaterial(const QModelIndex& index) const;
    QModelIndex GetIndex(DAVA::NMaterial* material, const QModelIndex& parent = QModelIndex()) const;

    DAVA::NMaterial* GetGlobalMaterial() const;

    void Sync();

    // drag and drop support
    QMimeData* mimeData(const QModelIndexList& indexes) const;
    QStringList mimeTypes() const;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    bool dropCanBeAccepted(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

protected:
    SceneEditor2* curScene;

    static const int supportedLodColorsCount = 4;
    static const int supportedSwColorsCount = 2;

    QColor lodColors[supportedLodColorsCount];
    QColor switchColors[supportedSwColorsCount];

private:
    void ReloadLodSwColors();
    bool SetItemSelection(MaterialItem* item, const SelectableGroup* group);
    void Sync(MaterialItem* item);
};

Q_DECLARE_METATYPE(DAVA::NMaterial*)

#endif // __MATERIALS_MODEL_H__
