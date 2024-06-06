#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QObject>
#include <QWidget>
#include <QGraphicsScene>
#include <QMap>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QPainter>

#include "unit.h"


class Data;

QT_BEGIN_NAMESPACE
class QTimeLine;
class QParallelAnimationGroup;
QT_END_NAMESPACE


///////////////////////////////////////////////////////////////////////
/// PUBLIC  DATA STRUCTURES                                         ///
///////////////////////////////////////////////////////////////////////
/// Qt doesn't support nested QObjects

class MyGraphicsLineItem : public QObject, public QGraphicsLineItem{
    Q_OBJECT
    Q_PROPERTY(QLineF line READ line WRITE setLine)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
};

class MyGraphicsPathItem : public QObject, public QGraphicsPathItem{
    Q_OBJECT
    Q_PROPERTY(QPainterPath path READ path WRITE setPath)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
};

class MyUnitItem : public QObject, public QGraphicsPixmapItem{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    MyUnitItem(int id, const QPixmap & pixmap, QGraphicsItem * parent = nullptr):
        QGraphicsPixmapItem (pixmap, parent), mUniqueId(id){}

    // QGraphicsItem interface
public:
    bool sceneEvent(QEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public:
    QPointF mFixedPos;
    int mUniqueId;
};




class MyTaskOrgGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

    ///////////////////////////////////////////////////////////////////////
    /// PRIVATE DATA STRUCTURES                                         ///
    ///////////////////////////////////////////////////////////////////////

    struct MyAnimationPos{
        int id;
        qreal startX;
        qreal startY;
        qreal endX;
        qreal endY;
    };


    ///////////////////////////////////////////////////////////////////////
    /// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
    ///////////////////////////////////////////////////////////////////////
public:
    MyTaskOrgGraphicsScene(Data * d, QObject *parent = Q_NULLPTR);
    virtual ~MyTaskOrgGraphicsScene() override;

public slots:
    void clear();

    ///////////////////////////////////////////////////////////////////////
    /// GETTERS, SETTERS                                                ///
    ///////////////////////////////////////////////////////////////////////
protected:
    MyGraphicsLineItem * getLine(QGraphicsScene *scene);
    MyGraphicsPathItem * getPath(QGraphicsScene *scene);


    ///////////////////////////////////////////////////////////////////////
    /// UPDATE THE DATA STRUCTURE                                       ///
    ///////////////////////////////////////////////////////////////////////
public slots:
    void addUniqueId(int uniqueId);
    void deleteUniqueId(int name);
    void updateUnitContent(Unit unit);
    void changeUnitParent(int parentId, int uniqueId);
    void removeUnitParent(int uniqueId);
    void debug();

    ///////////////////////////////////////////////////////////////////////
    /// UPDATE THE DISPLAY                                              ///
    ///////////////////////////////////////////////////////////////////////
public slots:
    void selectUnit(int uniqueId);
    void setDisplayFullFamily(bool value);
    void updateTextSize(qreal size);
    void resizeToBoundingRect();

protected:
    void launchAnimationPos(const MyAnimationPos & mData, QParallelAnimationGroup *animationGroup = Q_NULLPTR);
    void launchAnimationOpacity(QGraphicsItem * unit, bool isAppearing, QParallelAnimationGroup *animationGroup = Q_NULLPTR);
    void launchAnimationFamilyLines(const QVector<int> & family, QParallelAnimationGroup *animationGroup = Q_NULLPTR);

    QVector<QVector<int>> getItemPlacement(int uniqueId);

    ///////////////////////////////////////////////////////////////////////
    /// QGRAPHICS OERRIDE                                               ///
    ///////////////////////////////////////////////////////////////////////
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

    ///////////////////////////////////////////////////////////////////////
    /// SIGNALS                                                         ///
    ///////////////////////////////////////////////////////////////////////
public:
signals:
    void unitSelected(int unit);


    ///////////////////////////////////////////////////////////////////////
    /// ATTRIBUTES                                                      ///
    ///////////////////////////////////////////////////////////////////////
private:
    Data * mData;

    QMap<int, QGraphicsItem *> mItems;
    QVector<MyGraphicsLineItem *> mFreeLines;
    QVector<MyGraphicsLineItem *> mUsedLines;
    QVector<MyGraphicsPathItem *> mFreePaths;
    QVector<MyGraphicsPathItem *> mUsedPaths;

    int mSelectedUniqueId;
    bool mDisplayFullFamily;
    bool mUnitSelectedFirst;

    QParallelAnimationGroup * animationGroup = nullptr;

    QVector<QVector<int>> mItemPlacement; // Not very useful for now

    int mUnitPressed;
    QPointF mPosUnitPressed;

    ///////////////////////////////////////////////////////////////////////
    /// STATIC                                                          ///
    ///////////////////////////////////////////////////////////////////////
public:
    static qreal textSize;

    static qreal centerX;   // Location of mSelectedUniqueId
    static qreal centerY;

    static qreal parentX;   // Location of parent of mSelectedUniqueId
    static qreal parentY;

    static qreal siblingDeltaX; // Delta for the siblings of mSelectedUniqueId

    static qreal parentToChildDeltaX; // Delta between mSelectedUniqueId and its
    static qreal parentToChildDeltaY; // first child (same for sibling/first nephew)

    static qreal childDeltaY;   // Delta between the children/nephews of mSelectedUniqueId

    static int animationDuration;


};


class HoverBoxFilter : public QObject {
    Q_OBJECT
public:
    HoverBoxFilter(QGraphicsScene* scene);

private:
    bool eventFilter(QObject* object, QEvent* event);

private:
    QGraphicsScene * mScene;

};

#endif // MYGRAPHICSSCENE_H
