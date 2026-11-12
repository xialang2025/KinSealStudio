#ifndef HMI_CANVAS_H
#define HMI_CANVAS_H

#include "project_model.h"

#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QPoint>
#include <QString>
#include <QVariant>

class QGraphicsScene;
class QMouseEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QContextMenuEvent;
class QKeyEvent;

class HmiControlItem : public QGraphicsRectItem
{
public:
    explicit HmiControlItem(const HmiObject &object);
    HmiObject toHmiObject() const;
    void setObject(const HmiObject &object);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QString m_id;
    QString m_type;
    QString m_name;
    QString m_text;
    QString m_address;
    HmiObject m_record; //20261108-14 add: Preserve all appearance, language and permission properties.
};

class ScreenCanvas : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ScreenCanvas(QWidget *parent = nullptr);
    void setProject(const HmiProject &project);
    QList<HmiObject> objects() const;
    HmiObject selectedObject(bool *selected) const;
    void addControl(const QString &type, const QPointF &position);
    void updateSelectedObject(const HmiObject &object);
    void setZoomPercent(int percent);
    void setActiveTool(const QString &type);
    void selectObject(const QString &id);
    void selectObjects(const QStringList &ids); //20261110-8 add: Stable programmatic selection without exposing graphics items.
    bool hasPendingPlacement() const;
    bool hasActiveInteraction() const;
    void cancelInteraction();
    void beginStaticTextPlacement(const HmiObject &object); //20261108-14 add
    void beginControlPlacement(const HmiObject &object); //20261108-15 add
    void executeEdit(const QString &command);
    void requestEditCommand(const QString &command); //20261110-7 add: Route hosted menus and keys through the same whole-project transaction.
    void setExternalEditRouting(bool enabled) { m_externalEditRouting = enabled; }
    bool canExecuteEdit(const QString &command) const;
    QStringList selectedObjectIds() const;
    QString arrangementReferenceObjectId(const QString &command) const; //20261110-8 add: Expose the actual operand instead of inventing a native primary-selection rule.
    void setArrangementReferenceObject(const QString &id);
    void setLanguage(const QString &language);
    QString language() const { return m_language; }

signals:
    void objectSelectionChanged(const HmiObject &object, bool selected);
    void pageCoordinatesChanged(const QPoint &point);
    void documentChanged();
    void controlLimitReached();
    void editObjectRequested(); //20261108-14 add
    void editCommandRequested(const QString &command); //20261110-7 add
    void interactionCancelled(); //20261110-8 add: Cancel pending resources without creating an edit transaction.

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    QGraphicsScene *m_scene;
    QGraphicsRectItem *m_pageItem;
    QString m_activeTool;
    int m_pageWidth;
    int m_pageHeight;
    int m_zoomPercent;
    HmiObject m_previousSelection;
    bool m_previousSelectionValid;
    QString m_language = QStringLiteral("中文"); //20261108-14 add
    HmiObject m_pendingObject;
    bool m_hasPending = false;
    bool m_drawing = false;
    QPointF m_dragOrigin;
    QGraphicsRectItem *m_draftRect = nullptr;
    QString m_resizeId;
    QRect m_resizeOriginal;
    int m_resizeHandle = 0;
    bool m_grid = false;
    QList<HmiObject> m_clipboard;
    QList<QList<HmiObject>> m_history;
    int m_historyIndex = -1;
    bool m_restoring = false;
    bool m_externalEditRouting = false; //20261110-7 add: Standalone canvases retain their existing local command behavior.
    bool m_moving = false; //20261110-8 add: Whole-selection movement uses one shared delta and one history transaction.
    QPointF m_moveOrigin;
    QRect m_moveBounds;
    QList<HmiObject> m_interactionOriginal;
    QStringList m_interactionSelection;
    QString m_arrangementReferenceId; //20261110-8 add: Optional explicit reference; empty keeps the observed native list-first/list-last policy.

    QMap<QString, QStringList> m_recipeTitles; //20261113-2 add: Transient resolved context is never copied into HmiObject or KSP.
    QMap<QString, int> m_recipeCounts;
    HmiControlItem *makeControlItem(const HmiObject &object);
    void bindRecipeContext(HmiControlItem *item);
    void rebuildPage(int width, int height, bool preserveItems);
    void publishSelection();
    void recordHistory();
    void restoreObjects(const QList<HmiObject> &objects);
    bool beginResize(const QPointF &point);
    void updateResize(const QPointF &point);
    void updateMove(const QPointF &point);
    bool publishDocumentChange(const QList<HmiObject> &before);
};

#endif
