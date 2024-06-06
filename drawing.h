#ifndef DRAWING_H
#define DRAWING_H

#include <QColor>
#include <QJsonObject>
#include <QPainterPath>
#include <QRectF>
#include <QSet>

class QGraphicsItem;

class Drawing
{
public:
    enum DRAWING_TYPE{
        POLYLINE,
        RECTANGLE,
        ELLIPSE,
        UNUSED
    };

    enum LAYER{
        PHASELINE_BIG,
        PHASELINE_MEDIUM,
        PHASELINE_SMALL,
        INTERDICTION,
        OTHER
    };
public:
    Drawing();
    Drawing(const Drawing & other);

    DRAWING_TYPE type() const;
    void setType(const DRAWING_TYPE &type);

    int uniqueId() const;
    void setUniqueId(int uniqueId);

    QRectF rectangle() const;
    void setRectangle(const QRectF &rectangle);

    QPointF pos() const;
    void setPos(const QPointF &pos);

    bool isOrder() const;
    void setIsIntention(bool isOrder);

    double rotation() const;
    void setRotation(double rotation);

    QPainterPath painterPath() const;
    void setPainterPath(const QPainterPath &painterPath);

    int width() const;
    void setWidth(int width);

    QColor borderColor() const;
    void setBorderColor(const QColor &borderColor);

    QColor interiorColor() const;
    void setInteriorColor(const QColor &interiorColor);

    int getIntentionUnitUniqueId() const;
    void setIntentionUnitUniqueId(int intentionUnitUniqueId);

    QSet<LAYER> getLayers() const;
    void setLayers(const QSet<LAYER> &layers);





    void moveBy(double dx, double dy);
    void addLayer(LAYER layer);

    void debug();


    void write(QJsonObject &json) const;
    void writePolyline(QJsonObject &json) const;
    void writeShape(QJsonObject &json) const;
    void writeRectangle(QJsonObject &json) const;
    void writeEllipse(QJsonObject &json) const;


    void read(const QJsonObject & json);
    void readPolyline(const QJsonObject & json);
    void readRectangle(const QJsonObject & json);
    void readEllipse(const QJsonObject & json);

//    QGraphicsItem *getGraphicsItem();
//    QGraphicsItem *getPolylineItem();
//    QGraphicsItem *getRectangleItem();
//    QGraphicsItem *getEllipseItem();


private:
    int mUniqueId;              // ALL

    DRAWING_TYPE mType;         // ALL
    bool mIsIntention;
    int mIntentionUnitUniqueId;

    QPointF mPos;                // ALL
    QRectF mRectangle;
    double mRotation;
    QPainterPath mPainterPath;

    int mWidth;                 // ALL
    QColor mBorderColor;        // ALL
    QColor mInteriorColor;

    QSet<LAYER> mLayers;
};

#endif // DRAWING_H
