#include "mytaskorggraphicsscene.h"

#include "unit.h"
#include "mainwindow.h"
#include "data.h"

#include <QDebug>
#include <QFile>
#include <QGraphicsItemAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QGraphicsView>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QStandardItemModel>
#include <QTimeLine>
#include <QGraphicsPixmapItem>


///////////////////////////////////////////////////////////////////////
/// STATIC                                                          ///
///////////////////////////////////////////////////////////////////////
///
qreal MyTaskOrgGraphicsScene::textSize = 3.5;

qreal MyTaskOrgGraphicsScene::centerX = 100.0;
qreal MyTaskOrgGraphicsScene::centerY = 100.0;

qreal MyTaskOrgGraphicsScene::parentX = 200.0;
qreal MyTaskOrgGraphicsScene::parentY = 0.0;

qreal MyTaskOrgGraphicsScene::siblingDeltaX = 200.0;

qreal MyTaskOrgGraphicsScene::parentToChildDeltaX = 50.0;
qreal MyTaskOrgGraphicsScene::parentToChildDeltaY = 100.0;

qreal MyTaskOrgGraphicsScene::childDeltaY = 100.0;

int MyTaskOrgGraphicsScene::animationDuration = 1000;


///////////////////////////////////////////////////////////////////////
/// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
///////////////////////////////////////////////////////////////////////
///
MyTaskOrgGraphicsScene::MyTaskOrgGraphicsScene(Data * d, QObject *parent):
    QGraphicsScene (parent),
    mData(d)
{
    mDisplayFullFamily = true;
}

MyTaskOrgGraphicsScene::~MyTaskOrgGraphicsScene()
{

}

void MyTaskOrgGraphicsScene::clear()
{
    if(animationGroup){
        qDebug() << "INTERRUPT ANIMATION AND SHIT";
        animationGroup->stop();
        animationGroup->deleteLater();
        animationGroup = nullptr;
    }
    //    qDebug() <<"Clear scene";
    mItems.clear();

    // No point to throw the lines/paths
    //    for(MyGraphicsLineItem * line : mUsedLines){
    //        mFreeLines.push_back(line);
    //    }
    mFreeLines.clear();
    mUsedLines.clear();

    //    for(MyGraphicsPathItem * path: mUsedPaths){
    //        mFreePaths.push_back(path);
    //    }
    mFreePaths.clear();
    mUsedPaths.clear();

    mSelectedUniqueId = -1;

    QGraphicsScene::clear();
}



///////////////////////////////////////////////////////////////////////
/// GETTERS, SETTERS                                                ///
///////////////////////////////////////////////////////////////////////
///
MyGraphicsLineItem *MyTaskOrgGraphicsScene::getLine(QGraphicsScene * scene)
{
    if(mFreeLines.empty()){
        MyGraphicsLineItem * item = new MyGraphicsLineItem();
        scene->addItem(item);
        item->hide();
        mFreeLines.push_back(item);
    }
    MyGraphicsLineItem * answer = mFreeLines.takeLast();
    answer->show();
    mUsedLines.push_back(answer);
    return answer;
}

MyGraphicsPathItem *MyTaskOrgGraphicsScene::getPath(QGraphicsScene *scene)
{
    if(mFreePaths.empty()){
        MyGraphicsPathItem * item = new MyGraphicsPathItem();
        scene->addItem(item);
        item->hide();
        mFreePaths.push_back(item);
    }
    MyGraphicsPathItem * answer = mFreePaths.takeLast();
    answer->show();
    mUsedPaths.push_back(answer);
    return answer;
}

///////////////////////////////////////////////////////////////////////
/// UPDATE THE DATA STRUCTURE                                       ///
///////////////////////////////////////////////////////////////////////
//void MyTaskOrgGraphicsScene::loadGame()
//{
//    clear();
//    auto units = mData->units();
//    //    qDebug() << "Scene trying to load " << QString::number(units.size()) << "elements";

//    for(Unit * unit : units){
//        int uniqueId = unit->uniqueId();
//        addUniqueId(uniqueId);
//    }

//    qDebug() << "Scene loaded with " << QString::number(mItems.size()) << "/" << QString::number(units.size()) << " elements";
//}

void MyTaskOrgGraphicsScene::addUniqueId(int uniqueId)
{
    auto units = mData->units();
    if(!units.contains(uniqueId)){
        return;
    }

    // Grab the icon from the Unit and put it in a QGraphicsPixmapItem
    Unit * unit = mData->units()[uniqueId];
    QIcon icon = unit->getIcon();
    QSize size = QSize(Unit::iconTotalWidth, Unit::iconTotalHeight);
    MyUnitItem * item = new MyUnitItem(unit->uniqueId(), icon.pixmap(size));
    item->mUniqueId = uniqueId;
    item->setScale(0.4);
    item->setFlag(QGraphicsItem::ItemIsMovable);
//    item->setFlag(QGraphicsItem::ItemIsSelectable);

    item->setAcceptHoverEvents(true);
//    item->installSceneEventFilter(new HoverBoxFilter(this));
    item->hide();
    addItem(item);
    mItems[uniqueId] = item;


    // Well, you have added something, it may impact the visuals
    selectUnit(mSelectedUniqueId);
}

void MyTaskOrgGraphicsScene::deleteUniqueId(int uniqueId)
{
    if(!mItems.contains(uniqueId)){
        return;
    }
    removeItem(mItems[uniqueId]);
    delete mItems[uniqueId];
    mItems.remove(uniqueId);
}

//void MyGraphicsScene::deleteUniqueIds(const QVector<int> &uniqueIds)
//{
//    for(int uniqueId : uniqueIds){
//        deleteUniqueId(uniqueId);
//    }
//}

//void MyTaskOrgGraphicsScene::updateUnit(int uniqueId)
//{
//    deleteUniqueId(uniqueId);
//    addUniqueId(uniqueId);
//}

void MyTaskOrgGraphicsScene::updateUnitContent(Unit unit)
{
    int uniqueId = unit.uniqueId();
    deleteUniqueId(uniqueId);
    addUniqueId(uniqueId);
}

void MyTaskOrgGraphicsScene::changeUnitParent(int parentId, int uniqueId)
{

}

void MyTaskOrgGraphicsScene::removeUnitParent(int uniqueId)
{

}

void MyTaskOrgGraphicsScene::debug()
{

}

///////////////////////////////////////////////////////////////////////
/// UPDATE THE DISPLAY                                              ///
///////////////////////////////////////////////////////////////////////
///
void MyTaskOrgGraphicsScene::selectUnit(int uniqueId)
{
    if(!mItems.contains(uniqueId)){
        return;
    }
    //    if(mSelectedUniqueId == uniqueId){
    //        return;
    //    }
    mSelectedUniqueId = uniqueId;

//    qDebug() << "Update centered on " << uniqueId;


    // The stuff we build

    QVector<MyAnimationPos> animationData;
    QSet<QGraphicsItem * > beginVisibleItems;
    QSet<QGraphicsItem * > endVisibleItems;
    QVector<int> family;
    QVector<QVector<int>> itemPlacement = getItemPlacement(uniqueId);
    QVector<bool> identicalColumns = QVector<bool>(itemPlacement.size(), false);

    for(int col=0; col < qMin(itemPlacement.size(), mItemPlacement.size()); ++col){
        if(itemPlacement[col] == mItemPlacement[col]){
            identicalColumns[col] = true;
        }
    }

    for(QGraphicsItem * unit : mItems){
        if(unit->isVisible()){
            beginVisibleItems.insert(unit);
        }
    }


    // Start with the central item
    QGraphicsItem * center = mItems[uniqueId];
    if(!center){
        qDebug() << "Bug trying to display" << uniqueId;
        return;
    }

    endVisibleItems.insert(center);
    animationData.push_back(MyAnimationPos{uniqueId, centerX, centerY, centerX, centerY});


    // Children
    int childIndex = 0;
    Unit * unit = mData->units()[uniqueId];
    for(int childId : unit->children()){
        QGraphicsItem * childItem = mItems[childId];
        endVisibleItems.insert(childItem);
        animationData.push_back(MyAnimationPos{childId,
                                               centerX + parentToChildDeltaX,
                                               centerY,
                                               centerX + parentToChildDeltaX,
                                               centerY + parentToChildDeltaY + childDeltaY*childIndex});
        childIndex++;
    }
    family.push_back(childIndex);

    if(mDisplayFullFamily){
        //Parent
        Unit * unitParent = mData->units().value(unit->parentId(), nullptr);
        if(unitParent){
            int parentId = unitParent->uniqueId();
            //        if(parentString.isNull() | parentString.isEmpty()){
            //            return;
            //        }
            if(!(parentId<0)){
                QGraphicsItem * parentGraphicsItem = mItems[parentId];
                endVisibleItems.insert(parentGraphicsItem);
                animationData.push_back(MyAnimationPos{parentId,
                                                       parentX,
                                                       parentY,
                                                       parentX,
                                                       parentY});
            }
        }
        else{
//            qDebug() << "No parent for : " << uniqueId;
        }


        int brotherIndex = 1;
        for(int brother : mData->siblings(uniqueId)){

            QGraphicsItem * brotherItem = mItems[brother];
            endVisibleItems.insert(brotherItem);
            animationData.push_back(MyAnimationPos{brother,
                                                   centerX,
                                                   centerY,
                                                   centerX + siblingDeltaX*brotherIndex,
                                                   centerY});

            childIndex = 0;
            for(int nephewString : mData->children(brother)){
                QGraphicsItem * nephewItem = mItems[nephewString];
                endVisibleItems.insert(nephewItem);
                animationData.push_back(MyAnimationPos{nephewString,
                                                       centerX + siblingDeltaX*brotherIndex + parentToChildDeltaX,
                                                       centerY,
                                                       centerX + siblingDeltaX*brotherIndex + parentToChildDeltaX,
                                                       centerY + parentToChildDeltaY + childDeltaY*childIndex});

                childIndex++;
            }
            family.push_back(childIndex);
            brotherIndex++;
        }
    }

    // All data computed, launch the animation

    // Visible items don't teleport
    for(MyAnimationPos & animData : animationData){
        QGraphicsItem * unit = mItems[animData.id];
        if(unit->isVisible()){
            animData.startX = unit->x();
            animData.startY = unit->y();
        }
    }

    QSet<QGraphicsItem * > appearingItems = endVisibleItems - beginVisibleItems;
    QSet<QGraphicsItem * > dissappearingItems = beginVisibleItems - endVisibleItems;
    QSet<QGraphicsItem * > stayVisibleItems = endVisibleItems.intersect(beginVisibleItems);

    animationGroup = new QParallelAnimationGroup();

    for(const MyAnimationPos & animData : animationData){
        launchAnimationPos(animData, animationGroup);
    }

    for(QGraphicsItem * unit : appearingItems){
        launchAnimationOpacity(unit, true, animationGroup);
    }

    for(QGraphicsItem * unit : dissappearingItems){
        launchAnimationOpacity(unit, false, animationGroup);
    }

    for(QGraphicsItem * unit : stayVisibleItems){
        launchAnimationOpacity(unit, true, animationGroup);
        //        launchAnimationOpacityFinish(unit, animationGroup);
    }


    launchAnimationFamilyLines(family, animationGroup);

    connect(animationGroup, &QParallelAnimationGroup::finished, [this](){resizeToBoundingRect(); animationGroup->deleteLater(); animationGroup = nullptr;});
    animationGroup->start();
    //    qDebug() << "Update done";
    mItemPlacement = getItemPlacement(mSelectedUniqueId);
}


void MyTaskOrgGraphicsScene::launchAnimationPos(const MyAnimationPos &data, QParallelAnimationGroup * animationGroup)
{
    MyUnitItem * unitItem = qgraphicsitem_cast<MyUnitItem*>(mItems[data.id]);
    unitItem->mFixedPos = QPointF(data.endX, data.endY);

    if((data.startX == data.endX) && (data.startY == data.endY)){
        //        qDebug() << "Skip " << data.id;
        mItems[data.id]->setPos(data.endX, data.endY);
        return;
    }
    //    qDebug() << "Anim " << data.id;


    QPropertyAnimation *animation = new QPropertyAnimation(unitItem, "pos");
    animation->setDuration(animationDuration);
    animation->setStartValue(QPointF(data.startX, data.startY));
    animation->setEndValue(QPointF(data.endX, data.endY));
    animation->setEasingCurve(QEasingCurve::InOutExpo);
    if(animationGroup){
        animationGroup->addAnimation(animation);
    }
    else{
        animation->start();
    }
}

void MyTaskOrgGraphicsScene::launchAnimationOpacity(QGraphicsItem *unit, bool isAppearing, QParallelAnimationGroup *animationGroup)
{
    MyUnitItem* u = qgraphicsitem_cast<MyUnitItem*>(unit);
    QPropertyAnimation *animation = new QPropertyAnimation(u, "opacity");
    animation->setDuration(animationDuration);
    if(isAppearing){
        if(unit->isVisible()){
            animation->setStartValue(unit->opacity());
        }
        else{
            animation->setStartValue(0.0);
        }
        // Make the selected unit appear immediately, so you can iterate fast over small changes
        if(u->mUniqueId == mSelectedUniqueId){
            animation->setStartValue(1.0);
        }
        unit->show();
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::InExpo);
    }
    else{
        animation->setStartValue(unit->opacity());
        animation->setEndValue(0.0);
        animation->setEasingCurve(QEasingCurve::OutExpo);
        connect(animation, &QPropertyAnimation::finished, [=](){unit->hide();});
    }

    if(animationGroup){
        animationGroup->addAnimation(animation);
    }
    else{
        animation->start();
    }
}


void MyTaskOrgGraphicsScene::launchAnimationFamilyLines(const QVector<int> &family, QParallelAnimationGroup *animationGroup)
{
    //    qDebug() << "Animation of the family " << family;
    for(MyGraphicsLineItem * line : mUsedLines){
        line->setOpacity(1.0);
        line->hide();
        mFreeLines.push_back(line);
    }
    mUsedLines.clear();

    for(MyGraphicsPathItem * path : mUsedPaths){
        path->setOpacity(1.0);
        path->hide();
        mFreePaths.push_back(path);
    }
    mUsedPaths.clear();



    for(int siblingIndex = 0; siblingIndex<family.size(); ++siblingIndex){
        int nbChildren = family[siblingIndex];
        if(nbChildren <= 0){
            continue;
        }
        // The big line between
        QPointF start = QPointF(centerX + siblingIndex*siblingDeltaX + parentToChildDeltaX/2,
                                centerY + parentToChildDeltaY*0.8);
        QPointF end = QPointF(centerX + siblingIndex*siblingDeltaX + parentToChildDeltaX/2,
                              centerY + parentToChildDeltaY + (nbChildren-0.6)*childDeltaY);
        MyGraphicsLineItem * vLine = getLine(this);
        QPropertyAnimation *animation = new QPropertyAnimation(vLine, "line");
        animation->setDuration(animationDuration);
        animation->setStartValue(QLineF(start, start));
        animation->setEndValue(QLineF(start, end));
        animation->setEasingCurve(QEasingCurve::InOutExpo);

        QPropertyAnimation *animationOpacity = new QPropertyAnimation(vLine, "opacity");
        animationOpacity->setDuration(animationDuration);
        animationOpacity->setStartValue(0.0);
        animationOpacity->setEndValue(1.0);
        animationOpacity->setEasingCurve(QEasingCurve::InExpo);


        if(animationGroup){
            animationGroup->addAnimation(animation);
            animationGroup->addAnimation(animationOpacity);
        }
        else{
            animation->start();
            animationOpacity->start();
        }
        for(int childIndex = 0; childIndex<nbChildren; ++childIndex){
            QPointF childEndLeft = QPointF(centerX + siblingIndex*siblingDeltaX + parentToChildDeltaX/2,
                                           centerY + parentToChildDeltaY + (childIndex + 0.4)*childDeltaY);
            MyGraphicsLineItem * hLine = getLine(this);
            QPropertyAnimation * childAnimation = new QPropertyAnimation(hLine, "line");
            childAnimation->setDuration(animationDuration);
            childAnimation->setStartValue(QLineF(start, start));
            childAnimation->setEndValue(QLineF(childEndLeft, childEndLeft + QPointF(parentToChildDeltaX/4, 0.0)));
            childAnimation->setEasingCurve(QEasingCurve::InOutExpo);

            QPropertyAnimation *childAnimationOpacity = new QPropertyAnimation(hLine, "opacity");
            childAnimationOpacity->setDuration(animationDuration);
            childAnimationOpacity->setStartValue(0.0);
            childAnimationOpacity->setEndValue(1.0);
            childAnimationOpacity->setEasingCurve(QEasingCurve::InExpo);

            if(animationGroup){
                animationGroup->addAnimation(childAnimation);
                animationGroup->addAnimation(childAnimationOpacity);
            }
            else{
                childAnimation->start();
                childAnimationOpacity->start();
            }
        }
    }

    MyGraphicsPathItem * path = getPath(this);
    QPainterPath painterPath;
    painterPath.addRect(80, 100, siblingDeltaX*0.9, centerY + parentToChildDeltaY*0.8 - 100);
    path->setPath(painterPath);
    path->setVisible(true);


    QPropertyAnimation * boxAnimation = new QPropertyAnimation(path, "opacity");
    boxAnimation->setDuration(animationDuration);
    boxAnimation->setStartValue(0.0);
    boxAnimation->setEndValue(1.0);
    boxAnimation->setEasingCurve(QEasingCurve::InExpo);
    if(animationGroup){
        animationGroup->addAnimation(boxAnimation);
    }
    else{
        boxAnimation->start();
    }

}

QVector<QVector<int>> MyTaskOrgGraphicsScene::getItemPlacement(int uniqueId)
{
    QVector<QVector<int>> answer;
    Unit * unit = mData->units()[uniqueId];
    Unit * parentUnit = mData->units().value(unit->parentId(), nullptr);


    if(mDisplayFullFamily){
        // Get the brothers in the right order
        QVector<int> siblings;
        if(mUnitSelectedFirst){
            siblings.push_back(uniqueId);
            siblings += mData->siblings(uniqueId);
        }
        else{
            if(parentUnit){
                siblings = parentUnit->children();
            }
            else{
                siblings.push_back(uniqueId);
            }
        }

        // Push the nephews
        for(int sibling : siblings){
            answer.push_back(QVector<int>());
            answer.back().push_back(sibling);
            answer.back() += mData->units()[sibling]->children();
        }
    }
    else{
        answer.push_back(QVector<int>());
        answer.back().push_back(uniqueId);
        answer.back() += mData->units()[uniqueId]->children();
    }

    return answer;
}

void MyTaskOrgGraphicsScene::setDisplayFullFamily(bool value)
{
    if(value != mDisplayFullFamily){
        mDisplayFullFamily = value;
        selectUnit(mSelectedUniqueId);
    }
}

void MyTaskOrgGraphicsScene::updateTextSize(qreal size)
{
    MyTaskOrgGraphicsScene::textSize = size;
    for(QGraphicsItem * item : mItems){
        for(QGraphicsItem * childItem : item->childItems()){
            QGraphicsTextItem * textItem = qgraphicsitem_cast<QGraphicsTextItem*>(childItem);
            if (textItem){
                textItem->setScale(size);
            }
        }
    }
}

void MyTaskOrgGraphicsScene::resizeToBoundingRect()
{
    setSceneRect(itemsBoundingRect());
}


///////////////////////////////////////////////////////////////////////
/// QGRAPHICS                                                       ///
///////////////////////////////////////////////////////////////////////
///
void MyTaskOrgGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mUnitPressed = -1;
    if(event->widget()){
        if(qobject_cast<QGraphicsView*>(event->widget()->parent())){
                        qDebug() << "Click";
            QGraphicsView * view = qobject_cast<QGraphicsView*>(event->widget()->parent());
            QGraphicsItem * item = itemAt(event->scenePos(), view->transform());
            if(dynamic_cast<MyUnitItem*>(item)){
                MyUnitItem * unitItem = dynamic_cast<MyUnitItem*>(item);
                                qDebug() << "Scene received a click on item " << unitItem->mUniqueId;
                mUnitPressed = unitItem->mUniqueId;
                mPosUnitPressed = event->scenePos();
                //                emit unitSelected(unitItem->mUniqueId);
            }
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void MyTaskOrgGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->widget()){
        if(qobject_cast<QGraphicsView*>(event->widget()->parent())){
                        qDebug() << "Click release";
            QGraphicsView * view = qobject_cast<QGraphicsView*>(event->widget()->parent());
            QGraphicsItem * item = itemAt(event->scenePos(), view->transform());
            if(dynamic_cast<MyUnitItem*>(item)){
                MyUnitItem * unitItem = dynamic_cast<MyUnitItem*>(item);
                                qDebug() << "Scene received a click release on item " << unitItem->mUniqueId;

                QPropertyAnimation *animation = new QPropertyAnimation(unitItem, "pos");
                animation->setDuration(animationDuration/2);
                animation->setStartValue(unitItem->scenePos());
                animation->setEndValue(unitItem->mFixedPos);
                animation->start();

                if (mUnitPressed == unitItem->mUniqueId &&
                        QLineF(mPosUnitPressed, event->scenePos()).length() < 50){
                    qDebug() << "Some interesting way to select";
                    emit unitSelected(unitItem->mUniqueId);
                }
            }
        }
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void MyTaskOrgGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug() << "dragEnterEvent";
    QGraphicsScene::dragEnterEvent(event);
}

void MyTaskOrgGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug() << "dragMoveEvent";
    QGraphicsScene::dragMoveEvent(event);
}

void MyTaskOrgGraphicsScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug() << "dragLeaveEvent";
    QGraphicsScene::dragLeaveEvent(event);
}

void MyTaskOrgGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug() << "dropEvent";
    QGraphicsScene::dropEvent(event);
}






HoverBoxFilter::HoverBoxFilter(QGraphicsScene *scene):
    mScene(scene)
{
    qDebug() << " Filter created";
}

bool HoverBoxFilter::eventFilter(QObject *object, QEvent *event)
{
    qDebug() << "dgdgdg";
    if (event->type() == QEvent::HoverEnter) {
        qDebug() << "Enter";
        return true;
    }
    if (event->type() == QEvent::HoverLeave) {
        qDebug() << "Leave";
        return true;
    }
    return false;
}

bool MyUnitItem::sceneEvent(QEvent *event)
{
//    qDebug() << "Event type " << event->type();
    switch(event->type()){
        default:
            (void) event;
    }
    if (event->type() == QEvent::GraphicsSceneHoverEnter) {
        //qDebug() << "Enter";
        return true;
    }
    if (event->type() == QEvent::GraphicsSceneHoverLeave) {
        //qDebug() << "Leave";
        return true;
    }
    if (event->type() == QEvent::GraphicsSceneMousePress ) {
        //qDebug() << "Mouse Press";
        return true;
    }
    if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        //qDebug() << "Mouse Release";
        return true;
    }
    if (event->type() == QEvent::GraphicsSceneMouseMove) {
        qDebug() << "Mouse Move";
        return true;
    }

    return false;
}

void MyUnitItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QGraphicsPixmapItem::paint(painter, option, widget);
    painter->restore();
}
