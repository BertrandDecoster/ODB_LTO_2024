#ifndef SELECTIONRECT_H
#define SELECTIONRECT_H

#include <QObject>
#include <QGraphicsRectItem>

class VESelectionRect : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
        enum { Type = 103 };
public:
    explicit VESelectionRect(QObject *parent = 0);
    ~VESelectionRect();

    int type() const override;

signals:

public slots:
};

#endif // SELECTIONRECT_H
