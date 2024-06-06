#ifndef VERECTANGLESETTINGS_H
#define VERECTANGLESETTINGS_H

#include <QGraphicsItem>
#include <QWidget>

class VERectangle;
class VEEllipse;
class Data;

namespace Ui {
class VEShapeSettings;
}

class VEShapeSettings : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color_1 READ color_1 WRITE setColor_1 NOTIFY color_1Changed)
    Q_PROPERTY(QColor color_2 READ color_2 WRITE setColor_2 NOTIFY color_2Changed)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)

public:
    explicit VEShapeSettings(Data * data, QWidget *parent = 0);
    ~VEShapeSettings();

    QColor color_1() const;
    QColor color_2() const;
    QColor borderColor() const;
    int borderWidth() const;

public slots:
    void setColor_1(const QColor &color);
    void setColor_2(const QColor &color);
    void setBorderColor(const QColor &color);
    void setBorderWidth(const int &width);

public slots:
//    void newRectangle(VERectangle *rect);
//    void newEllipse(VEEllipse *ellipse);
    void newShape(QAbstractGraphicsShapeItem * shape);
//    void loadRectangle(VERectangle *rect);
//    void loadEllipse(VEEllipse *ellipse);
    void loadShape(QAbstractGraphicsShapeItem * shape);
    void deselect();
    void setVisible(bool visible) override;

private slots:
    void comboBoxIndexChanged(int index);

private:
//    void setGradient(VERectangle *rectangle);
//    void setGradient(VEEllipse *ellipse);
    void setGradient(QAbstractGraphicsShapeItem *shape);

signals:
    void color_1Changed(const QColor &color);
    void color_2Changed(const QColor &color);
    void borderColorChanged(const QColor &color);
    void borderWidthChanged(const int &width);

private:
    Data * mData;
    QColor m_color_1;
    QColor m_color_2;
    QColor m_borderColor;
    int m_borderWidth;

private:
    Ui::VEShapeSettings *ui;
    QAbstractGraphicsShapeItem *currentShape;
//    VERectangle *currentRectangle;
//    VEEllipse *currentEllipse;


};

#endif // VERECTANGLESETTINGS_H
