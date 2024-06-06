#include "vepolyline.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainterPath>
#include "dotsignal.h"
#include "mymapgraphicsscene.h"

VEPolyline::VEPolyline(bool isUnbreakable, QObject *parent) :
    QObject(parent),
    m_pointForCheck(-1),
    mIsUnbreakable(isUnbreakable)
{
    setAcceptHoverEvents(true);
//    setFlags(ItemSendsGeometryChanges);
    setFlags(ItemSendsGeometryChanges);
//    setFlag(ItemIsMovable);
}

VEPolyline::~VEPolyline()
{

}

int VEPolyline::type() const{
    return Type;
}

QPointF VEPolyline::previousPosition() const
{
    return m_previousPosition;
}

void VEPolyline::setPreviousPosition(const QPointF previousPosition)
{
    if (m_previousPosition == previousPosition)
        return;

    m_previousPosition = previousPosition;
    emit previousPositionChanged();
}

void VEPolyline::setPath(const QPainterPath &path)
{
    QGraphicsPathItem::setPath(path);
}

void VEPolyline::write(QJsonObject &json) const
{
    json["scenePos"] = QJsonArray{scenePos().x(), scenePos().y()};
    json["width"] = pen().width();
    json["color"] = pen().color().name();
    QJsonArray jsonPath;

    QPainterPath p = path();
    for(int i=0; i< p.elementCount() ; ++i ){
        QPainterPath::Element e = p.elementAt(i);
        QJsonArray jsonElement;
        jsonElement.append(e.x);
        jsonElement.append(e.y);
        jsonPath.append(jsonElement);
    }
    json["path"] = jsonPath;
}

void VEPolyline::fromDrawing(Drawing &drawing)
{
    mUniqueId = drawing.uniqueId();
    mIsUnbreakable = drawing.isOrder();
    setPath(drawing.painterPath());

    QPen pen;
    pen.setWidth(drawing.width());
    pen.setColor(drawing.borderColor());
    setPen(pen);
    setPos(drawing.pos());
}

Drawing VEPolyline::toDrawing() const
{
    Drawing answer;
    answer.setUniqueId(mUniqueId);
    answer.setType(Drawing::POLYLINE);
    answer.setPainterPath(path());
    answer.setWidth(pen().width());
    answer.setBorderColor(pen().color());
    answer.setPos(pos());
    return answer;
}

//void VEPolyline::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
//{
//    if (m_leftMouseButtonPressed) {
//        auto dx = event->scenePos().x() - m_previousPosition.x();
//        auto dy = event->scenePos().y() - m_previousPosition.y();
//        moveBy(dx,dy);
//        setPreviousPosition(event->scenePos());
//        emit signalMove(this, dx, dy);
//    }
//    QGraphicsItem::mouseMoveEvent(event);
//}

//void VEPolyline::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    if (event->button() & Qt::LeftButton) {
//        m_leftMouseButtonPressed = true;
//        setPreviousPosition(event->scenePos());
//        emit clicked(this);
//    }
//    QGraphicsItem::mousePressEvent(event);
//}

//void VEPolyline::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
//{
//    if (event->button() & Qt::LeftButton) {
//        m_leftMouseButtonPressed = false;
//    }
//    QGraphicsItem::mouseReleaseEvent(event);
//}

void VEPolyline::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // DRAWING MODE CHECK
    MyMapGraphicsScene * s = dynamic_cast<MyMapGraphicsScene*>(scene());
    if(!s){
        QGraphicsItem::mousePressEvent(event);
        return;
    }
    switch(s->currentMode()){
    case MyMapGraphicsScene::UNIT_MODE:
        // Nothing
        break;
    case MyMapGraphicsScene::DRAWING_MODE:
        if (event->button() & Qt::LeftButton) {
            m_leftMouseButtonPressed = true;
            setPreviousPosition(event->scenePos());
            event->accept();    // If you don't accept, the item doesn't become
            // the mouse grabber, and as such doesn't receive the subsequent mouseMoveEvents
            emit drawingClicked(mUniqueId);
        }

        break;

    }

    QGraphicsItem::mousePressEvent(event);
}

void VEPolyline::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // DRAWING MODE CHECK
    MyMapGraphicsScene * s = dynamic_cast<MyMapGraphicsScene*>(scene());
    if(!s){
        QGraphicsItem::mousePressEvent(event);
        return;
    }
    switch(s->currentMode()){
    case MyMapGraphicsScene::UNIT_MODE:
        // Nothing
        break;
    case MyMapGraphicsScene::DRAWING_MODE:
        if (m_leftMouseButtonPressed) {
            auto dx = event->scenePos().x() - m_previousPosition.x();
            auto dy = event->scenePos().y() - m_previousPosition.y();
            moveBy(dx,dy);
            setPreviousPosition(event->scenePos());
            //            emit drawingMoved(this, dx, dy);
            Drawing drawing = toDrawing();
            drawing.moveBy(dx, dy);
            emit drawingUpdated(drawing);
        }

    }

    //    qDebug() << "Move";

    QGraphicsItem::mouseMoveEvent(event);
}



void VEPolyline::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // DRAWING MODE CHECK
    // Actually nothing

    if (event->button() & Qt::LeftButton) {
        m_leftMouseButtonPressed = false;
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void VEPolyline::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // Unbreakable lines can't be subdivided
    if(mIsUnbreakable){
        QGraphicsItem::mouseDoubleClickEvent(event);
        return;
    }

    // DRAWING MODE CHECK
    MyMapGraphicsScene * s = dynamic_cast<MyMapGraphicsScene*>(scene());
    if(!s){
        QGraphicsItem::mouseDoubleClickEvent(event);
        return;
    }

    // When not in drawing mode, do nothing
    if(!(s->currentMode() == MyMapGraphicsScene::DRAWING_MODE)){
        QGraphicsItem::mouseDoubleClickEvent(event);
        return;
    }

    QPointF clickPos = event->pos();
    QLineF checkLineFirst(clickPos.x() - 5, clickPos.y() - 5, clickPos.x() + 5, clickPos.y() + 5);
    QLineF checkLineSecond(clickPos.x() + 5, clickPos.y() - 5, clickPos.x() - 5, clickPos.y() + 5);
    QPainterPath oldPath = path();
    QPainterPath newPath;
    for(int i = 0; i < oldPath.elementCount(); i++){
        QLineF checkableLine(oldPath.elementAt(i), oldPath.elementAt(i+1));
        if(checkableLine.intersect(checkLineFirst,0) == 1 || checkableLine.intersect(checkLineSecond,0) == 1){
            if(i == 0){
                newPath.moveTo(oldPath.elementAt(i));
                newPath.lineTo(clickPos);
            } else {
                newPath.lineTo(oldPath.elementAt(i));
                newPath.lineTo(clickPos);
            }
        } else {
            if(i == 0){
                newPath.moveTo(oldPath.elementAt(i));
            } else {
                newPath.lineTo(oldPath.elementAt(i));
            }
        }
        if(i == (oldPath.elementCount() - 2)) {
            newPath.lineTo(oldPath.elementAt(i + 1));
            i++;
        }
    }
    setPath(newPath);
    updateDots();

    emit drawingUpdated(toDrawing());
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void VEPolyline::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    // DRAWING MODE CHECK
    // Actually nothing

    if(!listDotes.isEmpty()){
        foreach (DotSignal *dot, listDotes) {
            dot->deleteLater();
        }
        listDotes.clear();
    }
    QGraphicsItem::hoverLeaveEvent(event);
}

void VEPolyline::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    // DRAWING MODE CHECK
    // Actually nothing
    QGraphicsItem::hoverMoveEvent(event);
}

void VEPolyline::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // DRAWING MODE CHECK
    MyMapGraphicsScene * s = dynamic_cast<MyMapGraphicsScene*>(scene());
    if(!s){
        QGraphicsItem::hoverEnterEvent(event);
        return;
    }

    // When not in drawing mode, don't display the squares
    if(!(s->currentMode() == MyMapGraphicsScene::DRAWING_MODE)){
        QGraphicsItem::hoverEnterEvent(event);
        return;
    }

    QPainterPath linePath = path();
    for(int i = 0; i < linePath.elementCount(); i++){
        QPointF point = linePath.elementAt(i);
        DotSignal *dot = new DotSignal(point, this);
        connect(dot, &DotSignal::signalMove, this, &VEPolyline::slotMove);
        connect(dot, &DotSignal::signalMouseRelease, this, &VEPolyline::checkForDeletePoints);
        dot->setDotFlags(DotSignal::Movable);
        listDotes.append(dot);
    }
    QGraphicsItem::hoverEnterEvent(event);
}

void VEPolyline::slotMove(QGraphicsItem *signalOwner, qreal dx, qreal dy)
{
    QPainterPath linePath = path();
    for(int i = 0; i < linePath.elementCount(); i++){
        if(listDotes.at(i) == signalOwner){
            QPointF pathPoint = linePath.elementAt(i);
            linePath.setElementPositionAt(i, pathPoint.x() + dx, pathPoint.y() + dy);
            m_pointForCheck = i;
            if(m_pointForCheck < -1){
                qDebug() << "Oops";
            }
        }
    }
    setPath(linePath);
    emit drawingUpdated(toDrawing());
}

void VEPolyline::checkForDeletePoints()
{
    if(m_pointForCheck != -1){
        QPainterPath linePath = path();

        QPointF pathPoint = linePath.elementAt(m_pointForCheck);
        if(m_pointForCheck > 0){
            QLineF lineToNear(linePath.elementAt(m_pointForCheck-1),pathPoint);
            if(lineToNear.length() < 6.0) {
                QPainterPath newPath;
                newPath.moveTo(linePath.elementAt(0));
                for(int i = 1; i < linePath.elementCount(); i++){
                    if(i != m_pointForCheck){
                        newPath.lineTo(linePath.elementAt(i));
                    }
                }
                setPath(newPath);
            }
        }
        if(m_pointForCheck < linePath.elementCount() - 1){
            QLineF lineToNear(linePath.elementAt(m_pointForCheck+1),pathPoint);
            if(lineToNear.length() < 6.0) {
                QPainterPath newPath;
                newPath.moveTo(linePath.elementAt(0));
                for(int i = 1; i < linePath.elementCount(); i++){
                    if(i != m_pointForCheck){
                        newPath.lineTo(linePath.elementAt(i));
                    }
                }
                setPath(newPath);
            }
        }
        updateDots();
        m_pointForCheck = -1;
    }
}

int VEPolyline::uniqueId() const
{
    return mUniqueId;
}

void VEPolyline::setUniqueId(int uniqueId)
{
    mUniqueId = uniqueId;
}


bool VEPolyline::isUnbreakable() const
{
    return mIsUnbreakable;
}

void VEPolyline::setIsUnbreakable(bool isUnbreakable)
{
    mIsUnbreakable = isUnbreakable;
}

void VEPolyline::updateDots()
{
    if(!listDotes.isEmpty()){
        foreach (DotSignal *dot, listDotes) {
            dot->deleteLater();
        }
        listDotes.clear();
    }
    QPainterPath linePath = path();
    for(int i = 0; i < linePath.elementCount(); i++){
        QPointF point = linePath.elementAt(i);
        DotSignal *dot = new DotSignal(point, this);
        connect(dot, &DotSignal::signalMove, this, &VEPolyline::slotMove);
        connect(dot, &DotSignal::signalMouseRelease, this, &VEPolyline::checkForDeletePoints);
        dot->setDotFlags(DotSignal::Movable);
        listDotes.append(dot);
    }
}

QPainterPath VEPolyline::shape() const
{
    QPainterPath myPath = path();
    QPen myPen = pen();
    const qreal penWidthZero = qreal(0.00000001);
    if (myPath == QPainterPath() || myPen == Qt::NoPen)
        return myPath;
    QPainterPathStroker ps;
    ps.setCapStyle(myPen.capStyle());
    if (myPen.widthF() <= 0.0)
        ps.setWidth(penWidthZero);
    else
        ps.setWidth(myPen.widthF());
    ps.setJoinStyle(myPen.joinStyle());
    ps.setMiterLimit(myPen.miterLimit());
    QPainterPath answer = ps.createStroke(myPath);
    return answer;
}
