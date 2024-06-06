#ifndef MYMAPGRAPHICSSCENE_H
#define MYMAPGRAPHICSSCENE_H

#include <QObject>
#include <QGraphicsScene>
#include <QDir>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QJsonObject>

#include "unit.h"
#include "drawing.h"
#include "order.h"

class Data;
class MainWindow;
class VEPolyline;

QT_BEGIN_NAMESPACE
class QGraphicsSimpleTextItem;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////
/// PUBLIC  DATA STRUCTURES                                         ///
///////////////////////////////////////////////////////////////////////
/// Qt doesn't support nested QObjects
class MapUnitItem : public QObject, public QGraphicsPixmapItem{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    enum { Type = 200 };
public:
    MapUnitItem(int id, const QPixmap & pixmap, QGraphicsItem * parent = nullptr);

    // Getter
    int uniqueId() const;

    // QGraphicsItem interface
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;



    int type() const {return Type;}

public:
signals:
    void unitMoved(int uniqueId, QPointF pos);
    void orderUpdatedByUser(Order order);

    void hoverEntered();
    void hoverLeft();

public:
    int mUniqueId;          // ID of the unit
    QPointF mDeltaClick;    // Delta between the position of the click and the (0,0) of the unit

    QGraphicsSimpleTextItem * mDistanceItem;
    qreal mOldZValue;
    Qt::MouseButton mMouseButton;
};









///////////////////////////////////////////////////////////////////////
/// MyMapGraphicsScene                                              ///
///////////////////////////////////////////////////////////////////////
class MyMapGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

    //VE
    Q_PROPERTY(int currentAction READ currentAction WRITE setCurrentAction NOTIFY currentActionChanged)
    Q_PROPERTY(QPointF clickDownPosition READ clickDownPosition WRITE setClickDownPosition NOTIFY clickDownPositionChanged)

public:
    enum DrawingTypes {
        DefaultType,
        LineType,
        RectangleType,
        EllipseType,
        SelectionType
    };

    enum MAP_MODE{
        UNIT_MODE,
        DRAWING_MODE
    };


public:
    MyMapGraphicsScene(Data * data, bool isAdmin, QObject * parent = nullptr);
    void setupHover();
    void clear();
    void clearUnits();
    void clearOrders();

    // GETTERS SETTERS
public:
    int currentAction() const;
        void setCurrentAction(const int type);

    QPointF clickDownPosition() const;
    void setClickDownPosition(const QPointF clickDownPosition);

    QString getSessionName() const;
    void setSessionName(const QString &sessionName);

    bool isAdmin() const;
    bool isObserver() const;
    bool adminView() const;
    bool playerView() const;

//    bool drawingMode() const;
    MAP_MODE currentMode() const;

    // Change mode
public slots:
    void setUnitMode();
    void setDrawingMode();

    // Background
public slots:
    void loadMap(QDir dir);

    // Units
public slots:
    void addUniqueId(int uniqueId);
    void addUniqueIdWithCustomIcon(int uniqueId, const QString & app6, const QString &displayedText, int life = 100);
    void deleteUniqueId(int name);
    void updateUnitContent(Unit unit);
    void updateUnitPosition(int uniqueId, QPointF pos);
    void changeUnitParent(int parentId, int uniqueId);
    void removeUnitParent(int uniqueId);
    void selectUnit(int uniqueId);

    QGraphicsItem * getUnit(int uniqueId);

    // Drawings
    QGraphicsItem *createDrawing(Drawing drawing);
    void deleteDrawing(int uniqueId);
    void updateDrawing(Drawing drawing);

    // Display
public slots:
    void displayAll();
    void displayMicro();
    void displayMacro();

private:
    void displayPrivate(const QMap<int, bool> & unitsVisibleAtThisHierarchyLevel);


    // Selection
public slots:
    void mySelectionChanged();

private slots:
    void deselectItems();

    // Visibility
public slots:
    void setUnitVisible(MapUnitItem * item, bool b = true);

    void clearUnitVisibility();
    void setUnitVisibility(int uniqueId, const QString & app6, const QString &displayedText);
    void setUnitVisibilityLife(int uniqueId, const QString & app6, const QString &displayedText, int life);
    void setAdminUnitVisibility();

    // Order
public slots:
    void updateOrder(Order order);

    void showOrders(int unitUniqueId);
    void hideAllOrders();

public:
    QTransform getTransform();


    // Hover
public:
    void setHoverPosition(QPointF pos);
    void setHoverRadius(qreal r);

public slots:
    void unitHoverEnter();
    void unitHoverLeave();



    // Misc
public:
    QPointF convertToRealPos(QPointF scenePos);
    QPointF convertToScenePos(QPointF realPos);


    void debug();
    int getDrawingUniqueId(QGraphicsItem * item);
    qreal getGraphicsViewScale();


public slots:
    void moveDrawingSelection(QGraphicsItem *signalOwner, qreal dx, qreal dy);


    //    QJsonObject getControlMeasures() const;
    //    void setControlMeasures(QJsonObject & json);
    //    QJsonObject getIntentions() const;





    // QGraphicsScene interface
public:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

protected:
    void mousePressEventUnitMode(QGraphicsSceneMouseEvent *event);
    void mouseMoveEventUnitMode(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEventUnitMode(QGraphicsSceneMouseEvent *event);

    void mousePressEventDrawingMode(QGraphicsSceneMouseEvent *event);
    void mouseMoveEventDrawingMode(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEventDrawingMode(QGraphicsSceneMouseEvent *event);

protected:
    void keyPressEvent(QKeyEvent *event) override;

    //public:
    //    void mousePressCreateOrderEvent(QGraphicsSceneMouseEvent *event, int uniqueId) ;
    //    void mouseMoveCreateOrderEvent(QGraphicsSceneMouseEvent *event, int uniqueId) ;
    //    void mouseReleaseCreateOrderEvent(QGraphicsSceneMouseEvent *event, int uniqueId) ;



    //    void userMovedUnit(int uniqueId, QPointF pos);



signals:
    void unitMoved(int uniqueId, QPointF pos);
    void unitDropped(int uniqueId, QPointF pos);
    void unitSelected(int uniqueId);


    //VE


signals:
    void clickDownPositionChanged();
    void currentActionChanged(int);

    void drawingSelected(Drawing drawing);
//    void drawingCreatedByUser(QGraphicsItem *item, Drawing drawing);
    void drawingCreatedByUser(Drawing drawing);
    void drawingUpdatedByUser(Drawing drawing);
    void drawingDeletedByUser(int uniqueId);
    void drawingFinishedByUser();




private:
    Data * mData;
    QMap<int, MapUnitItem *> mUnitItems;
    QMap<int, QGraphicsItem*> mDrawings;
    QMap<int, QMap<Order::ORDER_TYPE, QGraphicsItem*> > mOrders;

    double mTop;
    double mBottom;
    double mLeft;
    double mRight;

//    int mIdUserMoved;       // The ID of the unit/drawing/order currently moved

    bool mIsDisplayingAll;  // Admin view
    int mDisplayLevel;      //  Platoon/Company/Battalion/...

    QDir mOldDir;
    QPixmap mBackgroundPixmap; // Needs to become a tiling

    MAP_MODE mMapMode;

    MainWindow * mMainWindow;

    QGraphicsEllipseItem * mHoverItemSpeed;     // Large blue circle
    QGraphicsEllipseItem * mHoverItemCautious;  // Small red circle


    //DRAWING specific
private:
    int idOfDrawingInConstruction;              // Unique ID (grabbed from Data) between press and release
    int mCurrentDrawingAction;                  //  Selection, line, ellipse, rectangle
    QPointF mClickDownPosition;                 // Where did the click happen
    bool mLeftMouseButtonPressed;               // Was it with left click

    // VISIBILILTY
private:
};

#endif // MYMAPGRAPHICSSCENE_H
