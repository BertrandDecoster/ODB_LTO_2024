#ifndef MYSTANDARDITEMMODEL_H
#define MYSTANDARDITEMMODEL_H

#include <QStandardItemModel>
#include "unit.h"

class Data;

class MyStandardItemModel : public QStandardItemModel
{
    Q_OBJECT

    ///////////////////////////////////////////////////////////////////////
    /// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
    ///////////////////////////////////////////////////////////////////////
public:
    MyStandardItemModel(Data * data, QObject *parent = Q_NULLPTR);
    ~MyStandardItemModel() override;
    void init();

public slots:
        void clear();

    ///////////////////////////////////////////////////////////////////////
    /// GETTERS, SETTERS                                                ///
    ///////////////////////////////////////////////////////////////////////
    QMap<int, QStandardItem *> standardItems() const;

    ///////////////////////////////////////////////////////////////////////
    /// UPDATE DATA                                                     ///
    ///////////////////////////////////////////////////////////////////////
public slots:
    void addUniqueId(int uniqueId);
    void deleteUniqueId(int uniqueId);
    void updateUnitContent(Unit unit);
    void removeUnitParent(int uniqueId);
    void changeUnitParent(int parentId, int uniqueId);


private:
    QStandardItem * getTrueParent(QStandardItem * item); // rootInvisibleItem and not nullptr



    ///////////////////////////////////////////////////////////////////////
    /// UPDATE VISUALIZATION                                            ///
    ///////////////////////////////////////////////////////////////////////
//    void selectUnit(int uniqueId);


    ///////////////////////////////////////////////////////////////////////
    /// VIEW MODEL                                                      ///
    ///////////////////////////////////////////////////////////////////////
public:
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex indexFromId(int uniqueId) const;

    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;

public slots:
    void debug();
    ///////////////////////////////////////////////////////////////////////
    /// ATTRIBUTES                                                      ///
    ///////////////////////////////////////////////////////////////////////
private:
    Data * mData;

    QMap<int, QStandardItem *> mStandardItems;


};

#endif // MYSTANDARDITEMMODEL_H
