#include "data.h"
//#include "treemodel.h"
//#include "treeitem.h"
#include "treedelegate.h"
#include "mystandarditemmodel.h"
#include "mainwindow.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QJsonDocument>
#include <QString>
#include <QStandardItemModel>
#include <QDateTime>
#include <QTimer>
#include <QVector2D>



///////////////////////////////////////////////////////////////////////
/// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
///////////////////////////////////////////////////////////////////////
Data::Data(QObject *parent):QObject(parent)
{
    mHighestUnitIndex = 0;
    //    mHighestDrawingIndex = 0;
    QDateTime cd = QDateTime::currentDateTime();
    qsrand(cd.toTime_t());
    mHighestDrawingIndex = qrand() % 1000000; // TODO IMPROVE
    qDebug() << "RAND for drawing index " << mHighestDrawingIndex;

    mMainWindow = dynamic_cast<MainWindow*>(parent);
    if(!mMainWindow){
        qWarning() << "Data does not know the session it works for";
        Q_ASSERT(false);
    }

    QTimer * timer = new QTimer();
    connect(timer, &QTimer::timeout, [this](){computeVisibility();});
    timer->start(5000);
}

Data::~Data()
{
    clear();

}

void Data::clear()
{
    mHighestUnitIndex = 0;
    mHighestDrawingIndex = 0;

    clearDrawings();
    clearOrders();
    clearUnitsRoots();
    clearPlayers();
    clearMap();
}

void Data::clearMap()
{
    mMapFolder.clear();
}

void Data::clearPlayers()
{
    for(Player * player: mTruePlayers){
        if(player){
            delete player;
        }
    }
    mTruePlayers.clear();
    emit playersCleared();
}

void Data::clearUnitsRoots()
{
    for(Unit * unit : mUnits){
        if(unit){
            delete unit;
        }
    }
    mUnits.clear();
    mRootUnits.clear();
    emit unitsCleared();
}

void Data::clearOrders()
{
    for(auto map : mOrders){
        for (Order * order : map){
            delete order;
            order = nullptr;
        }
    }
    mOrders.clear();
    emit ordersCleared();
}

void Data::clearDrawings()
{
    for(Drawing * drawing : mDrawings){
        if(drawing){
            delete drawing;
        }
    }
    mDrawings.clear();
    emit drawingsCleared();
}

//void Data::clear()
//{
//    mHighestUnitIndex = 0;
//    mHighestDrawingIndex = 0;

//    emit dataCleared();
//}


///////////////////////////////////////////////////////////////////////
/// GETTERS, SETTERS                                                ///
///////////////////////////////////////////////////////////////////////
QMap<int, Unit*> Data::units() const
{
    return mUnits;
}

QMap<int, Drawing *> Data::drawings() const
{
    return mDrawings;
}

QMap<int, QMap<Order::ORDER_TYPE, Order *> > Data::getOrders() const
{
    return mOrders;
}

QMap<QString, Player *> Data::getTruePlayers() const
{
    return mTruePlayers;
}

QStringList Data::getPlayerNames() const{
    QStringList answer;
    for(const Player * player : mTruePlayers){
        answer.push_back(player->name());
    }
    return answer;
}

///////////////////////////////////////////////////////////////////////
/// EXPLORING UNIT                                                  ///
///////////////////////////////////////////////////////////////////////
QVector<int> Data::children(int uniqueId) const
{
    QVector<int> answer;
    if(!mUnits.contains(uniqueId)){
        return answer;
    }
    return mUnits[uniqueId]->children();
}

QVector<int> Data::siblings(int uniqueId) const
{
    // Siblings with Units
    // It kind of fail on toplevel units (no parent)
    QVector<int> answer;
    if(!mUnits.contains(uniqueId)){
        return answer;
    }

    int parentId = mUnits[uniqueId]->parentId();
    if(!mUnits.contains(parentId)){
        return answer;
    }

    answer = mUnits[parentId]->children();
    answer.removeAll(uniqueId);
    return answer;
}


QVector<int> Data::getDescendants(int uniqueId) const
{
    QVector<int> answer;
    QSet<int> descendants;  // Not really useful, it's a tree, not a DAG

    answer.push_back(uniqueId);
    descendants.insert(uniqueId);

    int index=0;
    while(index < answer.size()){
        Unit * currentUnit = mUnits[answer[index]];
        for(int childId : currentUnit->children()){
            if(!descendants.contains(childId)){
                descendants.insert(childId);
                answer.push_back(childId);
            }
        }
        index++;
    }

    return answer;
}

QVector<Unit> Data::getFullDescendants(int uniqueId) const
{
    QVector<int> descendants = getDescendants(uniqueId);
    QVector<Unit> answer;

    for(int id : descendants){
        answer.push_back(Unit(*(mUnits[id])));
    }

    // Remove the children
    for(int i=0; i<answer.size(); ++i){
        answer[i].clearChildren();
    }

    return answer;
}

// TODO : somewhat fail if there is a weird kid with a position but not its parent
QVector<int> Data::getLeavesOnMap(int uniqueId) const
{
    QVector<int> allDescendants = getDescendants(uniqueId);

    QVector<int> answer;
    for(int id : allDescendants){
        if(isALeafOnMap(id)){
            answer.push_back(id);
        }
    }
    return answer;
}

bool Data::isALeafOnMap(int uniqueId) const
{
    Unit * unit = mUnits[uniqueId];
    if(!unit->isPosLegal()){
        return false;
    }
    for(int childId : unit->children()){
        Unit * childUnit = mUnits[childId];
        if(childUnit->isPosLegal()){
            return false;
        }
    }
    return true;
}

QMap<int, bool> Data::unitsAtLevel(int level) const
{
    QMap<int, bool> answer;
    if(level < 0){
        for(auto it = mUnits.constBegin(); it != mUnits.constEnd(); ++it){
            answer[it.key()] = it.value()->isPosLegal();
        }
        return answer;
    }

    for(auto it = mUnits.constBegin(); it != mUnits.constEnd(); ++it){
        answer[it.key()] = false;
    }

    QVector<QSet<int>> dataStructure;

    dataStructure.push_back(QSet<int>());
    dataStructure[0] = mRootUnits;

    qDebug() << "ROOTS ARE " << mRootUnits;


    for(int i=1; i<=level; ++i){
        dataStructure.push_back(QSet<int>());
        for(int id : dataStructure[i-1]){

            if(isALeafOnMap(id)){
                dataStructure[i].insert(id);
                continue;
            }
            Unit * unit = mUnits[id];
            for(int childId : unit->children()){
                if(mUnits[childId]->isPosLegal()){
                    dataStructure[i].insert(childId);
                }
            }
        }
    }

    for(int visibleId : dataStructure[level]){
        answer[visibleId] = true;
    }

    return answer;
}

QMap<int, bool> Data::unitsAtLevel(int level, const QString &player) const
{

}

QPointF Data::getUnitPosition(int uniqueId) const
{
    if(!mUnits.contains(uniqueId)){
        return Unit::illegalPosition();
    }
    return mUnits[uniqueId]->pos();
}



///////////////////////////////////////////////////////////////////////
/// NETWORK/LOAD/SAVE                                               ///
///////////////////////////////////////////////////////////////////////
bool Data::loadGame(QString filename, Data::SaveFormat saveFormat)
{
    //    QFile loadFile(saveFormat == Json
    //                   ? (filename + QStringLiteral(".json"))
    //                   : (filename + QStringLiteral(".dat")));
    
    // Convert file as JSON
    QFile loadFile(filename);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file : " << (filename);
        return false;
    }
    QByteArray saveData = loadFile.readAll();
    QJsonDocument jsonDoc(saveFormat == Json
                          ? QJsonDocument::fromJson(saveData)
                          : QJsonDocument::fromBinaryData(saveData));
    
    const QJsonObject & jsonObject = jsonDoc.object();

    // Load JSON
    if(loadJson(jsonObject)){
        //        qDebug() << "Loaded save using "
        //                 << (saveFormat != Json ? "binary " : "") << "JSON...\n";
        return true;
    }

    return false;
}

bool Data::saveGame(QString filename, Data::SaveFormat saveFormat)
{
    //    QFile saveFile(saveFormat == Json
    //                   ? (filename + QStringLiteral(".json"))
    //                   : (filename + QStringLiteral(".dat")));

    qDebug() << "Save file " << filename;
    QFile saveFile(filename);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file");
        return false;
    }

    QJsonObject gameObject;
    writeJson(gameObject);

    QJsonDocument saveDoc(gameObject);
    saveFile.write(saveFormat == Json
                   ? saveDoc.toJson()
                   : saveDoc.toBinaryData());

    qDebug() << "File saved : " << (filename);
    return true;
}

bool Data::loadJson(const QJsonObject &json)
{
    qDebug() << "Load JSON";
    qDebug() << json;

    mHighestUnitIndex = 0;
    mHighestDrawingIndex = 0;

    loadMap(json);
    loadTime(json);
    loadPlayers(json);
    loadUnits(json);
    loadOrders(json);
    loadDrawings(json);

    //    debug();

    return true;    // TODO unsued bool
}

void Data::loadMap(const QJsonObject &json)
{

    clearMap();

    // MAP
    if (json.contains("map") && json["map"].isString()){
        mMapFolder = json["map"].toString();
        QDir mapFolder = QDir::current();
        mapFolder.cd(mMapFolder);
        qDebug() << "Map folder is : " << (mapFolder.dirName());
        emit mapLoaded(mapFolder);
    }
}

void Data::loadTime(const QJsonObject &json)
{
    if (json.contains("time") && json["time"].isObject()){
        QJsonObject timeObject = json["time"].toObject();
        if (timeObject.contains("format") && timeObject["format"].isString()){
            mDateTimeFormat = timeObject["format"].toString();
        }
        else{
            mDateTimeFormat = QString("hh:mm:ss");
        }

        if (timeObject.contains("start") && timeObject["start"].isString()){
            mStartTime = QDateTime::fromString(timeObject["start"].toString(), mDateTimeFormat);
        }
        else{
            mStartTime = QDateTime::fromString("00:00:00", "hh:mm:ss");
        }

        if (timeObject.contains("current") && timeObject["current"].isString()){
            mCurrentTime = QDateTime::fromString(timeObject["current"].toString(), mDateTimeFormat);
        }
        else{
            mCurrentTime = QDateTime::fromString("00:00:00", "hh:mm:ss");
        }

        if (timeObject.contains("timestep") && timeObject["timestep"].isDouble()){
            mTimestep = static_cast<int>(timeObject["timestep"].toDouble());
        }
        else{
            mTimestep = 3600;
        }
    }
    else{
        mDateTimeFormat = QString("hh:mm:ss");
        mStartTime = QDateTime::fromString("00:00:00", "hh:mm:ss");
        mCurrentTime = mStartTime;
        mTimestep = 3600;
    }

    if (json.contains("turnHistory") && json["turnHistory"].isArray()){
        QJsonArray turnArray = json["turnHistory"].toArray();

        mCurrentTurn = turnArray.size();
    }
    else{
        mCurrentTurn = 0;
    }

    emitCurrentTime();

}

void Data::loadPlayers(const QJsonObject &json)
{
    // Clear
    clearPlayers();

    // PLAYERS
    if (json.contains("players") && json["players"].isArray()) {
        QJsonArray playersArray = json["players"].toArray();
        for(QJsonValue val : playersArray){
            QString playerName = val.toString();
            mTruePlayers[playerName] = new Player(playerName);
            //            mPlayers.append(val.toString());
        }
    }

    if (json.contains("playerSide") && json["playerSide"].isObject()) {
        QJsonObject sides = json["playerSide"].toObject();
        for(const QString & playerName : getPlayerNames()){
            if(sides.contains(playerName) && sides[playerName].isString()){
                QString playerAffiliation = sides[playerName].toString();
                mTruePlayers[playerName]->setAffiliation(playerAffiliation);
            }
        }
    }
    emit playersLoaded(mTruePlayers);   // TODO merge the two player thingies
}



void Data::loadUnits(const QJsonObject &json)
{
    clearOrders();
    clearUnitsRoots();

    // UNITS
    // Create all units placeholders
    if (json.contains("units") && json["units"].isArray()) {
        QJsonArray unitsArray = json["units"].toArray();
        for (int unitIndex = 0; unitIndex < unitsArray.size(); ++unitIndex) {
            QJsonObject unitObject = unitsArray[unitIndex].toObject();
            Unit unit;
            unit.readJson(unitObject);
            createUnitId(unit.uniqueId());
            *(mUnits[unit.uniqueId()]) = unit;
        }
    }

    // Fill the units with the actual data
    for(Unit * unit : mUnits){
        emit unitContentUpdated(*unit);
        emit unitPositionUpdated(unit->uniqueId(), unit->pos());
        for(int childId: unit->children()){
            emit unitParentChanged(unit->uniqueId(), childId);
        }
    }

    for(const Unit * unit : mUnits){
        if(unit->parentId()<0){
            mRootUnits.insert(unit->uniqueId());
        }
    }
    qDebug() << "Nb of root : " << mRootUnits.size();

    // Ownership
    resetOrderData();
    recomputeWhoOwnsWhat();
}

void Data::loadUpdateToUnits(const QJsonObject &json)
{
    // UNITS
    // Check all units placeholders
    QSet<int> newUnits;
    if (json.contains("units") && json["units"].isArray()) {
        QJsonArray unitsArray = json["units"].toArray();
        for (int unitIndex = 0; unitIndex < unitsArray.size(); ++unitIndex) {
            QJsonObject unitObject = unitsArray[unitIndex].toObject();
            Unit unit;
            unit.readJson(unitObject);
            int uniqueId = unit.uniqueId();
            if(!mUnits.contains(uniqueId)){
                createUnitId(uniqueId);
                newUnits.insert(uniqueId);
            }
            *(mUnits[uniqueId]) = unit;
        }
    }

    // Broadcast the changes
    for(Unit * unit : mUnits){
        emit unitContentUpdated(*unit);
        emit unitPositionUpdated(unit->uniqueId(), unit->pos());
        for(int childId: unit->children()){
            emit unitParentChanged(unit->uniqueId(), childId);
        }
    }

    mRootUnits.clear();
    for(const Unit * unit : mUnits){
        if(unit->parentId()<0){
            mRootUnits.insert(unit->uniqueId());
        }
    }
    qDebug() << "Nb of root : " << mRootUnits.size();

    // Ownership
    recomputeWhoOwnsWhat();
}

void Data::loadOrders(const QJsonObject &json)
{
    // Clear the old orders
    clearOrders();

    // Load the new ones
    loadAdditionalOrders(json);
}

void Data::loadAdditionalOrders(const QJsonObject &json)
{
    // ORDERS
    if (json.contains("orders") && json["orders"].isArray()) {
        QJsonArray ordersArray = json["orders"].toArray();
        qDebug() << "NB orders " << ordersArray.size();
        for(QJsonValue val : ordersArray){
            QJsonObject order = val.toObject();
            Order o(order);
            updateOrder(o);
        }
    }
}

void Data::loadDrawings(const QJsonObject &json)
{
    // Clear
    for(Drawing * drawing : mDrawings){
        if(drawing){
            delete drawing;
        }
    }
    mDrawings.clear();
    emit drawingsCleared();

    // DRAWINGS
    if (json.contains("drawings") && json["drawings"].isArray()) {
        QJsonArray drawingsArray = json["drawings"].toArray();
        qDebug() << "NB drawing " << drawingsArray.size();
        for(QJsonValue val : drawingsArray){
            QJsonObject drawing = val.toObject();
            Drawing d;
            d.read(drawing);
            createDrawing(d);
        }
    }
}


void Data::writeJson(QJsonObject &json, bool skipOrders) const
{
    writeMap(json);
    writeTime(json);
    writePlayers(json);
    writePlayerSides(json); // TODO
    writeUnits(json);
    if(!skipOrders)
        writeOrders(json);
    writeDrawings(json);
}

void Data::writeMap(QJsonObject &json) const
{
    QMutexLocker lock(&mMutex);
    json["map"] = mMapFolder;
}

void Data::writeTime(QJsonObject &json) const
{
    QJsonObject object;
    object["format"] = mDateTimeFormat;
    object["start"] = mStartTime.toString(mDateTimeFormat);
    object["current"] = mCurrentTime.toString(mDateTimeFormat);
    object["timestep"] = mTimestep;

    json["time"] = object;
}



void Data::writePlayers(QJsonObject &json) const
{
    QMutexLocker lock(&mMutex);
    QStringList players = getPlayerNames();
    if(!players.empty()){
        QJsonArray playerArray;
        for(const QString & player : players){
            playerArray.append(player);
        }
        json["players"] = playerArray;
    }
}

void Data::writePlayerSides(QJsonObject &json) const
{
    QMutexLocker lock(&mMutex);
    if(!mTruePlayers.empty()){
        QJsonObject playerSide;
        for(Player * player : mTruePlayers){
            playerSide[player->name()] = player->affiliationString();
        }
        json["playerSide"] = playerSide;
    }
}

void Data::writeUnits(QJsonObject &json) const
{
    QMutexLocker lock(&mMutex);
    QJsonArray unitsArray;
    for(const Unit * unit : mUnits){
        QJsonObject unitObject;
        unit->writeJson(unitObject);
        unitsArray.append(unitObject);
    }
    json["units"] = unitsArray;
}

void Data::writeOrders(QJsonObject & json) const
{
    QJsonArray orders;
    for(auto map : mOrders){
        for (Order * order : map){
            if(order->isEnabled()){
                QJsonObject object;
                order->writeJson(object);
                orders.push_back(object);
            }
        }
    }
    json["orders"] = orders;
}

void Data::writeDrawings(QJsonObject &json) const
{
    QMutexLocker lock(&mMutex);
    QJsonArray drawings;
    for(Drawing * drawing : mDrawings){
        QJsonObject object;
        drawing->write(object);
        drawings.push_back(object);
    }

    json["drawings"] = drawings;
}

QString Data::serializeData(bool skipOrders)
{
    QJsonObject gameObject;
    writeJson(gameObject, skipOrders);
    QJsonDocument doc(gameObject);
    return doc.toJson(QJsonDocument::Compact);
}

QString Data::serializeUnits()
{
    QJsonObject gameObject;
    writeUnits(gameObject);
    QJsonDocument doc(gameObject);
    return doc.toJson(QJsonDocument::Compact);
}


///////////////////////////////////////////////////////////////////////
/// MANIPULATE UNIT                                                 ///
///////////////////////////////////////////////////////////////////////



// SLOT
// Assumes all the data in unit already exists (such as parent/children)
void Data::createUnit(const Unit & unit)
{
    qDebug().noquote() << "Creation of unit " << unit;
    int uniqueId = unit.uniqueId();
    Unit * createdUnit = createUnitId(uniqueId);
    if(createdUnit){
        updateUnit(unit);
    }
    if(mUnits.contains(unit.parentId())){
        connectHierarchy(unit.parentId(), uniqueId);
    }
}

// SLOT
// Create the default child of a unit
void Data::insertChild(int parentId)
{
    Unit childUnit = getTheoreticalChild(parentId);
    createUnit(childUnit);
}


// SLOT
// Updates all the info (except the parent) and broadcasts it
// Does not check if the parent has changed, because you there is not enough information to know
// what rank the unit has among its siblings
void Data::updateUnit(const Unit & unit)
{
    QMutexLocker lock(&mMutex);
    int uniqueId = unit.uniqueId();
    qDebug().noquote() << "Unit " << uniqueId << " needs an update : " << unit.operator QString();

    Unit * oldUnit = mUnits[uniqueId];
    // Skip if identical
    if(unit == *oldUnit){
        return;
    }

    // Sanity check for the usage of this method
    if(oldUnit->parentId() >= 0){
        Q_ASSERT(oldUnit->parentId() == unit.parentId());
    }


    // Store temporary data
    //    int oldParent = oldUnit->parentId();
    //    int newParent = unit.parentId();

    QVector<int> oldChildren = oldUnit->children();
    QVector<int> newChildren = unit.children();

    QString oldOwner = oldUnit->owner();

    // Inside content stuff
    *(oldUnit) = unit;

    // Hierarchy children stuff
    if(oldChildren != newChildren){
        for(int oldChildId : oldChildren){
            disconnectHierarchy(uniqueId, oldChildId);
        }
        for(int newChildId : newChildren){
            connectHierarchy(uniqueId, newChildId);
        }
    }

    emit unitContentUpdated(unit);
    emit unitPositionUpdated(unit.uniqueId(), unit.pos());

    // If the unit has a new owner, all its descendants belong to the same owner
    if(oldOwner != unit.owner()){
        //        qDebug() << "CASCADE OWNER";
        QVector<int> descendants = getDescendants(uniqueId);
        for(int id : descendants){
            //            qDebug() << "Des " << id;
            Unit * descendant = mUnits[id];
            descendant->setOwner(unit.owner());
            emit unitContentUpdated(*descendant);
            emit unitPositionUpdated(id, descendant->pos());
        }
    }
    else{
        //        qDebug() << "NO CASCADE";
    }

    // Recompute visibility

}

void Data::updateUnitPosition(int uniqueId, QPointF position)
{
    if(!mUnits.contains(uniqueId)){
        return;
    }
    Unit * unit = mUnits[uniqueId];
    unit->setPos(position);
    emit unitPositionUpdated(uniqueId, position);
}


void Data::deleteUnit(int uniqueId)
{
    Unit * unit = mUnits[uniqueId];
    for(int childId : unit->children()){
        deleteUnit(childId);
    }
    deleteUniqueId(uniqueId);
}

void Data::selectUnit(int uniqueId)
{
    if(!mUnits.contains(uniqueId)){
        return;
    }
    emit unitSelected(uniqueId);
}








// PRIVATE
Unit * Data::createUnitId(int uniqueId)
{
    QMutexLocker lock(&mMutex);
    if(mUnits.contains(uniqueId)){
        qWarning() << "Data was asked to create the already existing unit " << uniqueId;
        return nullptr;
    }

    // Create
    Unit * answer = new Unit(uniqueId);
    mUnits[uniqueId] =  answer;
    createDefaultOrders(uniqueId);

    // Broadcast the existence
    emit uniqueIdAdded(uniqueId);
    return answer;
}



// PRIVATE
// Leaf only
void Data::deleteUniqueId(int uniqueId)
{
    QMutexLocker lock(&mMutex);
    qDebug() << "Delete Unit : " << uniqueId;

    // Check
    if(!mUnits.contains(uniqueId)){
        qWarning() << "Data was asked to delete the non-existing unit " << uniqueId;
        return;
    }

    // Check that there are no children
    Unit * unit = mUnits[uniqueId];
    if(unit->children().size() > 0){
        qWarning() << "Delete ID with children: " << uniqueId;
        Q_ASSERT(false);
    }

    // Disconnect from parent
    if(mUnits.contains(unit->parentId())){
        disconnectHierarchy(unit->parentId(), uniqueId);
    }

    // Delete
    delete unit;
    mUnits.remove(uniqueId);

    // Broadcast
    emit uniqueIdDeleted(uniqueId);

}

void Data::connectHierarchy(int parentId, int childId)
{
    Unit * childUnit = mUnits[childId];
    Unit * parentUnit = mUnits[parentId];

    if(childUnit->parentId() != parentId){
        disconnectHierarchy(childUnit->parentId(), childId);
        childUnit->setParent(parentId);
    }

    if(!parentUnit->hasChild(childId)){
        parentUnit->addChild(childId);
        emit unitParentChanged(parentId, childId);
    }
}

void Data::disconnectHierarchy(int parentId, int childId)
{
    if(parentId <0){
        return;
    }

    Unit * childUnit = mUnits[childId];
    Unit * parentUnit = mUnits[parentId];

    parentUnit->setChildren(siblings(childId));
    childUnit->setParent(-1);

    emit unitParentRemoved(childId);
}



int Data::getChildUniqueId(int parentId)
{
    qDebug() << "getUniqueId from parent " << parentId;

    int index = mHighestUnitIndex;
    while(mUnits.contains(index)){
        index++;
    }
    mHighestUnitIndex = index;
    return index;
}


QString Data::getAutogeneratedChildDisplayedText(int parentId)
{
    Unit * parentUnit = mUnits[parentId];
    int index = 1;
    while(true){
        //        QString answer = parentUnit->displayedText() + QChar('/') + QString::number(index);
        QString answer = parentUnit->displayedText() + QString::number(index);

        bool nameAlreadyTaken = false;
        for(int childId : parentUnit->children()){
            if(mUnits[childId]->displayedText() == answer){
                nameAlreadyTaken = true;
                continue;
            }
        }
        if(!nameAlreadyTaken){
            return answer;
        }
        index++;
        if(index > 1000){
            qWarning() << "You look like you are looping.";
            if(index > 100000){
                Q_ASSERT(false);
            }
        }
    }
}

void Data::updatePositionFromChildren(int uniqueId)
{
    Unit * unit = mUnits[uniqueId];
    int nbValidChildren = 0;
    QPointF pos;
    for(int childId : unit->children()){
        Unit * childUnit = mUnits[childId];
        if(childUnit->isPosLegal()){
            pos += childUnit->pos();
            nbValidChildren++;
        }
    }

    if(nbValidChildren==0){
        Q_ASSERT(false);
    }
    qDebug() << "Unit " << uniqueId << " has " << nbValidChildren << " valid children. Ends up in " << (pos/nbValidChildren);
    unit->setPos(pos/nbValidChildren);
    emit unitContentUpdated(*unit);
}



// Create the child of unit :
// - same function
// - lower size
// - autogenerated displayed text
Unit Data::getTheoreticalChild(int parentId)
{
    int childId = getChildUniqueId(parentId);
    Unit answer = mUnits[parentId]->createChild(childId);
    answer.setDisplayedText(getAutogeneratedChildDisplayedText(parentId));
    return answer;
}

// If a unit moves
// Then all its children get the same move
// All its parents get impacted by the move
void Data::getTheoreticalMovements(int uniqueId, QPointF newPosition, QMap<int, QPointF> &oldPositions, QMap<int, QPointF> &newPositions)
{
    if(!mUnits.contains(uniqueId)){
        return;
    }

    // The unit
    Unit * unit = mUnits[uniqueId];
    QPointF oldPosition = unit->pos();
    oldPositions[unit->uniqueId()] = oldPosition;
    newPositions[uniqueId] = newPosition;
    QPointF delta = newPosition - oldPosition;
    //    qDebug() << "Data has theoretical order to move " << uniqueId << " to " << newPosition << " with delta " << delta;

    // The children
    // This can erase the unit, which is not a problem
    QVector<int> descendants = getDescendants(uniqueId);
    int index = descendants.size();
    while(index --> 0){ // "goes down to 0" operator
        Unit * childUnit = mUnits[descendants[index]];
        if(childUnit->isPosLegal()){
            int childId = childUnit->uniqueId();
            oldPositions[childId] = childUnit->pos();
            if(isALeafOnMap(childId)){
                //                qDebug() << "Directly moving leaf " << childUnit->uniqueId();
                newPositions[childId] = childUnit->pos() + delta;
            }
            else{
                newPositions[childId] = getFuturePositionFromChildren(childId, newPositions);
            }
        }
        else{
            // Do nothing
        }
    }


    // The parents
    int parentId = unit->parentId();
    while(mUnits.contains(parentId)){
        Unit * parentUnit = mUnits[parentId];
        oldPositions[parentId] = parentUnit->pos();
        newPositions[parentId] = getFuturePositionFromChildren(parentId, newPositions);
        parentId = mUnits[parentId]->parentId();
    }


    //    qDebug() << "Old positions " << oldPositions;
    //    qDebug() << "New positions " << newPositions;
}

QPointF Data::getPositionFromChildren(int uniqueId)
{
    Unit * unit = mUnits[uniqueId];

    // Grab all the children and sum their positions
    int nbValidChildren = 0;
    QPointF answer;
    for(int childId : unit->children()){
        Unit * childUnit = mUnits[childId];
        if(childUnit->isPosLegal()){
            answer += childUnit->pos();
            nbValidChildren++;
        }
    }

    if(nbValidChildren==0){
        return unit->pos();
    }
    //    qDebug() << "Unit " << uniqueId << " has " << nbValidChildren << " valid children. Ends up in " << (answer/nbValidChildren);

    return answer/nbValidChildren;
}

QPointF Data::getFuturePositionFromChildren(int uniqueId, QMap<int, QPointF> &newPositions)
{
    Unit * unit = mUnits[uniqueId];

    // Grab all the children and sum their positions
    int nbValidChildren = 0;
    QPointF answer;
    for(int childId : unit->children()){
        // If it's in the new positions
        if(newPositions.contains(childId)){
            answer += newPositions[childId];
            nbValidChildren++;
            continue;
        }

        // If it's in the old positions
        Unit * childUnit = mUnits[childId];
        if(childUnit->isPosLegal()){
            answer += childUnit->pos();
            nbValidChildren++;
        }
    }

    if(nbValidChildren==0){
        Q_ASSERT(false);
    }
    //    qDebug() << "Unit " << uniqueId << " has " << nbValidChildren << " valid children. Ends up in " << (answer/nbValidChildren);

    return answer/nbValidChildren;
}

void Data::computeAllPositionsFromLeaves()
{
    for(int rootId : mRootUnits){
        QVector<int> descendants = getDescendants(rootId);
        int i = descendants.size();
        while(i --> 0){ // goes down to 0
            Unit * unit = mUnits[descendants[i]];
            if(unit->hasChildren()){
                unit->setPos(getPositionFromChildren(descendants[i]));
            }
        }
    }

    for(auto it = mUnits.cbegin(); it != mUnits.cend(); ++it){
        int uniqueId = it.key();
        emit unitPositionUpdated(uniqueId, mUnits[uniqueId]->pos());
    }
}

void Data::createDefaultOrders(int uniqueId)
{
    mOrders[uniqueId] = QMap<Order::ORDER_TYPE, Order*>();
    for(int i=0; i<Order::NB_ORDERS; ++i){
        Order::ORDER_TYPE orderType = static_cast<Order::ORDER_TYPE>(i);
        mOrders[uniqueId][orderType] = new Order(uniqueId, orderType);
    }
}


Order Data::applySameOrder(Order originalOrder, int newOriginId)
{
    Order answer = originalOrder;
    answer.setOriginUniqueId(newOriginId);

    switch (originalOrder.orderType()) {
    case Order::MOVEMENT:{
        if(originalOrder.isEnabled()){
            QPointF delta = originalOrder.destinationPosition() - getUnitPosition(originalOrder.originUniqueId());
            answer.setDestinationPosition(getUnitPosition(newOriginId) + delta);
        }
        else{
            answer.setDestinationPosition(Unit::illegalPosition());
        }
        break;
    }
    case Order::ATTACK:{
        // Nothing to do
        break;
    }
    default:
        break;

    }

    return answer;
}

void Data::recomputeWhoOwnsWhat()
{   
    // Create all the data structures to fill
    mUnownedUnits.clear();
    QMap<QString, QSet<int>> playerOwnedUnits;
    for(Player * player : mTruePlayers){
        playerOwnedUnits[player->name()] = QSet<int>();
    }

    for(const Unit * unit : mUnits){
        const QString & player = unit->owner();
        if(playerOwnedUnits.contains(player)){
            playerOwnedUnits[player].insert(unit->uniqueId());
        }
        else{
            mUnownedUnits.insert(unit->uniqueId());
        }
    }
    for(Player * player : mTruePlayers){
        player->setOwnedUnits(playerOwnedUnits[player->name()]);
    }

}

void Data::advanceState()
{
    preprocessOrders();
    applyOrders();
    clearOrders();

    mCurrentTime = mCurrentTime.addSecs(mTimestep);
    mCurrentTurn++;
    qDebug() << "Current turn " << mCurrentTurn;
    emitCurrentTime();
}

void Data::emitCurrentTime()
{
    qDebug() << "Current time " << mCurrentTime;
    emit timeUpdated(mCurrentTime.toString(mDateTimeFormat));
}

void Data::changeTime(QDateTime datetime)
{
    mCurrentTime = datetime;
}

void Data::setAdminView()
{
    qDebug() << "Set admin view ";

    // Unit visibility
    emit clearUnitVisibility();
    for(Unit * unit : mUnits){
        emit setUnitVisibilityLife(unit->uniqueId(), unit->app6(), unit->displayedText(), static_cast<int>(unit->getLife()));
    }

    // Orders
    //    emit ordersHidden();
    emit hideAllOrders();


    //    emit setAdminUnitVisibility();
}

void Data::setPlayerView(const QString &playerName)
{
    qDebug() << "Set player view " << playerName;

    // Unit visibility
    computePlayerVisibility(playerName);

    Player * player = mTruePlayers[playerName];
    player->debug();
    QMap<int, QStringList> visibleInfo = player->visibleUnitInfo();

    //UNITS
    // Hide all the units
    emit clearUnitVisibility();

    // Then all that is seen by the player, with the relevant APP6, text, etc...
    for(auto it = visibleInfo.cbegin(); it != visibleInfo.cend(); ++it){
        emit setUnitVisibilityLife(it.key(), it.value()[0], it.value()[1], static_cast<int>(mUnits[it.key()]->getLife()));
    }

    // ORDERS
    // Hide all orders
    //    emit ordersHidden();
    emit hideAllOrders();


    // Then display all the orders that the player can see
    for(int ownedId : mTruePlayers[playerName]->ownedUnits()){
        emit showOrders(ownedId);
    }
    emit ordersShowed(playerName);
}

void Data::computeVisibility()
{

    for(const QString & playerName : getPlayerNames()){
        computePlayerVisibility(playerName);
    }
}

void Data::computePlayerVisibility(const QString &playerName)
{
    //    QMutexLocker lock(&mMutex);
    Player * player = mTruePlayers[playerName];
    player->clearVisibility();

    // OWNED
    for(Unit * unit : mUnits){
        if(unit->owner() == playerName){
            player->setVisible(unit->uniqueId(), QStringList{unit->app6(), unit->displayedText()});
        }
    }

//    // ALLY
//    QStringList allies;
//    if(friendlies().contains(playerName)){
//        allies = friendlies();
//    }
//    else{
//        allies = hostiles();
//    }
//    for(Unit * unit : mUnits){
//        if(!unit->isPosLegal()){
//            continue;
//        }

//        if(allies.contains(unit->owner())){
//            player->setVisible(unit->uniqueId(), QStringList{unit->app6(), unit->displayedText()});
//        }
//    }

    // REST
    auto alliedUnits = player->visibleUnitInfo();

    // TODO
    for(Unit * unit : mUnits){
        if(!unit->isPosLegal()){
            continue;
        }

        if(alliedUnits.contains(unit->uniqueId())){
            continue;
        }
        // Unprocessed unit
        QPointF pos = unit->pos();
        qreal minDistance = 100000.0;

        bool safeMoveUnit = mOrderSafeMove[unit->uniqueId()];

        for(auto it = alliedUnits.cbegin(); it != alliedUnits.cend(); ++it){
            int uniqueId = it.key();
            Unit * otherUnit = mUnits[uniqueId];
            double realDistance = QVector2D(pos-(otherUnit->pos())).length();
            bool safeMoveOther = mOrderSafeMove[otherUnit->uniqueId()];

            if(safeMoveOther){
                realDistance *= 1.25;
            }
            if(safeMoveUnit){
                realDistance *= 0.8;
            }
            minDistance = qMin(minDistance, realDistance);
        }
        //        qDebug() << "Unit " << unit->displayedText() << " is at distance " << minDistance << " from a unit of " << playerName << " or its allies";
        if(minDistance < 10){
            // Side known, nature connue, volume connu
            player->setVisible(unit->uniqueId(), QStringList{unit->app6(), unit->displayedText()});
            continue;
        }
        if(minDistance < 20){
            // Side known, nature inconnu, volume connu
            player->setVisible(unit->uniqueId(), QStringList{unit->app6ToNoAffiliationNoSize(), QString("")});
            continue;
        }
        if(minDistance < 30){
            // Side unknown, nature inconnu, volume inconnu
            player->setVisible(unit->uniqueId(), QStringList{unit->app6ToUnknown(), QString("")});
            continue;
        }
    }
}

Unit Data::getPlayerVisionOfUnit(const QString &player, int uniqueId)
{
    Player * p = mTruePlayers[player];
    QMap<int, QStringList> unitInfo = p->visibleUnitInfo();
    Unit answer;
    if(unitInfo.contains(uniqueId)){
        QStringList info = unitInfo[uniqueId];
        answer.setApp6(info[0]);
        answer.setDisplayedText(info[1]);
        answer.setLife(mUnits[uniqueId]->getLife());
    }
    else{
        answer.setApp6("SUGPU-------");
    }

    return answer;
}

QStringList Data::friendlies() const
{
    QStringList answer;
    for(Player * player : mTruePlayers){
        if(player->affiliation() == Unit::FRIENDLY){
            answer.push_back(player->name());
        }
    }
    return answer;
}

QStringList Data::hostiles() const
{
    QStringList answer;
    for(Player * player : mTruePlayers){
        if(player->affiliation() == Unit::HOSTILE){
            answer.push_back(player->name());
        }
    }
    return answer;
}

QStringList Data::neutrals() const
{
    QStringList answer;
    for(Player * player : mTruePlayers){
        if(player->affiliation() == Unit::NEUTRAL){
            answer.push_back(player->name());
        }
    }
    return answer;
}



///////////////////////////////////////////////////////////////////////
/// DRAWING                                                         ///
///////////////////////////////////////////////////////////////////////

void Data::createDrawing(const Drawing &d)
{
    Drawing * drawing = new Drawing();
    *drawing = d;
    mDrawings[d.uniqueId()] = drawing;

    emit drawingCreated(d);

    //    switch(drawing.type()){
    //    case Drawing::POLYLINE:
    //        createPolyline(drawing);
    //        break;
    //    case Drawing::RECTANGLE:
    //        createRectangle(drawing);
    //        break;
    //    case Drawing::ELLIPSE:
    //        createEllipse(drawing);
    //        break;
    //    }
}

void Data::deleteDrawing(int uniqueId)
{
    Drawing * drawing = mDrawings[uniqueId];
    delete drawing;
    mDrawings.remove(uniqueId);
    emit drawingDeleted(uniqueId);
}

void Data::updateDrawing(Drawing drawing)
{
    int uniqueId = drawing.uniqueId();
    Drawing * oldDrawing = mDrawings[uniqueId];
    (*oldDrawing) = drawing;
    emit drawingUpdated(drawing);
}


int Data::getDrawingUniqueId()
{
    int index = mHighestDrawingIndex;
    while(mDrawings.contains(index)){
        index++;
    }
    mHighestDrawingIndex = index;
    return index;
}

void Data::userSpawnDrawing(const Drawing &d)
{
    int uniqueId = getDrawingUniqueId();
    Drawing * drawing = new Drawing();
    *drawing = d;
    mDrawings[uniqueId] = drawing;
    drawing->setUniqueId(uniqueId);

    // No emit
}



//void Data::createPolyline(QPointF pos)
//{
//    int uniqueId = getDrawingUniqueId();
//    Drawing * drawing = new Drawing();
//    drawing->setUniqueId(uniqueId);
//    drawing->setType(Drawing::POLYLINE);
//    drawing->setPos(pos);

//    QPainterPath painterPath(pos);
//    drawing->setPainterPath(painterPath);

//    mDrawings[uniqueId] = drawing;

//    emit drawingCreated(Drawing(*(mDrawings[uniqueId])));
//}

//void Data::createPolyline(const Drawing &d)
//{
//    int uniqueId = getDrawingUniqueId();
//    Drawing * drawing = new Drawing();
//    *drawing = d;
//    drawing->setUniqueId(uniqueId);
//    mDrawings[uniqueId] = drawing;

//    emit drawingCreated(Drawing(*(mDrawings[uniqueId])));
//}



//void Data::createIntention(const Drawing &d)
//{
//    int uniqueId = getDrawingUniqueId();
//    Drawing * drawing = new Drawing();
//    *drawing = d;
//    drawing->setUniqueId(uniqueId);
//    mDrawings[uniqueId] = drawing;
//    mDrawings[uniqueId]->setIsIntention(true);

//}





//void Data::cleanOrders()
//{
//    for(auto map : mOrders){
//        for (Order * order : map){
//            order->clear();
//            emit orderUpdated(*order);
//        }
//    }
//}

void Data::updateOrder(Order order)
{
    qDebug() << "Receive an update for order ";
    order.debug();
    int originUniqueId = order.originUniqueId();

    if(!mOrders.contains(originUniqueId)){
        qWarning() << "Updating an order of an inexistant unit with ID : " << originUniqueId;
        mOrders[originUniqueId] = QMap<Order::ORDER_TYPE, Order*>();
    }
    Order::ORDER_TYPE orderType = order.orderType();
    if(!mOrders[originUniqueId].contains(orderType)){
        qWarning() << "Updating an an inexistant order of unit with ID : " << originUniqueId << " and order type " << orderType;
        mOrders[originUniqueId][orderType] = new Order(originUniqueId, orderType);
    }


    Order * oldOrder = mOrders[originUniqueId][orderType];
    *oldOrder = order;
    emit orderUpdated(order);

}

void Data::updateOrderFromUser(Order order)
{
    qDebug() << "Receive an update for order ";
    order.debug();
    int originUniqueId = order.originUniqueId();

    if(!mOrders.contains(originUniqueId)){
        qWarning() << "Updating an order of an inexistant unit with ID : " << originUniqueId;
        mOrders[originUniqueId] = QMap<Order::ORDER_TYPE, Order*>();
    }
    Order::ORDER_TYPE orderType = order.orderType();
    if(!mOrders[originUniqueId].contains(orderType)){
        qWarning() << "Updating an an inexistant order of unit with ID : " << originUniqueId << " and order type " << orderType;
        mOrders[originUniqueId][orderType] = new Order(originUniqueId, orderType);
    }

    QVector<int> descendants = getDescendants(originUniqueId);
    for(int descendantId : descendants){
        Order orderForDescendant = applySameOrder(order, descendantId);

        if(!mOrders.contains(descendantId)){
            mOrders[descendantId] = QMap<Order::ORDER_TYPE, Order*>();
        }
        if(!mOrders[descendantId].contains(orderType)){
            mOrders[descendantId][orderType] = new Order(descendantId, orderType);
        }

        Order * oldOrder = mOrders[descendantId][orderType];
        *oldOrder = orderForDescendant;
        emit orderUpdated(orderForDescendant);
    }


    //    Order * oldOrder = mOrders[originUniqueId][orderType];
    //    *oldOrder = order;

    //    emit orderUpdated(order);
}

void Data::resetOrderData(){
    // By default, everyone does a safe move (no move), is not suppressed and receives no attacks
    mOrderNbTimesUnderAttack.clear();
    mOrderSuppressed.clear();
    mOrderSafeMove.clear();

    for(Unit * unit : mUnits){
        int uniqueId = unit->uniqueId();
        mOrderNbTimesUnderAttack[uniqueId] = 0;
        mOrderSafeMove[uniqueId] = true;
    }

}

void Data::preprocessOrders()
{
    resetOrderData();

    for(auto map : mOrders){
        for (Order * order : map){
            if(order->isEnabled()){
                Unit * unit = mUnits[order->originUniqueId()];
                switch (order->orderType()) {
                case Order::MOVEMENT:{
                    QPointF destination = order->destinationPosition();
                    QPointF distanceCovered = destination - unit->pos();
                    qDebug() << "Distance covered by " << unit->displayedText() << " is " << distanceCovered;
                    if(QVector2D(distanceCovered).length() > 5){
                        mOrderSafeMove[order->originUniqueId()] = false;
                    }

                    break;
                }
                case Order::ATTACK:{
                    if(isALeafOnMap(order->originUniqueId())){
                        int targetId = order->destinationUniqueId();
                        if(mUnits.contains(targetId)){
                            mOrderNbTimesUnderAttack[targetId]++;
                            if(mOrderNbTimesUnderAttack[targetId] > 1){
                                mOrderSuppressed.insert(targetId);
                            }
                        }
                    }
                    break;
                }
                default:
                    break;

                }
            }
        }
    }
}

void Data::applyOrders()
{
    for(auto map : mOrders){
        for (Order * order : map){
            if(order->isEnabled()){
                Unit * unit = mUnits[order->originUniqueId()];
                switch (order->orderType()) {
                case Order::MOVEMENT:{
                    QPointF destination = order->destinationPosition();
                    unit->setPos(destination);
                    // No need to emit an update because we are going to move everything anyways
                    //                    emit unitPositionUpdated(order->originUniqueId(), destination);
                    break;
                }
                case Order::ATTACK:{
                    if(isALeafOnMap(order->originUniqueId())){
                        int targetId = order->destinationUniqueId();
                        if(mUnits.contains(targetId)){
                            float life = mUnits[targetId]->getLife();
                            float damage = 20;
                            // Shooter is supressed
                            if(mOrderSuppressed.contains(order->originUniqueId())){
                                damage /= 2.0;
                            }
                            // Shooter stays fixed
                            if(mOrderSafeMove[order->originUniqueId()]){
                                damage *= 2.0;
                            }
                            //Target stays fixed
                            if(mOrderSafeMove[order->destinationUniqueId()]){
                                damage /= 2.0;
                            }
                            damage *= (unit->getLife()/100.0);

                            mUnits[targetId]->setLife(life-damage);
                            emit unitContentUpdated(*(mUnits[targetId]));
                            emit unitPositionUpdated(targetId, mUnits[targetId]->pos());
                        }
                    }
                    break;
                }
                default:
                    break;

                }
            }
        }
    }

    computeAllPositionsFromLeaves();
}

int Data::siblingRank(int uniqueId) const
{
    if(mUnits.contains(uniqueId)){
        return -1;
    }
    if(mUnits.contains(mUnits[uniqueId]->parentId())){
        return -1;
    }
    return mUnits[mUnits[uniqueId]->parentId()]->children().indexOf(uniqueId);
}



void Data::debug()
{
    qDebug() << "DEBUG DATA";
    qDebug() << "Units. Nb units: " << mUnits.size();
    for(const Unit * unit : mUnits){
        qDebug().noquote() << *unit;
    }

    qDebug() << "Drawings. Nb drawings: " << mDrawings.size();
    for(const Unit * unit : mUnits){
        qDebug().noquote() << *unit;
    }

    qDebug() << "Orders";
    for(auto map : mOrders){
        for (Order * order : map){
            order->debug();
        }
    }

    qDebug() << "Who owns what";
    for(const Player * player : getTruePlayers()){
        qDebug() << player->ownedUnits();
    }
    emit debugSignal();
}

void Data::setCurrentTime(const QDateTime &currentTime)
{
    mCurrentTime = currentTime;
}

QDateTime Data::getCurrentTime() const
{
    return mCurrentTime;
}

QMap<QString, QSet<int> > Data::getPlayersOwnedUnits() const
{
    QMap<QString, QSet<int> > answer;
    for(Player * player : mTruePlayers){
        answer[player->name()] = player->ownedUnits();
    }
    return answer;
}

QSet<int> Data::getPlayerOwnedUnits(const QString & playerName) const
{
    QSet<int> answer;
    if(mTruePlayers.contains(playerName)){
        answer = mTruePlayers[playerName]->ownedUnits();
    }
    return answer;
}









///////////////////////////////////////////////////////////////////////
/// MISC                                                            ///
///////////////////////////////////////////////////////////////////////
Data::operator QString() const
{
    QString answer = QString("Data to QString. List of units : \n");
    for(const Unit * unit : mUnits){
        answer += unit->operator QString();
    }
    return answer;
}



