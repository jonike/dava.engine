#ifndef __QUICKED_PREVIEW_WIDGET_H__
#define __QUICKED_PREVIEW_WIDGET_H__

#include "ui_PreviewWidget.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/SelectionContainer.h"
#include <QWidget>
#include <QCursor>
#include <QPointer>

namespace Ui
{
class PreviewWidget;
}
class EditorSystemsManager;

class Document;
class DavaGLWidget;
class ControlNode;
class ScrollAreaController;
class PackageBaseNode;
class RulerController;
class AbstractProperty;
class QWheelEvent;
class QNativeGestureEvent;
class QDragMoveEvent;
class ContinuousUpdater;
class QDragLeaveEvent;
class QDropEvent;

class PreviewWidget : public QWidget, public Ui::PreviewWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    ~PreviewWidget();
    DavaGLWidget* GetGLWidget() const;
    ScrollAreaController* GetScrollAreaController();
    float GetScale() const;
    RulerController* GetRulerController();
    ControlNode* OnSelectControlByMenu(const DAVA::Vector<ControlNode*>& nodes, const DAVA::Vector2& pos);

signals:
    void DeleteRequested();
    void ImportRequested();
    void CutRequested();
    void CopyRequested();
    void PasteRequested();
    void SelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OpenPackageFile(QString path);
    void DropRequested(const QMimeData* data, Qt::DropAction action, PackageBaseNode* targetNode, DAVA::uint32 destIndex, const DAVA::Vector2* pos);

public slots:
    void OnDocumentChanged(Document* document);
    void SaveSystemsContextAndClear();
    void LoadSystemsContext(Document* document);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnRootControlPositionChanged(const DAVA::Vector2& pos);
    void OnNestedControlPositionChanged(const QPoint& pos);
    void OnEmulationModeChanged(bool emulationMode);

private slots:
    void OnScaleChanged(qreal scale);
    void OnScaleByComboIndex(int value);
    void OnScaleByComboText();

    void OnGLWidgetResized(int width, int height);

    void OnVScrollbarMoved(int position);
    void OnHScrollbarMoved(int position);

    void UpdateScrollArea();
    void OnPositionChanged(const QPoint& position);
    void OnGLInitialized();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void LoadContext();
    void SaveContext();

    void CreateActions();
    void ApplyPosChanges();
    void OnWheelEvent(QWheelEvent* event);
    void OnNativeGuestureEvent(QNativeGestureEvent* event);
    void OnPressEvent(QMouseEvent* event);
    void OnReleaseEvent(QMouseEvent* event);
    void OnMoveEvent(QMouseEvent* event);
    void OnDragMoveEvent(QDragMoveEvent* event);
    bool ProcessDragMoveEvent(QDropEvent* event);
    void OnDragLeaveEvent(QDragLeaveEvent* event);
    void OnDropEvent(QDropEvent* event);
    void OnTransformStateChanged(bool inTransformState);

    qreal GetScaleFromWheelEvent(int ticksCount) const;
    qreal GetNextScale(qreal currentScale, int ticksCount) const;
    qreal GetPreviousScale(qreal currentScale, int ticksCount) const;

    void OnSelectionInSystemsChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnPropertiesChanged(const DAVA::Vector<ChangePropertyAction>& propertyActions, size_t hash);
    void NotifySelectionChanged();

    QPoint lastMousePos;
    QCursor lastCursor;
    QPointer<Document> document;
    DavaGLWidget* davaGLWidget = nullptr;
    ScrollAreaController* scrollAreaController = nullptr;
    QList<qreal> percentages;

    SelectionContainer selectionContainer;
    RulerController* rulerController = nullptr;
    QPoint rootControlPos;
    QPoint canvasPos;

    QAction* selectAllAction = nullptr;
    QAction* focusNextChildAction = nullptr;
    QAction* focusPreviousChildAction = nullptr;

    std::unique_ptr<EditorSystemsManager> systemsManager;

    ContinuousUpdater* continuousUpdater = nullptr;

    SelectedNodes tmpSelected; //for continuousUpdater
    SelectedNodes tmpDeselected; //for continuousUpdater
};

inline DavaGLWidget* PreviewWidget::GetGLWidget() const
{
    return davaGLWidget;
}

#endif // __QUICKED_PREVIEW_WIDGET_H__
