#include "mymapgraphicsscene.h"
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMimeData>
#include <QPropertyAnimation>
#include <QApplication>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QTimer>
#include <QGraphicsSimpleTextItem>
#include <QFont>

#include "commands.h"
#include "data.h"
#include "mainwindow.h"
#include "VectorEditor/vepolyline.h"
#include "VectorEditor/verectangle.h"
#include "VectorEditor/veellipse.h"
#include "VectorEditor/veselectionrect.h"

MyMapGraphicsScene::MyMapGraphicsScene(Data *data, bool isAdmin, QObject *parent):
    QGraphicsScene (parent),
    mData(data),
    mTop(0),
    mBottom(1000),
    mLeft(0),
    mRight(1000),
    //    mIdUserMoved(-1),
    mIsDisplayingAll(false),
    mDisplayLevel(1),
    //    mDrawingMode(false),
    mMapMode(UNIT_MODE)
{
    connect(this, &MyMapGraphicsScene::selectionChanged, this, &MyMapGraphicsScene::mySelectionChanged);

    // VE
    mCurrentDrawingAction = DefaultType;
    idOfDrawingInConstruction = -1;


    mMainWindow = dynamic_cast<MainWindow*>(parent);

    //    int intentionId = mData->getDrawingUniqueId();
    //    mData->createDrawing()


    setupHover();
}

void MyMapGraphicsScene::setupHover()
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    QPen pen;

    mHoverItemSpeed = new QGraphicsEllipseItem();
    brush.setColor(QColor(0, 0, 255, 64));
    mHoverItemSpeed->setBrush(brush);
    pen.setStyle(Qt::NoPen);
    mHoverItemSpeed->setPen(pen);

    addItem(mHoverItemSpeed);
    mHoverItemSpeed->hide();



    mHoverItemCautious = new QGraphicsEllipseItem();
    brush.setColor(QColor(255, 0, 0, 64));
    mHoverItemCautious->setBrush(brush);
    mHoverItemCautious->setPen(pen);

    addItem(mHoverItemCautious);
    mHoverItemCautious->hide();

    setHoverRadius(20);
}

void MyMapGraphicsScene::clear()
{
    mUnitItems.clear();
    mDrawings.clear();
    mOrders.clear();
    //    mIdUserMoved = -1;

    QGraphicsScene::clear(); // Delete all the QGraphicsItem

    setupHover(); // Because the hover items were cleared too!
}

void MyMapGraphicsScene::clearUnits()
{
    for(MapUnitItem * item : mUnitItems){
        removeItem(item);
        item->deleteLater();
    }
    mUnitItems.clear();

    unitHoverLeave();

    clearOrders();
}

void MyMapGraphicsScene::clearOrders()
{
    for(auto map : mOrders){
        for(QGraphicsItem * item : map){
            VEPolyline * line = qgraphicsitem_cast<VEPolyline*>(item);
            line->setPath(QPainterPath());
        }
    }
    mOrders.clear();

}

void MyMapGraphicsScene::loadMap(QDir dir)
{
    // Background
    //    QString file = dir.filePath("big-4x4.TIF");
    //        QString file = dir.filePath("2x2.tif");
    //    QString file = dir.filePath("4x4_png_UTM31.png");
    //        QString file = dir.filePath("8x8.tif");
    QString file = dir.filePath("8x8.png");
    //    QString file = dir.filePath("12x12.tif");
    //        QString file = dir.filePath("12x12.png");
    //        QString file = dir.filePath("16x16.png");

    qDebug() << "Map file " << file;
    //    addPixmap(QPixmap(file));

    if(mOldDir != dir){
        mBackgroundPixmap = QPixmap(file);
    }
    else{
        qDebug() << "No need to reload the map";
    }
    mOldDir = dir;
    QGraphicsPixmapItem * backgroundItem = new QGraphicsPixmapItem(mBackgroundPixmap);
    double factor = 1000.0 / mBackgroundPixmap.width();
    backgroundItem->setScale(factor);
    backgroundItem->setZValue(-1.0);
    //    backgroundItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    addItem(backgroundItem);


    setSceneRect(QRectF(-1000, -1000, 3000, 3000));


    // The metadata
    QString settingsFilename = dir.filePath("mapSettings.json");
    QFile settingsFile(settingsFilename);
    if (!settingsFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file : " << (settingsFilename);
    }
    else{
        QByteArray saveData = settingsFile.readAll();
        QJsonDocument jsonDoc(QJsonDocument::fromJson(saveData));
        const QJsonObject & jsonObject = jsonDoc.object();

        if(jsonObject.contains("top") && jsonObject["top"].isDouble()){
            mTop = jsonObject["top"].toDouble();
            qDebug() << "Top is " << mTop;
        }
        if(jsonObject.contains("bottom") && jsonObject["bottom"].isDouble()){
            mBottom = jsonObject["bottom"].toDouble();
            qDebug() << "bottom is " << mBottom;
        }

    }

}


void MyMapGraphicsScene::addUniqueId(int uniqueId)
{
    auto units = mData->units();
    if(!units.contains(uniqueId)){
        return;
    }

    // Grab the icon from the Unit and put it in a QGraphicsPixmapItem
    Unit * unit = mData->units()[uniqueId];
    addUniqueIdWithCustomIcon(uniqueId, unit->app6(), unit->displayedText(), static_cast<int>(unit->getLife()));

}

void MyMapGraphicsScene::addUniqueIdWithCustomIcon(int uniqueId, const QString &app6, const QString & displayedText, int life)
{
    Unit unit(uniqueId);
    unit.setApp6(app6);
    unit.setDisplayedText(displayedText);
    unit.setLife(life);

    //    qDebug() << "adding unit with life " << life;
    //    QIcon icon = unit.getIcon();
    QIcon icon = unit.getIconWithLife();

    QSize size = QSize(Unit::iconTotalWidth, Unit::iconTotalHeight);
    MapUnitItem * item = new MapUnitItem(uniqueId, icon.pixmap(size));
    item->mUniqueId = uniqueId;
    double scale = 0.4;
    item->setScale(scale);

    //    if(adminView()){
    //        item->setFlag(QGraphicsItem::ItemIsMovable);
    //    }
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    item->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    item->setAcceptHoverEvents(true);
    item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    //    item->setOffset(QPointF(-scale*Unit::iconWidth/2.0, -scale*Unit::iconTotalHeight/2.0));
    item->setOffset(QPointF(-Unit::iconWidth/2.0, -Unit::iconTotalHeight/2.0));
    item->setZValue(static_cast<int>(unit.size()));
    //    setUnitVisible(item, false);

    mUnitItems[uniqueId] = item;
    addItem(item);
    //    qDebug() << "Map added ID : " << uniqueId;

    connect(item, &MapUnitItem::unitMoved, this, &MyMapGraphicsScene::unitMoved);
    connect(item, &MapUnitItem::orderUpdatedByUser, mMainWindow, &MainWindow::orderUpdatedByUser);
    connect(item, &MapUnitItem::hoverEntered, this, &MyMapGraphicsScene::unitHoverEnter);
    connect(item, &MapUnitItem::hoverLeft, this, &MyMapGraphicsScene::unitHoverLeave);
    //    connect(item, &MapUnitItem::orderUpdated, mData, &Data::updateOrder);
}

void MyMapGraphicsScene::deleteUniqueId(int uniqueId)
{
    if(!mUnitItems.contains(uniqueId)){
        return;
    }
    removeItem(mUnitItems[uniqueId]);
    mUnitItems[uniqueId]->deleteLater();
    mUnitItems.remove(uniqueId);
}

void MyMapGraphicsScene::updateUnitContent(Unit unit)
{

    int uniqueId = unit.uniqueId();
    //    qDebug() << "Unit content update " << uniqueId << " SPESHUL ID " << mIdUserMoved;

    //    // TODO
    //    if(uniqueId == mIdUserMoved){
    //        //        qDebug() << "SKIP";
    //        return;
    //    }



    // TODO
    deleteUniqueId(uniqueId);
    addUniqueId(uniqueId);

    //    MapUnitItem * item = mUnitItems[uniqueId];


    //    if(!unit.isPosLegal()){
    //        setUnitVisible(item, false);
    //    }
    //    else{
    //        QPointF realPos = unit.pos();
    //        setUnitVisible(item, true);
    //        item->setPos(convertToScenePos(realPos));

    //        //        qDebug() << "Map updated position of " << unit.uniqueId() << " to : " << convertToScenePos(realPos);
    //    }
}

void MyMapGraphicsScene::updateUnitPosition(int uniqueId, QPointF pos)
{
    //    if(uniqueId == mIdUserMoved){
    //        return;
    //    }

    MapUnitItem * item = mUnitItems[uniqueId];
    //    item->setVisible(true);
    //    qDebug() << "Move " << uniqueId << " to " << pos;
    if(Unit::isPosLegal(pos)){
        item->setPos(convertToScenePos(pos));
        return;
    }

    item->hide();
}

void MyMapGraphicsScene::changeUnitParent(int parentId, int uniqueId)
{

}

void MyMapGraphicsScene::removeUnitParent(int uniqueId)
{

}

void MyMapGraphicsScene::selectUnit(int uniqueId)
{
    if(!mUnitItems.contains(uniqueId)){
        return;
    }
    const QSignalBlocker blocker(this); // No need to re-emit that uniqueId was selected
    MapUnitItem * item = mUnitItems[uniqueId];
    clearSelection();
    item->setSelected(true);
}



void MyMapGraphicsScene::debug()
{
    qDebug() << "Nb of items " << mUnitItems.size();
}

int MyMapGraphicsScene::getDrawingUniqueId(QGraphicsItem *item)
{
    switch (item->type()) {
    case VEEllipse::Type:{
        VEEllipse * ellipse = qgraphicsitem_cast<VEEllipse*>(item);
        return ellipse->uniqueId();
    }
    case VERectangle::Type:{
        VERectangle * rectangle = qgraphicsitem_cast<VERectangle*>(item);
        return rectangle->uniqueId();
    }
    case VEPolyline::Type:{
        VEPolyline * polyline = qgraphicsitem_cast<VEPolyline*>(item);
        return polyline->uniqueId();
    }
    default:
        return -1;
    }
}





QPointF MyMapGraphicsScene::convertToRealPos(QPointF scenePos)
{
    return scenePos;
}

QPointF MyMapGraphicsScene::convertToScenePos(QPointF realPos)
{
    return realPos;
}

QGraphicsItem *MyMapGraphicsScene::getUnit(int uniqueId)
{
    return mUnitItems.value(uniqueId, nullptr);
}

void MyMapGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    //    qDebug() << "DRAG ENTER";
    if (event->mimeData()->hasText()) {
        //        qDebug() << "DRAG HAS TEXT";
        event->setAccepted(true);
        //        dragOver = true;
        update();
    } else {
        event->setAccepted(false);
    }
}

void MyMapGraphicsScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    //    dragOver = false;
    update();
}

void MyMapGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(true);
    update();
}

void MyMapGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //    dragOver = false;
    qDebug() << "DROP";
    if (event->mimeData()->hasText()){
        qDebug() << "drop " << (event->mimeData()->text()) << " on pos " << event->scenePos();
        QPointF unitRealPos = convertToRealPos(event->scenePos());
        int uniqueId = event->mimeData()->text().toInt();
        //  If the ID is unknown (for example because you didn't drop a Unit), nothing happens
        emit unitDropped(uniqueId, unitRealPos);
    }
    update();
}










void MyMapGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
//    qDebug() << "Mouse press";

    if (event->button() & Qt::LeftButton) {
        mLeftMouseButtonPressed = true;
        setClickDownPosition(event->scenePos());
        //        if(QApplication::keyboardModifiers() & Qt::ShiftModifier){
        //            m_previousAction = m_currentAction;
        //            setCurrentAction(SelectionType);
        //        }
    }

    switch(mMapMode){
    case UNIT_MODE:
        mousePressEventUnitMode(event);
        break;
    case DRAWING_MODE:
        mousePressEventDrawingMode(event);
        break;
    }
}

void MyMapGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //    qDebug() << "Move";
    switch(mMapMode){
    case UNIT_MODE:
        mouseMoveEventUnitMode(event);
        break;
    case DRAWING_MODE:
        mouseMoveEventDrawingMode(event);
        break;
    }
}

void MyMapGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() & Qt::LeftButton)
    {
        mLeftMouseButtonPressed = false;


    }
    //    mIdUserMoved = -1;
    QGraphicsScene::mouseReleaseEvent(event);


    switch(mMapMode){
    case UNIT_MODE:
        mouseReleaseEventUnitMode(event);
        break;
    case DRAWING_MODE:
        mouseReleaseEventDrawingMode(event);
        break;
    }



    // UNIT MODE

}


void MyMapGraphicsScene::mousePressEventUnitMode(QGraphicsSceneMouseEvent *event)
{
    if(mMainWindow->adminView()){
        QGraphicsScene::mousePressEvent(event);
    }
    // Player view
    else{
        //        if (!mLeftMouseButtonPressed || (event->button() & Qt::RightButton) || (event->button() & Qt::MiddleButton)){
        //            QGraphicsScene::mousePressEvent(event);
        //            return;
        //        }

//        qDebug() << "Order phase 1 SCENE";
        QGraphicsScene::mousePressEvent(event);
    }
}

void MyMapGraphicsScene::mouseMoveEventUnitMode(QGraphicsSceneMouseEvent *event)
{
    if(mMainWindow->adminView()){
        QGraphicsScene::mouseMoveEvent(event);
    }
    // Player view
    else{
        if (!mLeftMouseButtonPressed){
            QGraphicsScene::mouseMoveEvent(event);
            return;
        }



        QGraphicsScene::mouseMoveEvent(event);
        //        qDebug() << "Order phase 2";
    }
}

void MyMapGraphicsScene::mouseReleaseEventUnitMode(QGraphicsSceneMouseEvent *event)
{
    if(mMainWindow->adminView()){
        QGraphicsScene::mouseReleaseEvent(event);
    }
    // Player view
    else{
        // Cleanup
//        qDebug() << "Order phase 3";
    }
}

void MyMapGraphicsScene::mousePressEventDrawingMode(QGraphicsSceneMouseEvent *event)
{
    if (!mLeftMouseButtonPressed || (event->button() & Qt::RightButton) || (event->button() & Qt::MiddleButton)){
        return;
    }
//    qDebug() << "Action";
    switch (mCurrentDrawingAction) {
    case LineType: {
        qDebug() << "Action draw line";
        deselectItems();

        Drawing drawing;
        drawing.setUniqueId(mData->getDrawingUniqueId());
        drawing.setType(Drawing::POLYLINE);
        QPainterPath path;
        path.moveTo(mClickDownPosition);
        drawing.setPainterPath(path);

        //        QGraphicsItem * polyline = createDrawing(drawing);
        idOfDrawingInConstruction = drawing.uniqueId();
        emit drawingCreatedByUser(drawing);

        //                VEPolyline *polyline = new VEPolyline(this);
        //                currentItem = polyline;
        //                addItem(currentItem);
        //                connect(polyline, &VEPolyline::drawingClicked, this, &MyMapGraphicsScene::drawingSelected);
        //                connect(polyline, &VEPolyline::drawingMoved, this, &MyMapGraphicsScene::moveDrawingSelection);
        //                QPainterPath path;
        //                path.moveTo(mClickDownPosition);
        //                polyline->setPath(path);
        //                emit drawingCreated(polyline);

        break;
    }
    case RectangleType: {
        deselectItems();

        Drawing drawing;
        drawing.setUniqueId(mData->getDrawingUniqueId());
        drawing.setType(Drawing::RECTANGLE);

        QGraphicsItem * rectangle = createDrawing(drawing);
        idOfDrawingInConstruction = drawing.uniqueId();
        emit drawingCreatedByUser(drawing);
        break;

        //        VERectangle *rectangle = new VERectangle(this);
        //        currentItem = rectangle;
        //        addItem(currentItem);
        //        connect(rectangle, &VERectangle::drawingClicked, this, &MyMapGraphicsScene::drawingSelected);
        //        connect(rectangle, &VERectangle::drawingMoved, this, &MyMapGraphicsScene::moveDrawingSelection);
        //        emit drawingCreatedByUser(rectangle);
        //        break;
    }
    case EllipseType: {

        deselectItems();

        Drawing drawing;
        drawing.setUniqueId(mData->getDrawingUniqueId());
        drawing.setType(Drawing::ELLIPSE);

        QGraphicsItem * ellipse = createDrawing(drawing);
        idOfDrawingInConstruction = drawing.uniqueId();
        emit drawingCreatedByUser(drawing);
        break;



        //        VEEllipse *ellipse = new VEEllipse();
        //        currentItem = ellipse;
        //        addItem(currentItem);
        //        connect(ellipse, &VEEllipse::drawingClicked, this, &MyMapGraphicsScene::drawingSelected);
        //        connect(ellipse, &VEEllipse::drawingMoved, this, &MyMapGraphicsScene::moveDrawingSelection);
        //        emit drawingCreatedByUser(ellipse, drawing);
        //        break;
    }
    case SelectionType: {

        //        deselectItems();
        //        VESelectionRect *selection = new VESelectionRect();
        //        currentItem = -1;
        //        addItem(currentItem);
        break;
    }
    default: { // DEFAULT_TYPE
        QGraphicsScene::mousePressEvent(event);
        break;
    }
    }

    QGraphicsScene::mousePressEvent(event);
}

void MyMapGraphicsScene::mouseMoveEventDrawingMode(QGraphicsSceneMouseEvent *event)
{
    switch (mCurrentDrawingAction) {
    case LineType: {
        if (mLeftMouseButtonPressed) {

            VEPolyline * polyline = qgraphicsitem_cast<VEPolyline *>(mDrawings[idOfDrawingInConstruction]);
            if(polyline){
                //                qDebug() << "BLI";
                QPainterPath path;
                path.moveTo(mClickDownPosition);
                path.lineTo(event->scenePos());
                qDebug() << "Line from " << mClickDownPosition << " to " << event->scenePos();
                polyline->setPath(path);
                Drawing drawing = polyline->toDrawing();
                //                drawing.setPainterPath(path);
                //                polyline->setPath(path);
                emit drawingUpdatedByUser(drawing);
            }
        }
        break;
    }
    case RectangleType: {
        if (mLeftMouseButtonPressed) {
            auto dx = event->scenePos().x() - mClickDownPosition.x();
            auto dy = event->scenePos().y() - mClickDownPosition.y();
            VERectangle * rectangle = qgraphicsitem_cast<VERectangle *>(mDrawings[idOfDrawingInConstruction]);
            rectangle->setRect((dx > 0) ? mClickDownPosition.x() : event->scenePos().x(),
                               (dy > 0) ? mClickDownPosition.y() : event->scenePos().y(),
                               qAbs(dx), qAbs(dy));
        }
        break;
    }
    case EllipseType: {
        if (mLeftMouseButtonPressed) {
            auto dx = event->scenePos().x() - mClickDownPosition.x();
            auto dy = event->scenePos().y() - mClickDownPosition.y();
            VEEllipse * ellipse = qgraphicsitem_cast<VEEllipse *>(mDrawings[idOfDrawingInConstruction]);
            ellipse->setRect((dx > 0) ? mClickDownPosition.x() : event->scenePos().x(),
                             (dy > 0) ? mClickDownPosition.y() : event->scenePos().y(),
                             qAbs(dx), qAbs(dy));
        }
        break;
    }
    case SelectionType: {
        //        if (m_leftMouseButtonPressed) {
        //            auto dx = event->scenePos().x() - mClickDownPosition.x();
        //            auto dy = event->scenePos().y() - mClickDownPosition.y();
        //            VESelectionRect * selection = qgraphicsitem_cast<VESelectionRect *>(mDrawings[currentItem]);
        //            selection->setRect((dx > 0) ? mClickDownPosition.x() : event->scenePos().x(),
        //                               (dy > 0) ? mClickDownPosition.y() : event->scenePos().y(),
        //                               qAbs(dx), qAbs(dy));
        //        }
        break;
    }
    default: {
        //        qDebug() << "Default move";
        QGraphicsScene::mouseMoveEvent(event);
        break;
    }
    }
    //    QGraphicsScene::mouseMoveEvent(event);
}

void MyMapGraphicsScene::mouseReleaseEventDrawingMode(QGraphicsSceneMouseEvent *event)
{
    switch (mCurrentDrawingAction) {
    case LineType:
        if (!mLeftMouseButtonPressed &&
                !(event->button() & Qt::RightButton) &&
                !(event->button() & Qt::MiddleButton)) {
            // Warn Data that a drawing has been created
            //            VEPolyline * current = qgraphicsitem_cast<VEPolyline*>(mDrawings[currentItem]);
            //            Drawing drawing = current->toDrawing();
            //            mData->createPolyline(drawing);

            idOfDrawingInConstruction = -1;
        }
        break;
    case RectangleType:
        if (!mLeftMouseButtonPressed &&
                !(event->button() & Qt::RightButton) &&
                !(event->button() & Qt::MiddleButton)) {
            // Warn Data that a drawing has been created
            QJsonObject object;


            idOfDrawingInConstruction = -1;
        }
        break;
    case EllipseType:{
        if (!mLeftMouseButtonPressed &&
                !(event->button() & Qt::RightButton) &&
                !(event->button() & Qt::MiddleButton)) {
            // Warn Data that a drawing has been created
            QJsonObject object;


            idOfDrawingInConstruction = -1;
        }
        break;
    }
    case SelectionType: {
        //        if (!m_leftMouseButtonPressed &&
        //                !(event->button() & Qt::RightButton) &&
        //                !(event->button() & Qt::MiddleButton)) {
        //            VESelectionRect * selection = qgraphicsitem_cast<VESelectionRect *>(mDrawings[currentItem]);
        //            if(!selection->collidingItems().isEmpty()){
        //                foreach (QGraphicsItem *item, selection->collidingItems()) {
        //                    item->setSelected(true);
        //                }
        //            }
        //            selection->deleteLater();
        //            if(selectedItems().length() == 1){
        ////                drawingSelected(selectedItems().at(0));
        //                int drawingUniqueId = getDrawingUniqueId(selectedItems().at(0));
        //                drawingSelected(*(mData->drawings()[drawingUniqueId]));
        //            }
        //            setCurrentAction(m_previousAction);
        //            currentItem = -1;
        //        }
        break;
    }
    default: {
        QGraphicsScene::mouseReleaseEvent(event);
        break;
    }
    }

    emit drawingFinishedByUser();
}


void MyMapGraphicsScene::keyPressEvent(QKeyEvent *event)
{
    if(!adminView()){
        return;
    }
    qDebug() << "key press " << event->key();
    switch (event->key()) {
    //    case Qt::Key_ :
    case Qt::Key_D:
    case Qt::Key_Backspace:
    case Qt::Key_Delete: {
        qDebug() << "Selection size : " << selectedItems().size();
        foreach (QGraphicsItem *item, selectedItems()) {
            switch(item->type()){
            case VEPolyline::Type:{
                VEPolyline * polyline = qgraphicsitem_cast<VEPolyline*>(item);
                int uniqueId = polyline->uniqueId();
                removeItem(item);
                delete item;
                mDrawings.remove(uniqueId);
                emit drawingDeletedByUser(uniqueId);
                break;
            }
            case VERectangle::Type:{
                VERectangle * rectangle = qgraphicsitem_cast<VERectangle*>(item);
                int uniqueId = rectangle->uniqueId();
                removeItem(item);
                delete item;
                mDrawings.remove(uniqueId);
                emit drawingDeletedByUser(uniqueId);
                break;
            }
            case VEEllipse::Type:{
                VEEllipse * ellipse = qgraphicsitem_cast<VEEllipse*>(item);
                int uniqueId = ellipse->uniqueId();
                removeItem(item);
                delete item;
                mDrawings.remove(uniqueId);
                emit drawingDeletedByUser(uniqueId);
                break;
            }
            case MapUnitItem::Type:{
                //                MapUnitItem * unit = qgraphicsitem_cast<MapUnitItem*>(item);
                //                MoveUnitCommand * command = new MoveUnitCommand(mDta, unit->uniqueId(), Unit::illegalPosition());
                //                mMainWindow->undoStack()->push(command);
                break;
            }


                break;
            default:
                break;
            }
        }
        deselectItems();
        break;
    }
    case Qt::Key_A: {
        if(QApplication::keyboardModifiers() & Qt::ControlModifier){
            foreach (QGraphicsItem *item, items()) {
                item->setSelected(true);
            }
            if(selectedItems().length() == 1) {
                int drawingUniqueId = getDrawingUniqueId(selectedItems().at(0));
                emit drawingSelected(*(mData->drawings()[drawingUniqueId]));
            }
        }
        break;
    }
    default:
        break;
    }
    QGraphicsScene::keyPressEvent(event);
}




//void MyMapGraphicsScene::mousePressCreateOrderEvent(QGraphicsSceneMouseEvent *event, int uniqueId)
//{
//    if (event->button() & Qt::LeftButton) {
//        m_leftMouseButtonPressed = true;
//        setClickDownPosition(event->scenePos());
//    }
//    if(!adminView()){
//        qDebug() << "CREATE INTENTION";

//        if (m_leftMouseButtonPressed && !(event->button() & Qt::RightButton) && !(event->button() & Qt::MiddleButton)) {
//            deselectItems();
//            VEPolyline *polyline = new VEPolyline(this);
//            polyline->setIsUnbreakable(true);
//            QPen pen;
//            pen.setWidth(5);
//            pen.setColor(Qt::green);
//            polyline->setPen(pen);
//            qDebug() << "WIDTH 5";
//            currentIntention = polyline;
//            addItem(currentIntention);
//            QPainterPath path;
//            path.moveTo(mClickDownPosition);
//            polyline->setPath(path);
//            //            emit signalNewSelectItem(polyline);
//        }


//        //        if (m_leftMouseButtonPressed && !(event->button() & Qt::RightButton) && !(event->button() & Qt::MiddleButton)) {
//        //            deselectItems();
//        //            QGraphicsPathItem *pathItem = new QGraphicsPathItem();
//        //            currentItem = pathItem;
//        //            QPen pen;
//        //            pen.setWidth(2);
//        //            pathItem->setPen(pen);
//        //            addItem(currentItem);
//        //            QPainterPath path;
//        //            path.moveTo(m_previousPosition);
//        //            pathItem->setPath(path);
//        //            if(mIntentions.contains(uniqueId)){
//        //                QGraphicsItem * item = mIntentions[uniqueId];
//        //                removeItem(item);
//        //                delete item;
//        //            }
//        //            mIntentions[uniqueId] = pathItem;
//        //        }
//        //        else{
//        //            QGraphicsScene::mousePressEvent(event);
//        //        }
//        //        return;
//    }
//}

//void MyMapGraphicsScene::mouseMoveCreateOrderEvent(QGraphicsSceneMouseEvent *event, int uniqueId)
//{
//    if(!adminView()){
//        //        qDebug() << "MOVE";
//        if (m_leftMouseButtonPressed) {

//            QPainterPath path;
//            path.moveTo(mClickDownPosition);
//            path.lineTo(event->scenePos());
//            currentIntention->setPath(path);

//        }
//        else{
//            QGraphicsScene::mouseMoveEvent(event);
//        }
//        return;
//    }
//}

//void MyMapGraphicsScene::mouseReleaseCreateOrderEvent(QGraphicsSceneMouseEvent *event, int uniqueId)
//{
//    if(!adminView()){

//        if (!m_leftMouseButtonPressed &&
//                !(event->button() & Qt::RightButton) &&
//                !(event->button() & Qt::MiddleButton)) {
//            qDebug() << "RELEASE";
//            //            mData->createIntention(currentIntention->toDrawing());
//            currentIntention = nullptr;
//        }
//        else{
//            QGraphicsScene::mouseMoveEvent(event);
//        }
//        return;
//    }
//}













//void MyMapGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
//{
//    //    qDebug() << "BLI";
//    mIdUserMoved = -1;
//    QGraphicsScene::mouseReleaseEvent(event);
//}

void MyMapGraphicsScene::mySelectionChanged()
{
    QList<QGraphicsItem * > selection = selectedItems();
    qDebug() << "Selection size : " << selection.size();
    if(selection.empty()){
        return;
    }
    for(QGraphicsItem * item : selection){
        MapUnitItem * mapUnitItem = qgraphicsitem_cast<MapUnitItem*>(item);
        if(mapUnitItem){
            emit unitSelected(mapUnitItem->mUniqueId);
            return;
        }

        //
    }
}

//void MyMapGraphicsScene::userMovedUnit(int uniqueId, QPointF pos)
//{

//    if(!mUnitItems.contains(uniqueId)){
//        //        mIdUserMoved = -1;
//        return;
//    }

//    mIdUserMoved = uniqueId;
//    emit unitMoved(uniqueId, convertToRealPos(pos));
//}

void MyMapGraphicsScene::displayAll()
{
    mIsDisplayingAll = true;
    displayPrivate(mData->unitsAtLevel(-1));
}

void MyMapGraphicsScene::displayMicro()
{
    if(mIsDisplayingAll){
        mIsDisplayingAll = false;
    }
    else{
        mDisplayLevel++;
        mDisplayLevel = qBound(0, mDisplayLevel, 4);
    }
    displayPrivate(mData->unitsAtLevel(mDisplayLevel));
}

void MyMapGraphicsScene::displayMacro()
{
    if(mIsDisplayingAll){
        mIsDisplayingAll = false;
    }
    else{
        mDisplayLevel--;
        mDisplayLevel = qBound(0, mDisplayLevel, 4);
    }
    displayPrivate(mData->unitsAtLevel(mDisplayLevel));
}

void MyMapGraphicsScene::displayPrivate(const QMap<int, bool> &unitsVisibleAtThisHierarchyLevel)
{
    // Start with the hierarchy level (company, battalion...)
    QMap<int, bool> finalVisibility = unitsVisibleAtThisHierarchyLevel;

    // Remove the units the player can't see
    QMap<QString, Player*> playerData = mData->getTruePlayers();
    if(mMainWindow->adminView()){
        // Admin has no filter
    }else{
        QString player = mMainWindow->playerView();
        if(!playerData.contains(player)){
            // Unregistered players have a full filter (they see nothing)
            for(bool & visibility : finalVisibility){
                visibility = false;
            }
        }
        else{
            // Players have a visibility
            Player * p = playerData[player];
            QMap<int, QStringList> visibleUnitInfo = p->visibleUnitInfo();
            for(auto it = finalVisibility.begin(); it != finalVisibility.end(); ++it){
                int id = it.key();
                if(!visibleUnitInfo.contains(id)){
                    finalVisibility[id] = false;
                }
            }


        }
    }

    // Add the detections
    if(mMainWindow->adminView()){
        // Admin has no filter
    }else{
        QString player = mMainWindow->playerView();
        Player * p = playerData[player];
        QSet<int> ownedUnits = p->ownedUnits();
        QMap<int, QStringList> visibleUnitInfo = p->visibleUnitInfo();
        for(auto it = visibleUnitInfo.cbegin(); it != visibleUnitInfo.cend(); ++it){
            // If you are visible, not owned and a leaf, add back
            int uniqueId = it.key();

            if(!ownedUnits.contains(uniqueId)){
                if(mData->isALeafOnMap(uniqueId)){
                    finalVisibility[uniqueId] = true;
                    qDebug() << "Unit " << mData->units()[uniqueId]->app6() << " is vis";
                }
                else{
                    finalVisibility[uniqueId] = false;
                    qDebug() << "Unit " << mData->units()[uniqueId]->app6() << " is invis";
                }
            }
        }
    }


    // You have your list of visible units, now display them and their orders (and hide the rest)
    qDebug() << finalVisibility;
    for(auto it = finalVisibility.constBegin(); it != finalVisibility.constEnd(); ++it){
        MapUnitItem * item = mUnitItems[it.key()];
        QMap<Order::ORDER_TYPE, QGraphicsItem*> orders = mOrders[it.key()];

        // Invisible
        if((!it.value())){
            {
                QPropertyAnimation *animation = new QPropertyAnimation(item, "opacity");
                animation->setDuration(500);
                animation->setStartValue(item->opacity());
                animation->setEndValue(0);
                animation->setEasingCurve(QEasingCurve::InCirc);
                connect(animation, &QPropertyAnimation::finished, [=](){
                    //                    qDebug() << "Item " << item->mUniqueId << "invis";
                    setUnitVisible(item, false);
                    /*sender()->deleteLater();*/});
                animation->start();
            }
            for(QGraphicsItem * order : orders)
            {
                VEPolyline * line = qgraphicsitem_cast<VEPolyline*>(order);
                if(!line){
                    continue;
                }
                QPropertyAnimation *animation = new QPropertyAnimation(line, "opacity");
                animation->setDuration(500);
                animation->setStartValue(line->opacity());
                animation->setEndValue(0);
                animation->setEasingCurve(QEasingCurve::InCirc);
                connect(animation, &QPropertyAnimation::finished, [line](){line->hide();});
                animation->start();
            }
        }

        // Visible
        else{
            {
                QPropertyAnimation *animation = new QPropertyAnimation(item, "opacity");
                setUnitVisible(item, true);
                animation->setDuration(500);
                animation->setStartValue(item->opacity());
                animation->setEndValue(1.0);
                animation->setEasingCurve(QEasingCurve::OutCirc);
                connect(animation, &QPropertyAnimation::finished, [=](){
                    //                qDebug() << "Item " << item->mUniqueId << " is visible now";
                    setUnitVisible(item, true);
                    /*sender()->deleteLater();*/});
                animation->start();
            }
            for(QGraphicsItem * order : orders)
            {
                VEPolyline * line = qgraphicsitem_cast<VEPolyline*>(order);
                if(!line){
                    continue;
                }
                QPropertyAnimation *animation = new QPropertyAnimation(line, "opacity");
                line->show();
                animation->setDuration(500);
                animation->setStartValue(line->opacity());
                animation->setEndValue(1.0);
                animation->setEasingCurve(QEasingCurve::OutCirc);
                connect(animation, &QPropertyAnimation::finished, [line](){line->show();});
                animation->start();
            }
        }

        //        mUnitItems[it.key()]->setVisible(it.value());
    }


    // HOVER STUFF
    QTimer * timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &MyMapGraphicsScene::unitHoverLeave);
    connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
    timer->start(600);


    //    for(auto it = mOrders.constBegin(); it != mOrders.constEnd(); ++it){
    //        int uniqueId = it.key();
    //        QGraphicsItem * item = it.value();
    //        if(visibleUnits.contains(uniqueId)){
    //            item->setVisible(visibleUnits[uniqueId]);
    //        }
    //    }
}



int MyMapGraphicsScene::currentAction() const
{
    return mCurrentDrawingAction;
}

QPointF MyMapGraphicsScene::clickDownPosition() const
{
    return mClickDownPosition;
}

void MyMapGraphicsScene::setCurrentAction(const int type)
{
    mCurrentDrawingAction = type;
    emit currentActionChanged(mCurrentDrawingAction);
}

void MyMapGraphicsScene::setClickDownPosition(const QPointF previousPosition)
{
    if (mClickDownPosition == previousPosition)
        return;

    mClickDownPosition = previousPosition;
    emit clickDownPositionChanged();
}

void MyMapGraphicsScene::deselectItems()
{
    foreach (QGraphicsItem *item, selectedItems()) {
        item->setSelected(false);
    }
    selectedItems().clear();
}

qreal MyMapGraphicsScene::getGraphicsViewScale()
{
    QGraphicsView * graphicsView = mMainWindow->getMapGraphicsView();
    return graphicsView->transform().m11();
}

void MyMapGraphicsScene::moveDrawingSelection(QGraphicsItem *signalOwner, qreal dx, qreal dy)
{
    foreach (QGraphicsItem *item, selectedItems()) {
        if(item != signalOwner) item->moveBy(dx,dy);
    }
}

//QJsonObject MyMapGraphicsScene::getControlMeasures() const
//{
//    QJsonObject answer;
//    QJsonArray ellipseArray;
//    QJsonArray rectangleArray;
//    QJsonArray polylineArray;
//    for (QGraphicsItem * item : items()){
//        switch(item->type()){
//        case VEEllipse::Type:
//        {
//            QJsonObject unitObject;
//            VEEllipse * ellipse = qgraphicsitem_cast<VEEllipse*>(item);
//            ellipse->write(unitObject);
//            ellipseArray.append(unitObject);
//            break;
//        }
//        case VERectangle::Type:
//        {
//            QJsonObject unitObject;
//            VERectangle * rectangle = qgraphicsitem_cast<VERectangle*>(item);
//            rectangle->write(unitObject);
//            rectangleArray.append(unitObject);
//            break;
//        }
//        case VEPolyline::Type:
//        {
//            QJsonObject unitObject;
//            VEPolyline * polyline = qgraphicsitem_cast<VEPolyline*>(item);
//            polyline->write(unitObject);
//            polylineArray.append(unitObject);
//            break;
//        }
//        default:
//            break;
//        }
//    }

//    answer["ellipse"] = ellipseArray;
//    answer["rectangle"] = rectangleArray;
//    answer["polyline"] = polylineArray;

//    return answer;
//}

//void MyMapGraphicsScene::setControlMeasures(QJsonObject &json)
//{
//    for (QGraphicsItem * item : items()){
//        switch(item->type()){
//        case VEEllipse::Type:
//        case VERectangle::Type:
//        case VEPolyline::Type:{
//            removeItem(item);
//            delete item;
//            break;
//        }
//        default:
//            break;
//        }
//    }

//    QJsonArray ellipses = json["ellipse"].toArray();
//    QJsonArray rectangles = json["rectangle"].toArray();
//    QJsonArray polylines = json["polyline"].toArray();

//    for(QJsonValue item : ellipses){
//        QJsonObject obj = item.toObject();
//        VEEllipse * ellipse = new VEEllipse(this);
//        QJsonArray rectArray = obj["rect"].toArray();
//        ellipse->setRect(rectArray[0].toDouble(),
//                rectArray[1].toDouble(),
//                rectArray[2].toDouble(),
//                rectArray[3].toDouble());
//        ellipse->setRotation(obj["rotation"].toDouble());
//        QPen pen;
//        pen.setWidth(obj["borderWidth"].toDouble());
//        pen.setColor(QColor(obj["borderColor"].toString()));
//        ellipse->setPen(pen);
//        QBrush brush;
//        brush.setColor(QColor(obj["interiorColor"].toString()));
//        qDebug() << "COLOR " << brush.color();
//        ellipse->setBrush(brush);

//        connect(ellipse, &VEEllipse::clicked, this, &MyMapGraphicsScene::signalSelectItem);
//        connect(ellipse, &VEEllipse::signalMove, this, &MyMapGraphicsScene::slotMove);
//        addItem(ellipse);
//    }


//    for(QJsonValue item : rectangles){
//        QJsonObject obj = item.toObject();
//        VERectangle * rectangle = new VERectangle(this);
//        QJsonArray rectArray = obj["rect"].toArray();
//        rectangle->setRect(rectArray[0].toDouble(),
//                rectArray[1].toDouble(),
//                rectArray[2].toDouble(),
//                rectArray[3].toDouble());
//        rectangle->setRotation(obj["rotation"].toDouble());
//        QPen pen;
//        pen.setWidth(obj["borderWidth"].toDouble());
//        pen.setColor(QColor(obj["borderColor"].toString()));
//        rectangle->setPen(pen);
//        QBrush brush;
//        brush.setColor(QColor(obj["interiorColor"].toString()));
//        qDebug() << "COLOR " << brush.color();
//        rectangle->setBrush(brush);
//        connect(rectangle, &VERectangle::clicked, this, &MyMapGraphicsScene::signalSelectItem);
//        connect(rectangle, &VERectangle::signalMove, this, &MyMapGraphicsScene::slotMove);
//        addItem(rectangle);
//    }

//    for(QJsonValue item : polylines){
//        QJsonObject obj = item.toObject();
//        VEPolyline * polyline = new VEPolyline(this);
//        QPen pen;
//        pen.setWidth(obj["width"].toDouble());
//        pen.setColor(QColor(obj["color"].toString()));
//        polyline->setPen(pen);
//        QPainterPath painterPath;
//        QJsonArray path = obj["path"].toArray();
//        painterPath.moveTo(path[0].toArray()[0].toDouble(), path[0].toArray()[1].toDouble());
//        for(int i = 1; i< path.size(); ++i){
//            QJsonArray point = path[i].toArray();
//            painterPath.lineTo(point[0].toDouble(), point[1].toDouble());
//        }
//        polyline->setPath(painterPath);

//        connect(polyline, &VEPolyline::drawingClicked, this, &MyMapGraphicsScene::signalSelectItem);
//        connect(polyline, &VEPolyline::drawingMoved, this, &MyMapGraphicsScene::slotMove);
//        addItem(polyline);
//    }
//}

//QJsonObject MyMapGraphicsScene::getIntentions() const
//{
//    QJsonObject answer;
//    QJsonArray array;
//    for(auto it = mIntentions.begin(); it != mIntentions.end(); ++it){
//        QGraphicsPathItem * pathItem = it.value();

//        QJsonObject intention;
//        intention["uniqueId"] = it.key();

//        QPainterPath p = pathItem->path();
//        QJsonArray jsonPath;
//        for(int i=0; i< p.elementCount() ; ++i ){
//            QPainterPath::Element e = p.elementAt(i);
//            QJsonArray jsonElement;
//            jsonElement.append(e.x);
//            jsonElement.append(e.y);
//            jsonPath.append(jsonElement);
//        }
//        intention["line"] = jsonPath;
//        array.append(intention);
//    }

//    answer["movement"] = array;
//    return answer;
//}

void MyMapGraphicsScene::setUnitVisible(MapUnitItem *item, bool b)
{
    item->setVisible(b);
    return;
    //    //    item->setVisible(false);
    //    // If you are admin, everything is visible
    //    if(isAdmin()){
    //        item->setVisible(b);
    //        return;
    //    }

    //    // You can only see what you own in this version
    //    QString owner = mData->units()[item->mUniqueId]->owner();

    //    if(owner != mSessionName){
    //        item->setVisible(false);
    //        return;
    //    }


    //    item->setVisible(b);
}

void MyMapGraphicsScene::clearUnitVisibility()
{
    for(MapUnitItem * item : mUnitItems){
        item->hide();
    }
}

// TODO HUH?
void MyMapGraphicsScene::setUnitVisibility(int uniqueId, const QString &app6, const QString & displayedText)
{
    MapUnitItem * item = mUnitItems[uniqueId];
    QPointF oldPos = item->scenePos();

    deleteUniqueId(uniqueId);

    addUniqueIdWithCustomIcon(uniqueId, app6, displayedText);
    mUnitItems[uniqueId]->setVisible(true);
    mUnitItems[uniqueId]->setPos(oldPos);
}

void MyMapGraphicsScene::setUnitVisibilityLife(int uniqueId, const QString &app6, const QString &displayedText, int life)
{
    MapUnitItem * item = mUnitItems[uniqueId];
    QPointF oldPos = item->scenePos();

    deleteUniqueId(uniqueId);

    addUniqueIdWithCustomIcon(uniqueId, app6, displayedText, life);
    mUnitItems[uniqueId]->setVisible(true);
    mUnitItems[uniqueId]->setPos(oldPos);
}

void MyMapGraphicsScene::setAdminUnitVisibility()
{
    for(MapUnitItem * item : mUnitItems){
        item->setVisible(true);
    }
}

void MyMapGraphicsScene::updateOrder(Order order)
{
    qDebug() << "Update the order";
    order.debug();
    int originUniqueId = order.originUniqueId();
    if(!mOrders.contains(originUniqueId)){
        mOrders[originUniqueId] = QMap<Order::ORDER_TYPE, QGraphicsItem*>();
    }

    Order::ORDER_TYPE orderType = order.orderType();
    if(!(mOrders[originUniqueId].contains(orderType))){
        VEPolyline * line = new VEPolyline(true);
        addItem(line);
        mOrders[originUniqueId][orderType] = line;

        switch (orderType) {
        case Order::MOVEMENT:{
            QPen pen;
            pen.setColor(Qt::black);
            pen.setWidth(1);
            pen.setStyle(Qt::DashDotLine);
            line->setPen(pen);
            break;
        }
        case Order::ATTACK:{
            QPen pen;
            pen.setColor(Qt::magenta);
            pen.setWidth(1);
            line->setPen(pen);
            break;
        }
        default:
            break;
        }
    }

    // Adjust the line
    VEPolyline * line = qgraphicsitem_cast<VEPolyline*>(mOrders[originUniqueId][orderType]);
    if(line){
        switch (orderType) {
        case Order::MOVEMENT:{
            QPointF movementTarget = order.destinationPosition();
            QPainterPath path;
            path.moveTo(movementTarget);
            path.lineTo(mData->getUnitPosition(originUniqueId));    // UX order
            if(Unit::isPosLegal(movementTarget) && Unit::isPosLegal(mData->getUnitPosition(originUniqueId))){
                qDebug() << "Update move order from " << mData->getUnitPosition(originUniqueId) << " to " << movementTarget;
                line->setPath(path);
            }
            else{
                line->setPath(QPainterPath());
            }
            break;
        }
        case Order::ATTACK:{
            QPainterPath path;
            path.moveTo(mData->getUnitPosition(originUniqueId));
            int targetId = order.destinationUniqueId();
            // A target
            if(targetId>=0){
                path.lineTo(mData->getUnitPosition(targetId));
                if(Unit::isPosLegal(mData->getUnitPosition(targetId))){
                    line->setPath(path);
                }
                else{
                    line->setPath(QPainterPath());
                }

                QPen p = line->pen();
                p.setColor(Qt::red);
                line->setPen(p);

            }
            // A point on the ground
            else{
                QPointF attackTarget = order.destinationPosition();
                path.lineTo(attackTarget);
                if(Unit::isPosLegal(attackTarget) && Unit::isPosLegal(mData->getUnitPosition(originUniqueId))){
                    QPen p = line->pen();
                    p.setColor(Qt::magenta);
                    line->setPen(p);
                    line->setPath(path);

                    qDebug() << "Attack from " << mData->getUnitPosition(originUniqueId) << " to " << attackTarget;
                }
                else{
                    line->setPath(QPainterPath());
                }
            }
            break;
        }
        default:
            // Nothing
            break;
        }
    }
    else{
        qWarning() << "Suprise with order";
        order.debug();
    }

    // Don't show in admin view
    if(adminView()){
        line->hide();
        qDebug() << "Don't display order update because we are in admin view";
        return;
    }

    // Don't show the order if the current player doesn't own it
    const QString & currentPlayerView = mMainWindow->playerView();
    QSet<int> currentOwnedUnits = mData->getPlayerOwnedUnits(currentPlayerView);
    if(!currentOwnedUnits.contains(originUniqueId)){
        qDebug() << "Don't display order update because " << currentPlayerView << " does not control " << originUniqueId;
        qDebug() << "This player controls " << currentOwnedUnits;
        line->hide();
        return;
    }

    if(!mUnitItems[originUniqueId]->isVisible()){
        line->hide();
        qDebug() << "Don't display order update because the origin is hidden";
        return;
    }

    line->show();
}

void MyMapGraphicsScene::showOrders(int unitUniqueId)
{
    if(!mOrders.contains(unitUniqueId)){
        return;
    }

    for(QGraphicsItem * item : mOrders[unitUniqueId]){
        item->setVisible(true);
    }
}

void MyMapGraphicsScene::hideAllOrders()
{
    for(auto map : mOrders){
        for(QGraphicsItem * item : map){
            item->setVisible(false);
        }
    }
}

QTransform MyMapGraphicsScene::getTransform()
{
    QGraphicsView * view = mMainWindow->getMapGraphicsView();
    return view->transform();
}

void MyMapGraphicsScene::setHoverPosition(QPointF pos)
{
    mHoverItemSpeed->setPos(pos);
    mHoverItemCautious->setPos(pos);
}

void MyMapGraphicsScene::setHoverRadius(qreal r)
{
    mHoverItemSpeed->setRect(-r, -r, 2*r, 2*r);
    mHoverItemCautious->setRect(-r/4, -r/4, 2*r/4, 2*r/4);
}

void MyMapGraphicsScene::unitHoverEnter()
{
    MapUnitItem* item = qobject_cast<MapUnitItem*>(QObject::sender());

    int unitId = item->uniqueId();
    if(adminView()){
        return;
    }
    QString playerName = mMainWindow->playerView();

    QMap<QString, QSet<int> > owners = mData->getPlayersOwnedUnits();
    if(!owners.contains(playerName)){
        return;
    }
    QSet<int> ownedUnits = owners[playerName];
    if(!ownedUnits.contains(unitId)){
        return;
    }

    mHoverItemSpeed->show();
    mHoverItemCautious->show();
    setHoverPosition(item->pos());
}

void MyMapGraphicsScene::unitHoverLeave()
{
    mHoverItemSpeed->hide();
    mHoverItemCautious->hide();
}

QGraphicsItem* MyMapGraphicsScene::createDrawing(Drawing drawing)
{

    drawing.debug();
    //    QJsonObject json;
    //    drawing.write(json);

    int uniqueId = drawing.uniqueId();

    // Delete the drawing if it's already in memory
    deleteDrawing(uniqueId);

    qDebug() << "CREATE DRAWING with ID " << uniqueId;
    QGraphicsItem * d;
    switch(drawing.type()){
    case Drawing::POLYLINE:{
        VEPolyline * polyline = new VEPolyline(this);
        connect(polyline, &VEPolyline::drawingClicked, [this](int uniqueId){emit drawingSelected(*(mData->drawings()[uniqueId]));});
        connect(polyline, &VEPolyline::drawingMoved, this, &MyMapGraphicsScene::moveDrawingSelection);
        connect(polyline, &VEPolyline::drawingUpdated, this, &MyMapGraphicsScene::drawingUpdatedByUser);
        polyline->fromDrawing(drawing);
        mDrawings[uniqueId] = polyline;
        d = polyline;
        break;
    }
    case Drawing::RECTANGLE:{
        VERectangle * rectangle = new VERectangle(this);
        //        connect(rectangle, &VERectangle::drawingClicked, this, &MyMapGraphicsScene::drawingSelected);
        connect(rectangle, &VERectangle::drawingMoved, this, &MyMapGraphicsScene::moveDrawingSelection);
        rectangle->fromDrawing(drawing);
        mDrawings[uniqueId] = rectangle;
        d = rectangle;
        break;
    }
    case Drawing::ELLIPSE:{
        VEEllipse * ellipse = new VEEllipse(this);
        //        connect(ellipse, &VEEllipse::drawingClicked, this, &MyMapGraphicsScene::drawingSelected);
        connect(ellipse, &VEEllipse::drawingMoved, this, &MyMapGraphicsScene::moveDrawingSelection);
        ellipse->fromDrawing(drawing);
        mDrawings[uniqueId] = ellipse;
        d = ellipse;
        break;
    }
    default:
        return nullptr;
    }
    addItem(d);
    d->setVisible(true);

    //    qDebug() << "NB DRAWINGS IN MEMORY " << mDrawings.size();
    if(mMapMode == DRAWING_MODE){
        d->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }

    return d;
}

void MyMapGraphicsScene::deleteDrawing(int uniqueId)
{
    if(mDrawings.contains(uniqueId)){
        QGraphicsItem * item = mDrawings[uniqueId];
        if(item){
            removeItem(mDrawings[uniqueId]); // remove from scene
            delete mDrawings[uniqueId];
        }
        mDrawings.remove(uniqueId);
    }
}

void MyMapGraphicsScene::updateDrawing(Drawing drawing)
{
    int uniqueId = drawing.uniqueId();
    if(!mDrawings.contains(uniqueId)){
        return;
    }

    if(drawing.uniqueId() == idOfDrawingInConstruction){
        qDebug() << "Skip";
        return;
    }

    QGraphicsItem * item = mDrawings[uniqueId];
    switch(item->type()){
    case VEPolyline::Type:{
        VEPolyline * polyline = qgraphicsitem_cast<VEPolyline*>(item);
        polyline->fromDrawing(drawing);
        //        qDebug() << "Update drawing to";
        //        drawing.debug();
        break;
    }
    case VERectangle::Type:{
        VERectangle * rectangle = qgraphicsitem_cast<VERectangle*>(item);
        rectangle->fromDrawing(drawing);
        break;
    }
    case VEEllipse::Type:{
        VEEllipse * ellipse = qgraphicsitem_cast<VEEllipse*>(item);
        ellipse->fromDrawing(drawing);
        break;
    }
    default:
        break;
    }

    update();
}

void MyMapGraphicsScene::setUnitMode()
{
    //    mDrawingMode = false;
    mMapMode = UNIT_MODE;

    for(QGraphicsItem * item : items()){
        switch(item->type()){

        case VEPolyline::Type:
        case VEEllipse::Type:
        case VERectangle::Type:
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            break;
        case MapUnitItem::Type:
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
            break;
        case QGraphicsPixmapItem::Type:
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            break;
        default:
            break;
        }
    }
}

void MyMapGraphicsScene::setDrawingMode()
{
    //    mDrawingMode = true;
    mMapMode = DRAWING_MODE;

    for(QGraphicsItem * item : items()){
        switch(item->type()){

        case VEPolyline::Type:
        case VEEllipse::Type:
        case VERectangle::Type:
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
            break;
        case MapUnitItem::Type:
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            break;
        default:
            break;
        }
    }
}

//bool MyMapGraphicsScene::drawingMode() const
//{
//    return mDrawingMode;
//}

MyMapGraphicsScene::MAP_MODE MyMapGraphicsScene::currentMode() const
{
    return mMapMode;
}

bool MyMapGraphicsScene::isAdmin() const
{
    return mMainWindow->isAdmin();
}

bool MyMapGraphicsScene::isObserver() const
{
    return mMainWindow->isObserver();
}

bool MyMapGraphicsScene::adminView() const
{
    return mMainWindow->adminView();
}

bool MyMapGraphicsScene::playerView() const
{
    return !(mMainWindow->adminView());
}







///////////////////////////////////////////////////////////////////////
/// PUBLIC  DATA STRUCTURES                                         ///
///////////////////////////////////////////////////////////////////////


MapUnitItem::MapUnitItem(int id, const QPixmap &pixmap, QGraphicsItem *parent):
    QGraphicsPixmapItem (pixmap, parent),
    mUniqueId(id){

    mDistanceItem = new QGraphicsSimpleTextItem(this);

    mDistanceItem->setFont(QFont("Times", 36, QFont::Bold));
    setAcceptHoverEvents(true);
}


void MapUnitItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QGraphicsPixmapItem::paint(painter, option, widget);
    painter->restore();
}


void MapUnitItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    emit hoverEntered();

    //    qDebug() << "Hover enter";
}

void MapUnitItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    emit hoverLeft();

    //    qDebug() << "Hover leave";
}

void MapUnitItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{


    // Put on top
    mOldZValue = zValue();
    setZValue(1000);

    mMouseButton = event->button();

    MyMapGraphicsScene * s = dynamic_cast<MyMapGraphicsScene*>(scene());

    // Don't interact when in drawing mode
    if(s->currentMode() == MyMapGraphicsScene::DRAWING_MODE){
        return;
    }

    if(s->isObserver()){
        QGraphicsPixmapItem::mousePressEvent(event);
        return;
    }


    qDebug() << "Mouse press with buttons " << event->buttons();
    //    qDebug() << "Press on " << event->buttonDownScenePos(Qt::LeftButton);
    mDeltaClick = event->scenePos() - scenePos();



    // In admin view, then do the standard thing, which is drag and drop
    if(s->adminView()){
        qDebug() << "Default event handler";
        QGraphicsPixmapItem::mousePressEvent(event);
        return;
    }

    // Player view
    // Only left click
    if ((event->button() & Qt::LeftButton) &&
            !(event->button() & Qt::RightButton) &&
            !(event->button() & Qt::MiddleButton))
    {
        Order order(mUniqueId, Order::MOVEMENT);
        order.setIsEnabled(false);                  // If you just clicked, you killed the previous order
        emit orderUpdatedByUser(order);
        event->accept();
        return;
    }

    // Only right click
    if ((event->button() & Qt::RightButton) &&
            !(event->button() & Qt::LeftButton) &&
            !(event->button() & Qt::MiddleButton))
    {
        qDebug() << "ATTACK";
        Order order(mUniqueId, Order::ATTACK);
        order.setIsEnabled(false);                  // If you just clicked, you killed the previous order
        emit orderUpdatedByUser(order);
        event->accept();
        return;
    }



    //                s->mousePressCreateOrderEvent(event, mUniqueId);
    //            return;
    QGraphicsPixmapItem::mousePressEvent(event);
    return;
}


void MapUnitItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //    qDebug() << "Buttons " << event->buttons();
    //    qDebug() << "Move BLA " << event->buttonDownScenePos(Qt::LeftButton);


    MyMapGraphicsScene * s = dynamic_cast<MyMapGraphicsScene*>(scene());
    if(!s){
        // Seriously, is that even possible?
        QGraphicsPixmapItem::mouseMoveEvent(event);
        return;
    }

    // Don't interact when in drawing mode
    if(s->currentMode()== MyMapGraphicsScene::DRAWING_MODE){
        return;
    }


    if(s->isObserver()){
        return;
    }

    QPointF eventScenePos = event->scenePos();

    // In admin view, then do the standard thing, which is drag and drop
    if(s->adminView()){
        //            qDebug() << "Item change pos to " << (pos-mDeltaClick);
        //            s->userMovedUnit(mUniqueId, pos-mDeltaClick);

        // Only left click

        //        if ((event->button() & Qt::LeftButton) &&
        //                !(event->button() & Qt::RightButton) &&
        //                !(event->button() & Qt::MiddleButton))
        if(event->buttons() == Qt::LeftButton)
        {
            emit unitMoved(mUniqueId, eventScenePos-mDeltaClick);
            return;
        }
        else{
            QGraphicsPixmapItem::mouseMoveEvent(event);
            return;
        }
    }
    // In player view, draw orders
    else{
        QPointF textPos;
        QPointF delta = eventScenePos - scenePos();
        QVector2D deltaVector(delta);
        QVector2D unitVector = deltaVector.normalized();
        QPointF unit = unitVector.toPointF();

        if(event->buttons() == Qt::LeftButton)
        {
            Order order(mUniqueId, Order::MOVEMENT);
            order.setIsEnabled(true);
            order.setDestinationPosition(eventScenePos);

            //            mDistanceItem->setDefaultTextColor(Qt::black);
            mDistanceItem->setPen(QPen(Qt::white));
            mDistanceItem->setBrush(QBrush(Qt::black));
            mDistanceItem->setText(QString::number((deltaVector.length()/10.0), 'g', 4) + QString("km"));
            QPointF localPos = mapFromScene(scenePos() + (deltaVector.length()) * (s->getGraphicsViewScale()) * 1.2 * unit);
            mDistanceItem->setPos(localPos);
            mDistanceItem->show();

            emit orderUpdatedByUser(order);
            return;
        }

        // Only right click
        if(event->buttons() == Qt::RightButton)
            //        if ((event->button() & Qt::RightButton) &&
            //                !(event->button() & Qt::LeftButton) &&
            //                !(event->button() & Qt::MiddleButton))
        {
            qDebug() << "ATTACK2";
            Order order(mUniqueId, Order::ATTACK);
            order.setIsEnabled(true);
            order.setDestinationPosition(eventScenePos);

            mDistanceItem->setText(QString::number((deltaVector.length()/10.0), 'g', 4) + QString("km"));
            QPointF localPos = mapFromScene(scenePos() + (deltaVector.length()) * (s->getGraphicsViewScale()) * 1.2 * unit);
            mDistanceItem->setPos(localPos);
            mDistanceItem->show();

            // By default, no targets for the order
            order.setDestinationUniqueId(-1);
            mDistanceItem->setPen(QPen(Qt::black));
            mDistanceItem->setBrush(QBrush(Qt::magenta));
            //            mDistanceItem->setDefaultTextColor(Qt::magenta);
            // But if we find one, we replace the order
            QList<QGraphicsItem *> targets = s->items(eventScenePos, Qt::IntersectsItemShape, Qt::DescendingOrder, s->getTransform());
            qDebug() << "Hover targets " << targets;
            for(QGraphicsItem * target : targets){
                MapUnitItem * targetUnit = qgraphicsitem_cast<MapUnitItem * >(target);
                if(targetUnit){
                    order.setDestinationUniqueId(targetUnit->uniqueId());
                    mDistanceItem->setBrush(QBrush(Qt::red));
                    QVector2D deltaUnits(targetUnit->scenePos() - scenePos());
                    mDistanceItem->setText(QString::number((deltaUnits.length()/10.0), 'g', 4) + QString("km"));

                    //                    mDistanceItem->setDefaultTextColor(Qt::red);
                }
            }

            //            QGraphicsItem * target = s->itemAt(eventScenePos, s->getTransform());
            //            qDebug() << "Hover target " << target;

            //            MapUnitItem * targetUnit = qgraphicsitem_cast<MapUnitItem * >(target);
            //            if(targetUnit){
            //                order.setDestinationUniqueId(targetUnit->uniqueId());
            //                mDistanceItem->setDefaultTextColor(Qt::red);
            //            }
            //            else{
            //                order.setDestinationUniqueId(-1);
            //                mDistanceItem->setDefaultTextColor(Qt::magenta);
            //            }
            emit orderUpdatedByUser(order);
            return;
        }
    }


    QGraphicsPixmapItem::mouseMoveEvent(event);
}

void MapUnitItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setZValue(mOldZValue);

    //    qDebug() << "Release";
    mDeltaClick = QPointF();
    mDistanceItem->setText("");
    mDistanceItem->hide();

    MyMapGraphicsScene * s = dynamic_cast<MyMapGraphicsScene*>(scene());
    if(!s){
        // Seriously, is that even possible?
        QGraphicsPixmapItem::mouseReleaseEvent(event);
        return;
    }

    // Don't interact when in drawing mode
    if(s->currentMode()== MyMapGraphicsScene::DRAWING_MODE){
        return;
    }


    if(s->isObserver()){
        return;
    }

    // In player view, don't create orders that would just be on yourself
    if(s->playerView()){
        QList<QGraphicsItem *> targets = s->items(event->scenePos(), Qt::IntersectsItemShape, Qt::DescendingOrder, s->getTransform());
        qDebug() << "Hover targets " << targets;
        for(QGraphicsItem * target : targets){
            MapUnitItem * targetUnit = qgraphicsitem_cast<MapUnitItem * >(target);
            if(targetUnit){
                if(targetUnit == this){
                    if(mMouseButton == Qt::LeftButton){
                        Order order(mUniqueId, Order::MOVEMENT);
                        emit orderUpdatedByUser(order);
                    }
                    if(mMouseButton == Qt::RightButton){
                        Order order(mUniqueId, Order::ATTACK);
                        emit orderUpdatedByUser(order);
                    }
                    break;
                }
            }
        }
    }

    QGraphicsPixmapItem::mouseReleaseEvent(event);
}

int MapUnitItem::uniqueId() const
{
    return mUniqueId;
}





