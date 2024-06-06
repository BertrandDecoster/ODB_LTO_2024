#ifndef ORDER_H
#define ORDER_H

#include "drawing.h"

#include <QJsonObject>
#include <QPointF>
#include <QString>



class Order
{
    ///////////////////////////////////////////////////////////////////////
    /// ENUMS                                                           ///
    ///////////////////////////////////////////////////////////////////////
public:
    enum ORDER_TYPE{
        MOVEMENT,
        ATTACK,
        CUSTOM
    };


    ///////////////////////////////////////////////////////////////////////
    /// CONSTRUCTORS AND DESTRUCTOR                                     ///
    ///////////////////////////////////////////////////////////////////////
public:
    Order(int uniqueId, ORDER_TYPE orderType);
    Order(const Order & order);
    Order(QJsonObject & json);

    Order& operator=(Order other);

    void clear();



    ///////////////////////////////////////////////////////////////////////
    /// SERIALIZATION                                                   ///
    ///////////////////////////////////////////////////////////////////////
public:
    void readJson(const QJsonObject &json);
    void writeJson(QJsonObject & json) const;



    ///////////////////////////////////////////////////////////////////////
    /// GETTERS AND SETTERS                                             ///
    ///////////////////////////////////////////////////////////////////////
public:
    int originUniqueId() const;
    void setOriginUniqueId(int originUniqueId);

    ORDER_TYPE orderType() const;

    bool isEnabled() const;
    void setIsEnabled(bool isEnabled);

    int destinationUniqueId() const;
    void setDestinationUniqueId(int destinationUniqueId);

    QPointF destinationPosition() const;
    void setDestinationPosition(const QPointF &destinationPosition);

    QString customText() const;
    void setCustomText(const QString &customText);

    Drawing toDrawing();


    ///////////////////////////////////////////////////////////////////////
    /// MISC                                                            ///
    ///////////////////////////////////////////////////////////////////////
public:
    void debug();

    ///////////////////////////////////////////////////////////////////////
    /// ATTRIBUTES                                                      ///
    ///////////////////////////////////////////////////////////////////////
private:
    int mOriginUniqueId;
    ORDER_TYPE mOrderType;

    bool mIsEnabled;

    int mDestinationUniqueId;
    QPointF mDestinationPosition;
    QString mCustomText;

public:
    static const int NB_ORDERS = 3;
    static QString toString(Order::ORDER_TYPE type);

public:
    // For more explanation, refer to :
    // https://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom#3279550
    friend void swap(Order & first, Order& second)
    {
        using std::swap;
        swap(first.mOriginUniqueId, second.mOriginUniqueId);
        swap(first.mOrderType, second.mOrderType);
        swap(first.mIsEnabled, second.mIsEnabled);
        swap(first.mDestinationUniqueId, second.mDestinationUniqueId);
        swap(first.mDestinationPosition, second.mDestinationPosition);
        swap(first.mCustomText, second.mCustomText);
    }

};

#endif // ORDER_H
