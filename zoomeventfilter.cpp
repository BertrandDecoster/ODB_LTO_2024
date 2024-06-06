#include "zoomeventfilter.h"

#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <QDebug>
#include <qmath.h>
#include <QPropertyAnimation>
#include <QTimeLine>

ZoomEventFilter::ZoomEventFilter(QGraphicsView* view)
    : QObject(view), mView(view)
{
    mNumScheduledScalings = 0;
    mView->viewport()->installEventFilter(this);
    mView->setMouseTracking(true);
}

bool ZoomEventFilter::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
        if (wheel_event->orientation() == Qt::Vertical) {

            int numDegrees = wheel_event->delta() / 8;
            int numSteps = numDegrees / 15; // see QWheelEvent documentation
            mNumScheduledScalings += numSteps;
            if (mNumScheduledScalings * numSteps < 0) // if user moved the wheel in another direction, we reset previously scheduled scalings
                mNumScheduledScalings = numSteps;

            mZoomScenePos = mView->mapToScene(wheel_event->pos());
            mViewportPos = wheel_event->pos();

            QTimeLine *anim = new QTimeLine(350, this);
            anim->setUpdateInterval(20);

            connect(anim, &QTimeLine::valueChanged, this, &ZoomEventFilter::scalingTime);
            connect(anim, &QTimeLine::finished, this,  &ZoomEventFilter::animFinished);
            anim->start();
            return true;
        }
    }
    Q_UNUSED(object)
    return false;
}

void ZoomEventFilter::scalingTime(qreal x)
{
    Q_UNUSED(x)
//    qDebug() << mNumScheduledScalings;
    qreal factor = 1.0+ qreal(mNumScheduledScalings) / 300.0;
    mView->scale(factor, factor);

    mView->centerOn(mZoomScenePos);
     QPointF delta_viewport_pos = mViewportPos - QPointF(mView->viewport()->width() / 2.0,
                                                                mView->viewport()->height() / 2.0);
     QPointF viewport_center = mView->mapFromScene(mZoomScenePos) - delta_viewport_pos;
     mView->centerOn(mView->mapToScene(viewport_center.toPoint()));
}

void ZoomEventFilter::animFinished()
{
    if (mNumScheduledScalings > 0)
        mNumScheduledScalings--;
    else
        mNumScheduledScalings++;

    // Delete the sender (the timeline)
    sender()->~QObject();
//    sender()->deleteLater();
}
