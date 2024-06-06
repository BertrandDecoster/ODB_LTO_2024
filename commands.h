#ifndef COMMANDS_H
#define COMMANDS_H

#include <QJsonObject>
#include <QMap>
#include <QUndoCommand>

#include "unit.h"
#include "drawing.h"

class Data;

///////////////////////////////////////////////////////////////////////
/// LOAD                                                            ///
///////////////////////////////////////////////////////////////////////

class LoadCommand : public QUndoCommand
{
public:
    enum { Id = 1234 };

    LoadCommand(Data * data, const QString & filename,
                QUndoCommand *parent = nullptr);
    LoadCommand(Data * data, QJsonObject & newData,
                QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    QString mFilename;
    QJsonObject mOldData;
    QJsonObject mNewData;
};



///////////////////////////////////////////////////////////////////////
/// ADD UNIT                                                        ///
///////////////////////////////////////////////////////////////////////
/// It's expected that:
///  - it has no child
///  - it can have a parent
///  - it is appended to the end of the parent's child list

class AddUnitCommand : public QUndoCommand{
public:
    enum { Id = 1235 };

    AddUnitCommand(Data * data, Unit unit,
                   QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    Unit mUnit;
};



///////////////////////////////////////////////////////////////////////
/// DELETE UNIT                                                     ///
///////////////////////////////////////////////////////////////////////
/// This command also deletes all the unit's children recursively

class DeleteUnitCommand : public QUndoCommand{
public:
    enum { Id = 1236 };

    DeleteUnitCommand(Data * data, int uniqueId,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    QVector<Unit> mUnits; // Delete from right to left, undelete from left to right
};



///////////////////////////////////////////////////////////////////////
/// UPDATE UNIT                                                     ///
///////////////////////////////////////////////////////////////////////
class UpdateUnitCommand : public QUndoCommand{
public:
    enum { Id = 1237 };

    UpdateUnitCommand(Data * data, Unit unit,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    Unit mPreviousUnit;
    Unit mNextUnit;
};

///////////////////////////////////////////////////////////////////////
/// CHANGE UNIT PARENT                                              ///
///////////////////////////////////////////////////////////////////////
class ChangeUnitParentCommand : public QUndoCommand{
public:
    enum { Id = 1238 };

    ChangeUnitParentCommand(Data * data, Unit previousUnit, Unit nextUnit,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    Unit mPreviousUnit;
    Unit mNextUnit;
};

///////////////////////////////////////////////////////////////////////
/// MOVE UNIT                                                       ///
///////////////////////////////////////////////////////////////////////
class MoveUnitCommand : public QUndoCommand{
public:
    enum { Id = 1239 };

    MoveUnitCommand(Data * data, int uniqueId, QPointF position,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    int mUniqueId;  // For merge
    QMap<int, QPointF> mOldPositions;
    QMap<int, QPointF> mNewPositions;
};

///////////////////////////////////////////////////////////////////////
/// ADD DRAWING                                                     ///
///////////////////////////////////////////////////////////////////////
/// It's expected that:
///  - it has no child
///  - it can have a parent
///  - it is appended to the end of the parent's child list

class AddDrawingCommand : public QUndoCommand{
public:
    enum { Id = 1240 };

    AddDrawingCommand(Data * data, Drawing drawing,
                   QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    Drawing mDrawing;
};



///////////////////////////////////////////////////////////////////////
/// DELETE DRAWING                                                  ///
///////////////////////////////////////////////////////////////////////
/// This command also deletes all the unit's children recursively

class DeleteDrawingCommand : public QUndoCommand{
public:
    enum { Id = 1241 };

    DeleteDrawingCommand(Data * data, int uniqueId,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    Drawing mDrawing; // Delete from right to left, undelete from left to right
};

///////////////////////////////////////////////////////////////////////
/// UPDATE DRAWING                                              ///
///////////////////////////////////////////////////////////////////////
class UpdateDrawingCommand : public QUndoCommand{
public:
    enum { Id = 1242 };

    UpdateDrawingCommand(Data * data, Drawing previousDrawing, Drawing nextDrawing,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return Id; }
    QString toString() const;

private:
    Data * mData;
    Drawing mPreviousDrawing;
    Drawing mNextDrawing;
};



#endif // COMMANDS_H
