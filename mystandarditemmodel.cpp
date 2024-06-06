#include "mystandarditemmodel.h"
#include "data.h"



#include <QDebug>
#include <QMimeData>

///////////////////////////////////////////////////////////////////////
/// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
///////////////////////////////////////////////////////////////////////
///
MyStandardItemModel::MyStandardItemModel(Data * data, QObject *parent):
    QStandardItemModel (0, 4, parent),
    mData(data)
{
    init();
}

MyStandardItemModel::~MyStandardItemModel()
{
    
}

void MyStandardItemModel::init()
{
    setColumnCount(4);
    setHeaderData(0, Qt::Horizontal, QObject::tr("Unit"));
    setHeaderData(1, Qt::Horizontal, QObject::tr("Unique ID"));
    setHeaderData(2, Qt::Horizontal, QObject::tr("Add"));
    setHeaderData(3, Qt::Horizontal, QObject::tr("Delete"));
}

void MyStandardItemModel::clear()
{
    // TODO : check if I need to deleted the QStandardItems
    mStandardItems.clear();
    QStandardItemModel::clear();
    
    init();
}



///////////////////////////////////////////////////////////////////////
/// GETTERS, SETTERS                                                ///
///////////////////////////////////////////////////////////////////////
///
QMap<int, QStandardItem *> MyStandardItemModel::standardItems() const
{
    return mStandardItems;
}


///////////////////////////////////////////////////////////////////////
/// UPDATE DATA                                                     ///
///////////////////////////////////////////////////////////////////////


void MyStandardItemModel::addUniqueId(int uniqueId)
{
    if(mStandardItems.contains(uniqueId)){
        qWarning() << "ID " << uniqueId << " already exists";
        return;
    }
    
    QStandardItem * item = new QStandardItem(QString::number(uniqueId));
    mStandardItems[uniqueId] = item;
    

    QStandardItem * rootItem = invisibleRootItem();
    QList<QStandardItem *> items;
    items.push_back(item);
    items.push_back(new QStandardItem("Unknown text"));
    items.push_back(new QStandardItem(QIcon(":/files/icon/plus-64.png"), ""));
    //    if(canBeDeleted){
            items.push_back(new QStandardItem(QIcon(":/files/icon/bin.png"), ""));
    //    }
    //    else{
//    items.push_back(new QStandardItem(QIcon(":/files/icon/bin_off.png"), ""));
    //    }

    rootItem->appendRow(items);
}

// You already don't have parents or children
void MyStandardItemModel::deleteUniqueId(int uniqueId)
{
    if(!mStandardItems.contains(uniqueId)){
        qWarning() << "ID " << uniqueId << " cannot be deleted";
        Q_ASSERT(false);
    }
    
    QStandardItem * item = mStandardItems[uniqueId];

    getTrueParent(item)->removeRow(item->row());

    //    delete item;
    mStandardItems.remove(uniqueId);
    item = nullptr;
}


void MyStandardItemModel::updateUnitContent(Unit unit)
{
    int uniqueId = unit.uniqueId();
    QStandardItem * item = mStandardItems.value(uniqueId, nullptr);

    if(!item){
        qWarning() << "Standard item does not exist " << uniqueId;
        Q_ASSERT(false);
    }

    QModelIndex index = indexFromItem(item);
    QModelIndex displayedTextItem = sibling(item->row(), 1, index);
    itemFromIndex(displayedTextItem)->setData(QVariant{unit.displayedText()}, Qt::DisplayRole);
}

void MyStandardItemModel::removeUnitParent(int uniqueId)
{
    QStandardItem * item = mStandardItems.value(uniqueId, nullptr);

    if(!item){
        qWarning() << "Standard item does not exist " << uniqueId;
        Q_ASSERT(false);
    }
    changeUnitParent(-1, uniqueId);
}

void MyStandardItemModel::changeUnitParent(int parentId, int uniqueId)
{
    QStandardItem * item = mStandardItems.value(uniqueId, nullptr);
    if(!item){
        qWarning() << "Standard item does not exist " << uniqueId;
        return;
    }
    QStandardItem * parentItem = mStandardItems.value(parentId, invisibleRootItem());
    parentItem->appendRow(getTrueParent(item)->takeRow(item->row()));
}

QStandardItem * MyStandardItemModel::getTrueParent(QStandardItem *item)
{
    Q_ASSERT(item != invisibleRootItem());
    QStandardItem * parentItem = item->parent();
    if(parentItem == 0){
        parentItem = invisibleRootItem();
    }
    return parentItem;
}







///////////////////////////////////////////////////////////////////////
/// UPDATE VISUALIZATION                                            ///
///////////////////////////////////////////////////////////////////////
///
//void MyStandardItemModel::selectUnit(int uniqueId)
//{
//    if(!mStandardItems.contains(uniqueId)){
//        return;
//    }
//    QStandardItem * item = mStandardItems[uniqueId];
//    QModelIndex index = indexFromId(uniqueId);
    
//}


///////////////////////////////////////////////////////////////////////
/// VIEW MODEL                                                      ///
///////////////////////////////////////////////////////////////////////
///
QModelIndex MyStandardItemModel::indexFromId(int uniqueId) const
{
    if(!mStandardItems.contains(uniqueId)){
        qDebug() << "No index for " << uniqueId;
        return QModelIndex();
    }
//    qDebug() << "index for " << uniqueId;
    return indexFromItem(mStandardItems[uniqueId]);
}


Qt::ItemFlags MyStandardItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    
    // Icons are not editable
    //    if(index.column() == 0){
    //        return QAbstractItemModel::flags(index);
    //    }
    
    //        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
}


Qt::DropActions MyStandardItemModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

//Qt::ItemFlags DragDropListModel::flags(const QModelIndex &index) const
//{
//    Qt::ItemFlags defaultFlags = QStringListModel::flags(index);

//    if (index.isValid())
//        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
//    else
//        return Qt::ItemIsDropEnabled | defaultFlags;
//}

QStringList MyStandardItemModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    types << "application/vnd.text.list";
    return types;
}

QMimeData *MyStandardItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    if(indexes.size() != 1){
        qWarning() << "Nb of indexes is not 1";
        qWarning() << indexes;
        Q_ASSERT(false);
    }
    QString plainText = "-1";
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;

            QStandardItem * item = itemFromIndex(index);
            if(item){
                plainText = item->data(Qt::DisplayRole).toString();
                qDebug() << plainText;
                break;
            }
            qDebug() << "Index " << index;
            qDebug() << "Unit " << item->data(Qt::DisplayRole).toString();
        }
    }

    mimeData->setText(plainText);
    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}

void MyStandardItemModel::debug()
{
    qDebug() << "Root has nb children : " << invisibleRootItem()->rowCount();
    for(const QStandardItem * item : mStandardItems){
        qDebug().noquote() << "ID " << item->data(Qt::DisplayRole).toString() << "\tNb children: " << item->rowCount();
        //        qDebug().noquote() << QString(*item);
        //        qDebug().noquote() << item->operator QString();
    }
}
