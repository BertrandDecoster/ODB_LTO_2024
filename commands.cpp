#include "commands.h"

#include "data.h"

#include <QJsonDocument>
#include <QDebug>

LoadCommand::LoadCommand(Data *data, const QString &filename, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data),
    mFilename(filename)
{
    mData->writeJson(mOldData);

    QFile loadFile(filename);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file : " << (filename);
        Q_ASSERT(false);
    }
    QByteArray saveData = loadFile.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(saveData));
    const QJsonObject & newData = jsonDoc.object();

//    qDebug() << "Size saveData " << saveData.size();
//    qDebug() << "NEW DATA " << newData;

    mNewData = newData;
}

LoadCommand::LoadCommand(Data *data, QJsonObject &newData, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data),
    mNewData(newData)
{
    mData->writeJson(mOldData);
}

void LoadCommand::undo()
{
//    qDebug() << "Loading JSON data " << mOldData;
    mData->loadJson(mOldData);
//    mData->selectUnit(0);
}

void LoadCommand::redo()
{
//    qDebug() << "Loading file " << mFilename;
    mData->loadJson(mNewData);
//    mData->selectUnit(0);
}

bool LoadCommand::mergeWith(const QUndoCommand *command)
{
    const LoadCommand * loadCommand = static_cast<const LoadCommand *>(command);
    mOldData = loadCommand->mOldData;
    return true;
}

QString LoadCommand::toString() const
{
    return QString("COMMAND;Load");
}


///////////////////////////////////////////////////////////////////////
/// ADD UNIT                                                        ///
///////////////////////////////////////////////////////////////////////

AddUnitCommand::AddUnitCommand(Data *data, Unit unit, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data),
    mUnit(unit)
{

}

void AddUnitCommand::undo()
{
    int parentUniqueId = mUnit.parentId();
    mData->deleteUniqueId(mUnit.uniqueId());
    mData->selectUnit(parentUniqueId);
}

void AddUnitCommand::redo()
{
    qDebug().noquote() << toString();
    mData->createUnit(mUnit);
    mData->selectUnit(mUnit.parentId());
}

bool AddUnitCommand::mergeWith(const QUndoCommand *command)
{
    return false;
}

QString AddUnitCommand::toString() const
{
    return QString("COMMAND;AddUnit") + mUnit.operator QString();
}

//DeleteUnitCommand::DeleteUnitCommand(Data *data, Unit unit, QUndoCommand *parent):
//    QUndoCommand (parent),
//    mData(data),
//    mUnit(unit)
//{

//}

//void DeleteUnitCommand::undo()
//{
//    mData->createUnit(mUnit);
//    mData->selectUnit(mUnit.parent());
//}

//void DeleteUnitCommand::redo()
//{
//    qDebug() << toString();
//    QString parentUniqueId = mUnit.parent();
//    mData->deleteUnit(mUnit.uniqueId());
//    mData->selectUnit(parentUniqueId);
//}

//bool DeleteUnitCommand::mergeWith(const QUndoCommand *command)
//{
//    return false;
//}

//QString DeleteUnitCommand::toString() const
//{
//    return QString("COMMAND;DeleteUnit");
//}

///////////////////////////////////////////////////////////////////////
/// UPDATE UNIT                                                     ///
///////////////////////////////////////////////////////////////////////

UpdateUnitCommand::UpdateUnitCommand(Data *data, Unit unit, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data),
    mNextUnit(unit)
{
    mPreviousUnit = *(mData->units()[unit.uniqueId()]);
}

void UpdateUnitCommand::undo()
{
    mData->updateUnit(mPreviousUnit);
}

void UpdateUnitCommand::redo()
{
    qDebug().noquote() << toString();
    mData->updateUnit(mNextUnit);
}

bool UpdateUnitCommand::mergeWith(const QUndoCommand *command)
{
    // TODO of course merge (at least for life and position)
    return false;
}

QString UpdateUnitCommand::toString() const
{
    return QString("COMMAND;UpdateUnit ") + mNextUnit.operator QString();
}


///////////////////////////////////////////////////////////////////////
/// DELETE UNIT                                                     ///
///////////////////////////////////////////////////////////////////////

DeleteUnitCommand::DeleteUnitCommand(Data *data, int uniqueId, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data)
{
    mUnits = mData->getFullDescendants(uniqueId);
    qDebug().noquote() << "DELETE UNITS " << mUnits;
}

void DeleteUnitCommand::undo()
{
    // Insert units in the right order
    for(int index = 0; index < mUnits.size(); ++index){
        mData->createUnit(mUnits[index]);
    }
}

void DeleteUnitCommand::redo()
{
    int index = mUnits.size();
    if(!(index>0)){
        qWarning() << "Delete a negative number of elements";
        Q_ASSERT(false);
    }
    while(index --> 0){ // "goes down to 0" operator
        qDebug() << "DELETE ID " << mUnits[index].uniqueId();
        mData->deleteUniqueId(mUnits[index].uniqueId());
    }
}

bool DeleteUnitCommand::mergeWith(const QUndoCommand *command)
{
    const DeleteUnitCommand * deleteUnitsCommand = static_cast<const DeleteUnitCommand *>(command);
    mUnits = mUnits + deleteUnitsCommand->mUnits; // TODO check
    return true;
}

QString DeleteUnitCommand::toString() const
{
        return QString("COMMAND;DeleteUnits");
}


///////////////////////////////////////////////////////////////////////
/// MOVE UNIT                                                       ///
///////////////////////////////////////////////////////////////////////

MoveUnitCommand::MoveUnitCommand(Data *data, int uniqueId, QPointF position, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data),
    mUniqueId(uniqueId)
{

    qDebug() << "Spawn MOVE COMMAND " << position;
    mData->getTheoreticalMovements(uniqueId, position, mOldPositions, mNewPositions);
}

void MoveUnitCommand::undo()
{
    for(auto it = mOldPositions.constBegin(); it != mOldPositions.constEnd(); ++it){
        mData->updateUnitPosition(it.key(), it.value());
    }
}

void MoveUnitCommand::redo()
{
    for(auto it = mNewPositions.constBegin(); it != mNewPositions.constEnd(); ++it){
        mData->updateUnitPosition(it.key(), it.value());
    }
}

bool MoveUnitCommand::mergeWith(const QUndoCommand *command)
{
//    return false;
    const MoveUnitCommand * moveUnitCommand = static_cast<const MoveUnitCommand *>(command);
    if(mUniqueId != moveUnitCommand->mUniqueId){
        return false;
    }

    QMap<int, QPointF> oldP = moveUnitCommand->mOldPositions;
    for(auto it = mOldPositions.constBegin(); it != mOldPositions.constEnd(); ++it){
        oldP[it.key()] = it.value();
    }
    mOldPositions = oldP;

    QMap<int, QPointF> newP = mNewPositions;
    for(auto it = moveUnitCommand->mNewPositions.constBegin(); it != moveUnitCommand->mNewPositions.constEnd(); ++it){
        newP[it.key()] = it.value();
    }
    mNewPositions = newP;

    return true;
}

QString MoveUnitCommand::toString() const
{
    return QString("COMMAND;MoveUnitCommand");
}

AddDrawingCommand::AddDrawingCommand(Data *data, Drawing drawing, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data),
    mDrawing(drawing)
{

}

void AddDrawingCommand::undo()
{
    mData->deleteDrawing(mDrawing.uniqueId());
}

void AddDrawingCommand::redo()
{
    mData->createDrawing(mDrawing);
}

bool AddDrawingCommand::mergeWith(const QUndoCommand *command)
{
    return false;
}

QString AddDrawingCommand::toString() const
{
    return QString("COMMAND;AddDrawingCommand");
}

DeleteDrawingCommand::DeleteDrawingCommand(Data *data, int uniqueId, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data)
{
    mDrawing = Drawing(*(mData->drawings()[uniqueId]));
}

void DeleteDrawingCommand::undo()
{
    mData->createDrawing(mDrawing);
}

void DeleteDrawingCommand::redo()
{
    mData->deleteDrawing(mDrawing.uniqueId());
}

bool DeleteDrawingCommand::mergeWith(const QUndoCommand *command)
{
    return false;
}

QString DeleteDrawingCommand::toString() const
{
    return QString("COMMAND;DeleteDrawingCommand");
}



UpdateDrawingCommand::UpdateDrawingCommand(Data *data, Drawing previousDrawing, Drawing nextDrawing, QUndoCommand *parent):
    QUndoCommand (parent),
    mData(data),
    mPreviousDrawing(previousDrawing),
    mNextDrawing(nextDrawing)
{

}

void UpdateDrawingCommand::undo()
{
    mData->updateDrawing(mPreviousDrawing);
}

void UpdateDrawingCommand::redo()
{
    mData->updateDrawing(mNextDrawing);
}

bool UpdateDrawingCommand::mergeWith(const QUndoCommand *command)
{
    const UpdateDrawingCommand * moveUnitCommand = static_cast<const UpdateDrawingCommand *>(command);
    if(mNextDrawing.uniqueId() != moveUnitCommand->mNextDrawing.uniqueId()){
        return false;
    }

    // Keep the old drawing
    // Only update the next
    mNextDrawing = moveUnitCommand->mNextDrawing;
    return true;
}

QString UpdateDrawingCommand::toString() const
{
    return QString("COMMAND;UpdateDrawingCommand");
}
