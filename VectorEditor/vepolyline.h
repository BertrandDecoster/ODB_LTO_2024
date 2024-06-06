#ifndef VEPOLYLINE_H
#define VEPOLYLINE_H

#include <QObject>
#include <QGraphicsPathItem>
#include <drawing.h>

class DotSignal;
class QGraphicsSceneMouseEvent;

class VEPolyline : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF previousPosition READ previousPosition WRITE setPreviousPosition NOTIFY previousPositionChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    enum { Type = 100 };
public:
    explicit VEPolyline(bool isUnbreakable = false, QObject * parent = 0);
    ~VEPolyline() override;
    int type() const override;

    QPointF previousPosition() const;
    void setPreviousPosition(const QPointF previousPosition);
    void setPath(const QPainterPath &path);


    void write(QJsonObject &json) const;

    void fromDrawing(Drawing & drawing);
    Drawing toDrawing() const;

    bool isUnbreakable() const;
    void setIsUnbreakable(bool isUnbreakable);

    int uniqueId() const;
    void setUniqueId(int uniqueId);

    // QGraphicsItem interface
public:
    QPainterPath shape() const override;

signals:
    void previousPositionChanged();
    void drawingClicked(int uniqueId);
    void drawingMoved(QGraphicsItem *item, qreal dx, qreal dy);
    void drawingUpdated(Drawing drawing);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

public slots:

private slots:
    void slotMove(QGraphicsItem *signalOwner, qreal dx, qreal dy);
    void checkForDeletePoints();

private:
    int mUniqueId;
    QPointF m_previousPosition;
    bool m_leftMouseButtonPressed;
    QList<DotSignal *> listDotes;
    int m_pointForCheck;

    bool mIsUnbreakable;

    void updateDots();


};

#endif // VEPOLYLINE_H
