#include "order.h"

#include "unit.h"

#include <QJsonArray>
#include <QDebug>


Order::Order(int uniqueId, ORDER_TYPE orderType):
    mOriginUniqueId(uniqueId),
    mOrderType(orderType),
    mIsEnabled(false),
    mDestinationUniqueId(-1),
    mDestinationPosition(Unit::illegalPosition()),
    mCustomText(QString())
{

}


Order::Order(const Order &order):Order(order.mOriginUniqueId, order.mOrderType)
{
    mIsEnabled = order.mIsEnabled;
    mDestinationUniqueId = order.mDestinationUniqueId;
    mDestinationPosition = order.mDestinationPosition;
    mCustomText = order.mCustomText;
}

Order::Order(QJsonObject &json)
{
    readJson(json);
}



Order &Order::operator=(Order other)
{
    swap(*this, other);
    return *this;
}

void Order::clear()
{
    mIsEnabled = false;
    mDestinationUniqueId = -1;
    mDestinationPosition = Unit::illegalPosition();
    mCustomText = QString();
}

void Order::readJson(const QJsonObject &json)
{
    if (json.contains("originUniqueId") && json["originUniqueId"].isDouble())
        mOriginUniqueId = json["originUniqueId"].toInt();

    if (json.contains("orderType") && json["orderType"].isDouble())
        mOrderType = static_cast<ORDER_TYPE>(json["orderType"].toInt());

    if (json.contains("isEnabled") && json["isEnabled"].isBool())
        mIsEnabled = json["isEnabled"].toBool();

    if (json.contains("destinationUniqueId") && json["destinationUniqueId"].isDouble()){
        mDestinationUniqueId = json["destinationUniqueId"].toInt();
    }
    else{
        mDestinationUniqueId = -1;
    }

    if (json.contains("customText") && json["customText"].isString()){
        mCustomText = json["customText"].toString();
    }
    else{
        mCustomText = QString("");
    }

    if (json.contains("destinationPosition") && json["destinationPosition"].isArray()){
        QJsonArray posArray = json["destinationPosition"].toArray();
        mDestinationPosition = QPointF(posArray[0].toDouble(), posArray[1].toDouble());
    }
    else{
        mDestinationPosition = Unit::illegalPosition();
    }
}

void Order::writeJson(QJsonObject &json) const
{
    json["originUniqueId"] = mOriginUniqueId;
    json["orderType"] = static_cast<int>(mOrderType);
    json["isEnabled"] = mIsEnabled;
    if(!mCustomText.isEmpty()){
        json["customText"] = mCustomText;
    }
    if(Unit::isPosLegal(mDestinationPosition)){
        QJsonArray pos;
        pos.append(QJsonValue{mDestinationPosition.x()});
        pos.append(QJsonValue{mDestinationPosition.y()});
        json["destinationPosition"] = pos;
    }
    if(mDestinationUniqueId >= 0){
        json["destinationUniqueId"] = mDestinationUniqueId;
    }
}

///////////////////////////////////////////////////////////////////////
/// GETTERS AND SETTERS                                             ///
///////////////////////////////////////////////////////////////////////

Order::ORDER_TYPE Order::orderType() const
{
    return mOrderType;
}

int Order::originUniqueId() const
{
    return mOriginUniqueId;
}

int Order::destinationUniqueId() const
{
    return mDestinationUniqueId;
}

void Order::setDestinationUniqueId(int destinationUniqueId)
{
    mDestinationUniqueId = destinationUniqueId;
}

QPointF Order::destinationPosition() const
{
    return mDestinationPosition;
}

void Order::setDestinationPosition(const QPointF &destinationPosition)
{
    mDestinationPosition = destinationPosition;
}

QString Order::customText() const
{
    return mCustomText;
}

void Order::setCustomText(const QString &customText)
{
    mCustomText = customText;
}

Drawing Order::toDrawing()
{
    Drawing drawing;

    return drawing;
}

void Order::debug()
{
    qDebug() << "Order origin ID " << mOriginUniqueId;

    qDebug() << "Order type " << toString(mOrderType);
    if(!mIsEnabled){
        qDebug() << "Order disabled";
        return;
    }

    switch (mOrderType) {
    case Order::MOVEMENT:
        qDebug() << "Destination " << mDestinationPosition;
        break;

    case Order::ATTACK:{
        if(mDestinationUniqueId >= 0){
            qDebug() << "Target " << mDestinationUniqueId;
        }
        else{
            qDebug() << "Target " << mDestinationPosition;
        }
        break;
    }
    case Order::CUSTOM:{
        if(!mCustomText.isEmpty()){
            qDebug() << "Custom " << mCustomText;
        }
    }
    }
}

void Order::setOriginUniqueId(int originUniqueId)
{
    mOriginUniqueId = originUniqueId;
}

QString Order::toString(Order::ORDER_TYPE type)
{
    switch (type) {
    case Order::MOVEMENT:
        return QString("Movement");

    case Order::ATTACK:
        return QString("Attack");

    case Order::CUSTOM:
        return QString("Custom");
    }
}

bool Order::isEnabled() const
{
    return mIsEnabled;
}

void Order::setIsEnabled(bool isEnabled)
{
    mIsEnabled = isEnabled;
}

