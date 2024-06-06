#ifndef VELINESETTINGS_H
#define VELINESETTINGS_H

#include <QWidget>
#include <QComboBox>
#include "data.h"
#include <drawing.h>

class VEPolyline;

namespace Ui {
class VELineSettings;
}

class VEPolyLineSettings : public QWidget
{
    Q_OBJECT
//    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
//    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)

public:
    explicit VEPolyLineSettings(Data * data, QWidget *parent = 0);
    ~VEPolyLineSettings();

    QColor lineColor() const;
    int lineWidth() const;

    Drawing currentDrawing() const;
    void setCurrentDrawing(const Drawing &currentDrawing);

public slots:
    void setLineColor(const QColor &color);
    void setLineWidth(const int &width);
    void comboBoxIndexChanged(int index);

public slots:
//    void newPolyline(VEPolyline *polyline);
    Drawing newPolyline(Drawing drawing);
//    void loadPolyline(VEPolyline *polyline);
    void loadPolyline(Drawing drawing);
    void deselect();
//    void setVisible(bool visible);


signals:
    void lineUpdated(Drawing drawing);
//    void lineColorChanged(int uniqueId, const QColor &color);
//    void lineWidthChanged(const int &width);

private:
    Data * mData;
    QColor mLineColor;
    int mLineWidth;

private:
    Ui::VELineSettings *ui;
//    VEPolyline *currentPolyline;
    Drawing mCurrentDrawing;



};

#endif // VELINESETTINGS_H
