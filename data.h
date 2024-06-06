#ifndef DATA_H
#define DATA_H


// CF GAUMé (EMO?)
// Mined point?
// Mined area? (bande minée)
// Point of Interest = POI?
// Obstacle free area = OFA?
// Zone d'effort de feu =
// Zone d'effort ALAT = HIDACZ = High density airspace control zone

/**
  * Note about QStandardItemModel
  * Each model has an invisibleRootItem (why not)
  * When an item has no parent (nullptr) it implies that its
  * parent is this root item
  * However, it doesn't work the other way around, because
  * the root has no idea if you just created a new item (which
  * by default will have nullptr as its parent)
  *
  * Note about the signal
  * Add unit just spawns a unit, no hierarchy stuff
  * If you receive a addUnit, just add its name, no content
  * Only update it when you receive the actual update signal
  *
  * About the unit data structure
  * You can only create new Ids
  * You can always update them later
  * You can replace as a long list of swaps
  * You can only delete an Id if it has no children
  *
  *
  *
  * Data holds all the data of a game
  * Data can be changed :
  *  - by local modifications (through user input)
  *  - by network calls
  * Every time a change is done, data signals and everything
  * connected to these signals will be updated
  * Interestingly, this means that some user input will warn
  * directly Data, and only through this signal will they
  * update their own UI
  *
  */



#include "drawing.h"
#include "player.h"
#include "unit.h"
#include "order.h"

#include <QObject>
#include <QMap>
#include <QVector>

#include <QPixmap>
#include <QImage>
#include <QDir>
#include <QMutex>
#include <QDateTime>

//class TreeModel;
//class TreeItem;
class MyStandardItemModel;
class MainWindow;
QT_BEGIN_NAMESPACE
class QStandardItem;
class QAbstractItemModel;
class QPainter;
class QPalette;
class QRect;
QT_END_NAMESPACE


class Data : public QObject
{
    Q_OBJECT

    ///////////////////////////////////////////////////////////////////////
    /// ENUMS                                                           ///
    ///////////////////////////////////////////////////////////////////////
public:
    enum SaveFormat {
        Json, Binary
    };
    
    ///////////////////////////////////////////////////////////////////////
    /// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
    ///////////////////////////////////////////////////////////////////////
public:
    Data(QObject * parent = nullptr);
    ~Data();
    void clear();
    
    void clearMap();
    void clearPlayers();
    void clearUnitsRoots();
    void clearOrders();
    void clearDrawings();


    ///////////////////////////////////////////////////////////////////////
    /// GETTERS, SETTERS                                                ///
    ///////////////////////////////////////////////////////////////////////
public:
    QMap<int, Unit *> units() const;
    QMap<int, Drawing *> drawings() const;
    QMap<int, QMap<Order::ORDER_TYPE, Order*> > getOrders() const;
    QMap<QString, Player *> getTruePlayers() const;

    QStringList getPlayerNames() const;






    ///////////////////////////////////////////////////////////////////////
    /// NETWORK / LOAD / SAVE                                           ///
    ///////////////////////////////////////////////////////////////////////
public slots:
    bool loadGame(QString filename, Data::SaveFormat saveFormat = Data::SaveFormat::Json);
    bool saveGame(QString filename, Data::SaveFormat saveFormat = Data::SaveFormat::Json);

    bool loadJson(const QJsonObject &json);
    void loadMap(const QJsonObject &json);
    void loadTime(const QJsonObject & json);
    void loadPlayers(const QJsonObject &json);
    void loadUnits(const QJsonObject &json);
    void loadUpdateToUnits(const QJsonObject &json);
    void loadOrders(const QJsonObject &json);
    void loadAdditionalOrders(const QJsonObject &json);
    void loadDrawings(const QJsonObject &json);

    void writeJson(QJsonObject &json, bool skipOrders = false) const;
    void writeMap(QJsonObject &json) const;
    void writeTime(QJsonObject &json) const;
    void writePlayers(QJsonObject &json) const;
    void writePlayerSides(QJsonObject &json) const;
    void writeUnits(QJsonObject &json) const;
    void writeOrders(QJsonObject &json) const;
    void writeDrawings(QJsonObject &json) const;

    QString serializeData(bool skipOrders = false);
    QString serializeUnits();

    ///////////////////////////////////////////////////////////////////////
    /// EXPLORING UNIT                                                  ///
    ///////////////////////////////////////////////////////////////////////
public:
    QVector<int> children(int uniqueId) const;
    QVector<int> siblings(int uniqueId) const;
    QVector<int> getDescendants(int uniqueId) const; // Includes self
    QVector<Unit> getFullDescendants(int uniqueId) const;
    int siblingRank(int uniqueId) const;  // Among your siblings, what is your rank

    QVector<int> getLeavesOnMap(int uniqueId) const;
    bool isALeafOnMap(int uniqueId) const;
    QMap<int, bool> unitsAtLevel(int level) const;
    QMap<int, bool> unitsAtLevel(int level, const QString & player) const;
    QPointF getUnitPosition(int uniqueId) const;

    ///////////////////////////////////////////////////////////////////////
    /// MANIPULATE UNIT                                                 ///
    ///////////////////////////////////////////////////////////////////////
    /// Create/Insert/Delete are rather straightforward
    /// A unit is made of three things, content, parent and children
    /// The content is the easiest, just call update
    /// To change children, also call update
    /// To change parent, use the specialized updateUnitParent
public slots:
    void createUnit(const Unit & unit);
    void insertChild(int parentId);
    void updateUnit(const Unit &unit);                          // NOT FOR UPDATING THE PARENT
    void updateUnitPosition(int uniqueId, QPointF position);    // Specialized version
    void deleteUnit(int uniqueId);                              // And its descendants
    void deleteUniqueId(int uniqueId);                          // LEAF ONLY

    void selectUnit(int uniqueId);


private:
    Unit * createUnitId(int uniqueId);
    void connectHierarchy(int parentId, int childId);
    void disconnectHierarchy(int parentId, int childId);

    int getChildUniqueId(int parentId);
    QString getAutogeneratedChildDisplayedText(int parentId);

    void updatePositionFromChildren(int uniqueId);


public:
    Unit getTheoreticalChild(int parentId);
    void getTheoreticalMovements(int uniqueId,
                                 QPointF newPosition,
                                 QMap<int, QPointF> &oldPositions,
                                 QMap<int, QPointF> &newPositions);
    QPointF getPositionFromChildren(int uniqueId);              //
    QPointF getFuturePositionFromChildren(int uniqueId, QMap<int, QPointF> &newPositions);

private:
    void computeAllPositionsFromLeaves();


    ///////////////////////////////////////////////////////////////////////
    /// VISIBILITY                                                      ///
    ///////////////////////////////////////////////////////////////////////
public:
    void setAdminView();
    void setPlayerView(const QString & player);

    void computeVisibility();
    void computePlayerVisibility(const QString & playerName);

    Unit getPlayerVisionOfUnit(const QString & player, int uniqueId);

private:
    QStringList friendlies() const;
    QStringList hostiles() const;
    QStringList neutrals() const;




    ///////////////////////////////////////////////////////////////////////
    /// DRAWING                                                         ///
    ///////////////////////////////////////////////////////////////////////
public:
    void createDrawing(const Drawing & drawing);
    void deleteDrawing(int uniqueId);
    void updateDrawing(Drawing drawing);
    void userSpawnDrawing(const Drawing & drawing);

public:
    int getDrawingUniqueId();

//    void createPolyline(QPointF pos);
//    void createPolyline(const Drawing & drawing);
//    void createRectangle(const Drawing & drawing);
//    void createEllipse(const Drawing & drawing);
    //    void createIntention(const Drawing & drawing);


    ///////////////////////////////////////////////////////////////////////
    /// MANIPULATE ORDERS (STRONGLY LINKED TO UNITS)                    ///
    ///////////////////////////////////////////////////////////////////////


// Create and update orders
public slots:
//    void cleanOrders();
    void updateOrder(Order order);
    void updateOrderFromUser(Order order);

private:
    void createDefaultOrders(int uniqueId);

// Apply order
public slots:
    void resetOrderData();
    void preprocessOrders();
    void applyOrders();

private:
    Order applySameOrder(Order originalOrder, int newOriginId);


private:
    void recomputeWhoOwnsWhat();

    ///////////////////////////////////////////////////////////////////////
    /// ADVANCE STATE                                                   ///
    ///////////////////////////////////////////////////////////////////////
public slots:
    void advanceState();
        void emitCurrentTime();

        void changeTime(QDateTime datetime);



    ///////////////////////////////////////////////////////////////////////
    /// MISC                                                            ///
    ///////////////////////////////////////////////////////////////////////
public:
    operator QString() const;





    QMap<QString, QSet<int> > getPlayersOwnedUnits() const;
    QSet<int> getPlayerOwnedUnits(const QString &playerName) const;

    QDateTime getCurrentTime() const;

    void setCurrentTime(const QDateTime &currentTime);

public slots:
    void debug();


    ///////////////////////////////////////////////////////////////////////
    /// SIGNALS                                                         ///
    ///////////////////////////////////////////////////////////////////////
    /// Unit added :    create the unique ID with the relevant info but
    ///                 don't touch its parents/children
    /// Unit updated :  update the info AND the parents/children
    /// Unique ID deleted : remove the ID but don't touch its parents/
    ///                     children

signals:
    //    void gameLoaded();
    void uniqueIdAdded(int uniqueId);
    void uniqueIdDeleted(int uniqueId);
    void unitContentUpdated(Unit unit);                         // Full update (except parent)
    void unitPositionUpdated(int uniqueId, QPointF position);   // Specialized version of the full update
    void unitParentRemoved(int uniqueId);                       // Update for parent
    void unitParentChanged(int parentId, int uniqueId);         // Update for parent

    void dataCleared();
    void mapCleared();
    void playersCleared();
    void unitsCleared();
    void ordersCleared();
    void drawingsCleared();

    void unitSelected(int uniqueId);
    void debugSignal();

    void mapLoaded(QDir dir);

    void playersLoaded(QMap<QString, Player*> players);


    void clearUnitVisibility();
    void setUnitVisibility(int uniqueId, const QString & app6, const QString & displayedText);
    void setUnitVisibilityLife(int uniqueId, const QString & app6, const QString & displayedText, int life);
    void setAdminUnitVisibility();

    void drawingCreated(Drawing drawing);
    void drawingDeleted(int uniqueId);
    void drawingUpdated(Drawing drawing);

    void ordersHidden();
    void ordersShowed(const QString & playerName);
    void orderAdded(Order order);
    void orderUpdated(Order order);
    void orderDeleted(Order order);

    void showOrders(int uniqueId);
    void hideAllOrders();

    void timeUpdated(const QString & time);

    ///////////////////////////////////////////////////////////////////////
    /// ATTRIBUTES                                                      ///
    ///////////////////////////////////////////////////////////////////////
private:
    mutable QMutex mMutex;

    QMap<int, Unit*> mUnits;
    QMap<int, QMap<Order::ORDER_TYPE, Order* >> mOrders; // Each unit has a set of orders
    QMap<int, Drawing*> mDrawings;

    QSet<int> mRootUnits;

    QMap<QString, Player*> mTruePlayers;
    QMap<QString, QSet<int>> mPlayerOwnedUnits; // Should be in truePlayers
    QSet<int> mUnownedUnits;

    QString mMapFolder;

    int mHighestUnitIndex;
    int mHighestOrderIndex;
    int mHighestDrawingIndex;


    // The main window knows what it must display (admin or player view)
    MainWindow * mMainWindow;

    // Time
    QDateTime mStartTime;
    qint64 mTimestep;           // Time (in seconds) between two turns
    QString mDateTimeFormat;
    int mCurrentTurn;           // Not actually saved in json
    QDateTime mCurrentTime;

    // Attrition related
    QMap<int, int> mOrderNbTimesUnderAttack;
    QSet<int> mOrderSuppressed;
    QMap<int, bool> mOrderSafeMove;

};



#endif // DATA_H
