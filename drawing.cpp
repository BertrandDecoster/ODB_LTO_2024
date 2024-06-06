#include "drawing.h"

#include <QJsonArray>
#include <QDebug>
#include <QPen>

#include "unit.h"

#include "VectorEditor/veellipse.h"
#include "VectorEditor/verectangle.h"
#include "VectorEditor/vepolyline.h"


Drawing::Drawing():
    mUniqueId(-1),
    mType(Drawing::UNUSED),
    mIsIntention(false),
    mIntentionUnitUniqueId(-1),
    mPos(QPointF()),
    mRectangle(QRectF()),
    mRotation(0.0),
    mPainterPath(QPainterPath()),
    mWidth(1),
    mBorderColor(Qt::black),
    mInteriorColor(Qt::black)
{

}

Drawing::Drawing(const Drawing &other):
    mUniqueId(other.mUniqueId),
    mType(other.mType),
    mIsIntention(other.mIsIntention),
    mPos(other.mPos),
    mRectangle(other.mRectangle),
    mRotation(other.mRotation),
    mPainterPath(other.mPainterPath),
    mWidth(other.mWidth),
    mBorderColor(other.mBorderColor),
    mInteriorColor(other.mInteriorColor)
{

}

QRectF Drawing::rectangle() const
{
    return mRectangle;
}

void Drawing::setRectangle(const QRectF &rectangle)
{
    mRectangle = rectangle;
}

QPainterPath Drawing::painterPath() const
{
    return mPainterPath;
}

void Drawing::setPainterPath(const QPainterPath &painterPath)
{
    mPainterPath = painterPath;
}

int Drawing::width() const
{
    return mWidth;
}

void Drawing::setWidth(int width)
{
    mWidth = width;
}

QColor Drawing::borderColor() const
{
    return mBorderColor;
}

void Drawing::setBorderColor(const QColor &borderColor)
{
    mBorderColor = borderColor;
}

QColor Drawing::interiorColor() const
{
    return mInteriorColor;
}

void Drawing::setInteriorColor(const QColor &interiorColor)
{
    mInteriorColor = interiorColor;
}

void Drawing::moveBy(double dx, double dy)
{
    mPos = mPos + QPointF(dx, dy);
}

void Drawing::addLayer(Drawing::LAYER layer)
{
    mLayers.insert(layer);
}

void Drawing::debug()
{

}

void Drawing::write(QJsonObject &json) const
{
    json["uniqueId"] = mUniqueId;
    json["type"] = static_cast<int>(mType);
    json["pos"] = QJsonArray{mPos.x(), mPos.y()};
    switch (mType) {
    case POLYLINE:
        writePolyline(json);
        break;
    case RECTANGLE:
    case ELLIPSE:
        writeShape(json);
        break;
    }
}

void Drawing::writePolyline(QJsonObject &json) const
{
    json["width"] = mWidth;
    json["color"] = mBorderColor.name();
    json["intention"] = mIsIntention;

    QJsonArray jsonPath;
    for(int i=0; i< mPainterPath.elementCount() ; ++i ){
        QPainterPath::Element e = mPainterPath.elementAt(i);
        QJsonArray jsonElement;
        jsonElement.append(e.x);
        jsonElement.append(e.y);
        jsonPath.append(jsonElement);
    }
    json["path"] = jsonPath;
}

void Drawing::writeShape(QJsonObject &json) const
{
    json["rect"] = QJsonArray{mRectangle.x(), mRectangle.y(), mRectangle.width(), mRectangle.height()};
    json["rotation"] = mRotation;
    json["borderWidth"] = mWidth;
    json["borderColor"] = mBorderColor.name();
    json["interiorColor"] = mInteriorColor.name(QColor::HexArgb);

    switch (mType) {
    case RECTANGLE:
        writeRectangle(json);
        break;
    case ELLIPSE:
        writeEllipse(json);
        break;
    default:
        break;
    }
}

void Drawing::writeRectangle(QJsonObject &json) const
{

}

void Drawing::writeEllipse(QJsonObject &json) const
{

}

void Drawing::read(const QJsonObject &json)
{
    mUniqueId = static_cast<int>(json["uniqueId"].toDouble());
    mType = static_cast<Drawing::DRAWING_TYPE>(json["type"].toDouble());
    mPos = QPointF(json["pos"].toArray()[0].toDouble(), json["pos"].toArray()[1].toDouble());
    switch(mType){
    case Drawing::POLYLINE:
        readPolyline(json);
        return;
    case Drawing::RECTANGLE:
        readRectangle(json);
        return;
    case Drawing::ELLIPSE:
        readEllipse(json);
        return;
    }
}

void Drawing::readPolyline(const QJsonObject &json)
{
    qDebug().noquote() << json;
    mType = Drawing::POLYLINE;


    QJsonArray path = json["path"].toArray();
    for (int index = 0; index < path.size(); ++index){
        QJsonValue pointValue = path[index];
        QJsonArray point = pointValue.toArray();
        if(index == 0){
            QPointF start = QPointF(point[0].toDouble(), point[1].toDouble());
            mPainterPath = QPainterPath(start);
            continue;
        }
        QPointF next = QPointF(point[0].toDouble(), point[1].toDouble());
        mPainterPath.lineTo(next);
    }

    mWidth = json["width"].toDouble();
    mBorderColor = QColor(json["color"].toString());




    //    QJsonArray line = json["line"].toArray();
    //    QPointF end;
    //    for(QJsonValue point : line){
    //        QPointF current = QPointF(point.toArray()[0].toDouble(), point.toArray()[1].toDouble());
    //        end = current;
    //    }




    //    Unit * unit = mData->units()[uniqueId];
    //    Unit copy = Unit(*unit);
    //    copy.setPos(end);
    //    UpdateUnitCommand * command = new UpdateUnitCommand(mData, copy);
    //    dynamic_cast<MainWindow*>(parent())->undoStack()->push(command);



}

void Drawing::readRectangle(const QJsonObject &json)
{
    qDebug().noquote() << json;
    mType = Drawing::RECTANGLE;

    mWidth = json["width"].toDouble();
    mBorderColor = QColor(json["borderColor"].toString());
    mInteriorColor = QColor(json["interiorColor"].toString());

    QJsonArray rectangleArray = json["rectangle"].toArray();
    mRectangle = QRectF(rectangleArray[0].toDouble(),
            rectangleArray[1].toDouble(),
            rectangleArray[2].toDouble(),
            rectangleArray[3].toDouble());

    mRotation = json["rotation"].toDouble();


}

void Drawing::readEllipse(const QJsonObject &json)
{
    qDebug().noquote() << json;
    mType = Drawing::ELLIPSE;

    mWidth = json["width"].toDouble();
    mBorderColor = QColor(json["borderColor"].toString());
    mInteriorColor = QColor(json["interiorColor"].toString());

    QJsonArray rectangleArray = json["rectangle"].toArray();
    mRectangle = QRectF(rectangleArray[0].toDouble(),
            rectangleArray[1].toDouble(),
            rectangleArray[2].toDouble(),
            rectangleArray[3].toDouble());

    mRotation = json["rotation"].toDouble();


}

//QGraphicsItem *Drawing::getGraphicsItem()
//{
//    switch(mType){
//    case Drawing::POLYLINE:
//        return getPolylineItem();

//    case Drawing::RECTANGLE:
//        return getRectangleItem();

//    case Drawing::ELLIPSE:
//        return getEllipseItem();
//    }
//}

//QGraphicsItem *Drawing::getPolylineItem()
//{
//    VEPolyline * answer = new VEPolyline(0);
//    answer->setPos(mPos);
//    QPen pen;
//    pen.setColor(mBorderColor);
//    pen.setWidth(mWidth);
//    answer->setPen(pen);

//    answer->setPath(mPainterPath);


//    return answer;
//}

//QGraphicsItem *Drawing::getRectangleItem()
//{
//    VERectangle * answer = new VERectangle(0);
//    answer->setPos(mPos);
//    QPen pen;
//    pen.setColor(mBorderColor);
//    pen.setWidth(mWidth);
//    answer->setPen(pen);

//    answer->setRect(mRectangle);
//    answer->setRotation(mRotation);
//    QBrush brush{QColor(mInteriorColor)};
//    answer->setBrush(brush);

//    return answer;
//}

//QGraphicsItem *Drawing::getEllipseItem()
//{
//    VEEllipse * answer = new VEEllipse(0);
//    answer->setPos(mPos);
//    QPen pen;
//    pen.setColor(mBorderColor);
//    pen.setWidth(mWidth);
//    answer->setPen(pen);

//    answer->setRect(mRectangle);
//    answer->setRotation(mRotation);
//    QBrush brush{QColor(mInteriorColor)};
//    answer->setBrush(brush);


//    return answer;
//}

int Drawing::getIntentionUnitUniqueId() const
{
    return mIntentionUnitUniqueId;
}

void Drawing::setIntentionUnitUniqueId(int intentionUnitUniqueId)
{
    mIntentionUnitUniqueId = intentionUnitUniqueId;
}

QSet<Drawing::LAYER> Drawing::getLayers() const
{
    return mLayers;
}

void Drawing::setLayers(const QSet<LAYER> &layers)
{
    mLayers = layers;
}

QPointF Drawing::pos() const
{
    return mPos;
}

void Drawing::setPos(const QPointF &pos)
{
    mPos = pos;
}

bool Drawing::isOrder() const
{
    return mIsIntention;
}

void Drawing::setIsIntention(bool isIntention)
{
    mIsIntention = isIntention;
}

double Drawing::rotation() const
{
    return mRotation;
}

void Drawing::setRotation(double rotation)
{
    mRotation = rotation;
}








Drawing::DRAWING_TYPE Drawing::type() const
{
    return mType;
}

void Drawing::setType(const DRAWING_TYPE &type)
{
    mType = type;
}

int Drawing::uniqueId() const
{
    return mUniqueId;
}

void Drawing::setUniqueId(int uniqueId)
{
    mUniqueId = uniqueId;
}
