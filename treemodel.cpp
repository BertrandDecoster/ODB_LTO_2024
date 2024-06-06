/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "treeitem.h"
#include "treemodel.h"
#include "unit.h"

//! [0]
TreeModel::TreeModel(const QStringList &headers, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    rootItem = new TreeItem(this, rootData);
}
//! [0]

//! [1]
TreeModel::~TreeModel()
{
    delete rootItem;
}

void TreeModel::addUniqueId(int uniqueId)
{
    if(mItems.contains(uniqueId)){
        qWarning() << "Id " << uniqueId << " already exists";
        return;
    }
    QVector<QVariant> data = {QVariant(uniqueId), QVariant(), QVariant(), QVariant()};
    TreeItem * newItem = new TreeItem(this, data, rootItem);
    mItems[uniqueId] = newItem;

    if(uniqueId == 0){
        beginInsertRows(QModelIndex(), 0, 0);
        qDebug() << "INSERT : root , 0";
        rootItem->insertChild(newItem);
        endInsertRows();
    }
}

void TreeModel::deleteUniqueId(int uniqueId)
{
    if(!mItems.contains(uniqueId)){
        qWarning() << "Id " << uniqueId << " does not exist";
        return;
    }
    TreeItem * item = mItems[uniqueId];
    if(item->parent()){
        qWarning() << "Delete item with parent: " << uniqueId;
        Q_ASSERT(false);
    }
    if(item->childCount()>0){
        qWarning() << "Delete item with children: " << uniqueId;
        Q_ASSERT(false);
    }
    mItems.remove(uniqueId);
    delete item;
    item = nullptr;
}

//void TreeModel::updateUnit(Unit unit)
//{
//    int uniqueId = unit.uniqueId();
//    if(!mItems.contains(uniqueId)){
//        qWarning() << "Can't update " << uniqueId;
//        return;
//    }
//    qDebug() << "Update " << unit.uniqueId();

//    // Set data
//    TreeItem * item = mItems[uniqueId];
//    //    mItems[uniqueId]->setData(0, QVariant(uniqueId));
//    item->setData(1, QVariant(unit.displayedText()));
//    item->setData(2, QVariant(QString("Add child")));
//    item->setData(3, QVariant(QString("Delete")));

//    // Set hierarchy
////    int parentId = unit.parentId();
////    if(mItems.contains(parentId)){
////        TreeItem * parentItem = mItems[parentId];
////        if(item->parent() != parentItem){
////            item->setParent(mItems[parentId]);
////            parentItem->insertChild(item);
////            //            parentItem->insertChildren(parentItem->childCount(), 1, item->columnCount());
////        }
////    }

//    layoutAboutToBeChanged();
//    for(int childIndex = 0; childIndex < item->childCount(); ++childIndex){
//        TreeItem * childItem = item->child(childIndex);
//        setItemParent(item, childItem, childIndex);
//    }
//    layoutChanged();
//}

void TreeModel::updateUnitContent(Unit unit)
{
    int uniqueId = unit.uniqueId();
    if(!mItems.contains(uniqueId)){
        qWarning() << "Can't update " << uniqueId;
        return;
    }
    qDebug() << "Update " << unit.uniqueId();

    // Set data
    TreeItem * item = mItems[uniqueId];
    //    mItems[uniqueId]->setData(0, QVariant(uniqueId));
    item->setData(1, QVariant(unit.displayedText()));
    item->setData(2, QVariant(QString("Add child")));
    item->setData(3, QVariant(QString("Delete")));

    // Set hierarchy
    //    int parentId = unit.parentId();
    //    if(mItems.contains(parentId)){
    //        TreeItem * parentItem = mItems[parentId];
    //        if(item->parent() != parentItem){
    //            item->setParent(mItems[parentId]);
    //            parentItem->insertChild(item);
    //            //            parentItem->insertChildren(parentItem->childCount(), 1, item->columnCount());
    //        }
    //    }


    //    for(int childIndex = 0; childIndex < item->childCount(); ++childIndex){
    //        TreeItem * childItem = item->child(childIndex);
    //        setItemParent(item, childItem, childIndex);
    //    }

    //    TreeItem *parentItem = getItem(parent);

    //    TreeItem *childItem = parentItem->child(row);
    //    if (childItem)
    //        return createIndex(row, column, item);


    //    QList<TreeItem*> children;
    //    beginRemoveRows(index(), );

    //    for(int childId : unit.children()){
    //        children.append(mItems[childId]);
    //    }
    //    item->setChildren(children);
    //    layoutChanged();
}

void TreeModel::removeUnitParent(int uniqueId)
{

    TreeItem * item = mItems[uniqueId];
    TreeItem * parentItem = item->parent();
    if(parentItem && parentItem->hasChild(item)){
        beginRemoveRows(indexFromItem(parentItem), item->childNumber(), item->childNumber());
        qDebug() << "REMOVE the parent of  : " << uniqueId;
        item->parent()->remove2Children(item);
        endRemoveRows();
    }
    item->setParent(nullptr);
}

void TreeModel::changeUnitParent(int parentId, int uniqueId)
{
    TreeItem * item = mItems[uniqueId];
    TreeItem * parentItem = mItems[parentId];

    beginInsertRows(indexFromItem(parentItem), parentItem->childCount(), parentItem->childCount());
    qDebug() << "INSERT : " << parentId << ", " << uniqueId;
    parentItem->insertChild(item);
    endInsertRows();
    item->setParent(mItems[parentId]);
}

void TreeModel::clear()
{
    beginResetModel();
    rootItem->removeChildren(0, rootItem->childCount());
    qDeleteAll(mItems);
    mItems.clear();
    endResetModel();
}

//void TreeModel::unitParentChanged(int uniqueId)
//{
//    unitParentChanged()
//}

//void TreeModel::unitParentChanged(int uniqueId, int parentId)
//{

//}

void TreeModel::unitParentChanged(int uniqueId, int parentId, int position)
{
    if(!mItems.contains(uniqueId)){
        qWarning() << "Can't change parent of " << uniqueId;
        return;
    }

    // Set data
    TreeItem * item = mItems[uniqueId];

    if(parentId < 0){
        item->setParent(rootItem); // OUCH
        return;
    }


}

//void TreeModel::updateUnit(const Unit *unit)
//{
//    int uniqueId = unit->uniqueId();
//    TreeItem *  item = mItems[uniqueId];
//    item->setData(0, unit->uniqueId());
//    item->setData(1, unit->uniqueId());
//    item->setData(2, unit->uniqueId());
//    item->setData(3, unit->uniqueId());



//}
//! [1]

//! [2]
int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}
//! [2]

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem *item = getItem(index);

    return item->data(index.column());
}

//! [3]
Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if(index.column() == 1){
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }

    return QAbstractItemModel::flags(index);
}
//! [3]

//! [4]
TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

//void TreeModel::setItemParent(TreeItem *parentItem, TreeItem *childItem, int position)
//{
//    if(childItem->parent() != parentItem){
//        childItem->setParent(parentItem);
//    }
//    QList<TreeItem*> children = parentItem->children();
//    children.removeAll(childItem);
//    parentItem->setChildren(children);
//    parentItem->insertChild(childItem, position);

//}
//! [4]

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::indexFromId(int uniqueId)
{
    if(mItems.contains(uniqueId)){
//        return createIndex(0, 0, mItems[uniqueId]);
        return indexFromItem(mItems[uniqueId]);
    }
    return QModelIndex();
}

QModelIndex TreeModel::indexFromItem(TreeItem *item)
{
    return createIndex(item->childNumber(), 0, item);
}

//! [5]
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
    //! [5]

    //! [6]
    TreeItem *parentItem = getItem(parent);
    TreeItem *childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [6]

//bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
//{
//    bool success;

//    beginInsertColumns(parent, position, position + columns - 1);
//    success = rootItem->insertColumns(position, columns);
//    endInsertColumns();

//    return success;
//}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

//! [7]
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if(!parentItem)
        return QModelIndex();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(childItem->childNumber(), 0, parentItem);
}
//! [7]

//bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
//{
//    bool success;

//    beginRemoveColumns(parent, position, position + columns - 1);
//    success = rootItem->removeColumns(position, columns);
//    endRemoveColumns();

//    if (rootItem->columnCount() == 0)
//        removeRows(0, rowCount());

//    return success;
//}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

bool TreeModel::insertCustomRow(int childPosition, int parentId)
{

}

void TreeModel::debug()
{
    for(const TreeItem * item : mItems){
        qDebug().noquote() << *item;
        //        qDebug().noquote() << QString(*item);
        //        qDebug().noquote() << item->operator QString();
    }
}

//! [8]
int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}
//! [8]

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

//void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
//{
//    QList<TreeItem*> parents;
//    QList<int> indentations;
//    parents << parent;
//    indentations << 0;

//    int number = 0;

//    while (number < lines.count()) {
//        int position = 0;
//        while (position < lines[number].length()) {
//            if (lines[number].at(position) != ' ')
//                break;
//            ++position;
//        }

//        QString lineData = lines[number].mid(position).trimmed();

//        if (!lineData.isEmpty()) {
//            // Read the column data from the rest of the line.
//            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
//            QVector<QVariant> columnData;
//            for (int column = 0; column < columnStrings.count(); ++column)
//                columnData << columnStrings[column];

//            if (position > indentations.last()) {
//                // The last child of the current parent is now the new parent
//                // unless the current parent has no children.

//                if (parents.last()->childCount() > 0) {
//                    parents << parents.last()->child(parents.last()->childCount()-1);
//                    indentations << position;
//                }
//            } else {
//                while (position < indentations.last() && parents.count() > 0) {
//                    parents.pop_back();
//                    indentations.pop_back();
//                }
//            }

//            // Append a new item to the current parent's list of children.
//            TreeItem *parent = parents.last();
//            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
//            for (int column = 0; column < columnData.size(); ++column)
//                parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
//        }

//        ++number;
//    }
//}
