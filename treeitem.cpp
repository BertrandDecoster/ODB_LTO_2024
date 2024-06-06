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

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include "treeitem.h"


//! [0]
TreeItem::TreeItem(TreeModel * model, const QVector<QVariant> &data, TreeItem *parent):
    mTreeModel(model),
    mParentItem(parent),
    mItemData(data)
{

}
//! [0]

//! [1]
TreeItem::~TreeItem()
{

}
//! [1]

//! [2]
TreeItem *TreeItem::child(int number)
{
    return mChildItems.value(number);
}
//! [2]

//! [3]
int TreeItem::childCount() const
{
    return mChildItems.size();
}

QList<TreeItem *> TreeItem::children() const
{
    return mChildItems;
}

void TreeItem::clearChildren()
{
    mChildItems.clear();
}
//! [3]

//! [4]
int TreeItem::childNumber() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}
//! [4]

//! [5]
int TreeItem::columnCount() const
{
    return mItemData.count();
}
//! [5]

//! [6]
QVariant TreeItem::data(int column) const
{
    return mItemData.value(column);
}
//! [6]

//! [7]
bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > mChildItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(mTreeModel, data, this);
        mChildItems.insert(position, item);
    }

    return true;
}

bool TreeItem::insertChild(TreeItem *item, int position)
{
    int truePosition = position;
    if (position < 0 || position > mChildItems.size())
        truePosition = mChildItems.size();
    mChildItems.insert(truePosition, item);

    return true;
}
//! [7]

//! [8]
//bool TreeItem::insertColumns(int position, int columns)
//{
//    if (position < 0 || position > itemData.size())
//        return false;

//    for (int column = 0; column < columns; ++column)
//        itemData.insert(position, QVariant());

//    foreach (TreeItem *child, childItems)
//        child->insertColumns(position, columns);

//    return true;
//}
//! [8]

//! [9]
TreeItem *TreeItem::parent()
{
    return mParentItem;
}

void TreeItem::setChildren(QList<TreeItem *> childItems)
{
    mChildItems = childItems;
}

void TreeItem::setParent(TreeItem *p)
{
    mParentItem = p;
}
//! [9]

//! [10]
bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > mChildItems.size())
        return false;

    for (int row = 0; row < count; ++row){
        //        delete mChildItems.takeAt(position);
        TreeItem * childItem = mChildItems.takeAt(position);
    }

    return true;
}
//! [10]

//bool TreeItem::removeColumns(int position, int columns)
//{
//    if (position < 0 || position + columns > itemData.size())
//        return false;

//    for (int column = 0; column < columns; ++column)
//        itemData.remove(position);

//    foreach (TreeItem *child, childItems)
//        child->removeColumns(position, columns);

//    return true;
//}

//! [11]
bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= mItemData.size())
        return false;

    mItemData[column] = value;
    return true;
}

int TreeItem::uniqueId() const
{
    if(mItemData.empty()){
        return -1;
    }
    return mItemData[0].toInt();
}

bool TreeItem::remove2Children(TreeItem * childItem)
{
    mChildItems.removeAll(childItem);
}

bool TreeItem::hasChild(TreeItem *childItem)
{
    return mChildItems.contains(childItem);
}

TreeItem::operator QString() const
{
    QString answer = QString("TreeItem: Id:") + QString::number(uniqueId()) + "\n\tChildren: ";
    for(int index = 0; index<childCount(); ++index){
        answer += QString::number(mChildItems[index]->uniqueId()) + "\t";
    }
    answer += "\n\tData: ";
    answer += data(1).toString();
    return answer;
}
//! [11]
