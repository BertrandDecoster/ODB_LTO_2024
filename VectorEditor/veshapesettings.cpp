#include "veshapesettings.h"
#include "ui_veshapesettings.h"
#include <QColorDialog>
#include <QBrush>
#include <QPen>
#include <QDebug>
#include <QAbstractGraphicsShapeItem>

#include "verectangle.h"
#include "veellipse.h"
#include "data.h"

VEShapeSettings::VEShapeSettings(Data *data, QWidget *parent) :
    QWidget(parent),
    mData(data),
    ui(new Ui::VEShapeSettings),
    currentShape(nullptr)
{
    ui->setupUi(this);
    ui->color_2->setVisible(false);
    ui->labelColor_2->setVisible(false);

    QColor color(0, 0, 255, 48);
    setColor_1(color);
    setColor_2(Qt::white);
    setBorderColor(Qt::black);
    setBorderWidth(1);

    connect(ui->color_1, &ColorLabel::clicked,
            [=](){
        QColorDialog dialog;
        dialog.setOption(QColorDialog::ShowAlphaChannel);
        dialog.setCurrentColor(ui->color_1->color());
        connect(&dialog, &QColorDialog::colorSelected, this, &VEShapeSettings::setColor_1);
        dialog.exec();
    });
    connect(ui->color_2, &ColorLabel::clicked,
            [=](){
        QColorDialog dialog;
        dialog.setOption(QColorDialog::ShowAlphaChannel);
        connect(&dialog, &QColorDialog::colorSelected, this, &VEShapeSettings::setColor_2);
        dialog.exec();
    });
    connect(ui->borderColor, &ColorLabel::clicked,
            [=](){
        QColorDialog dialog;
        connect(&dialog, &QColorDialog::colorSelected, this, &VEShapeSettings::setBorderColor);
        dialog.exec();
    });

    connect(ui->comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &VEShapeSettings::comboBoxIndexChanged);
    connect(ui->borderWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &VEShapeSettings::setBorderWidth);

    ui->comboBox->setDisabled(true);
}

VEShapeSettings::~VEShapeSettings()
{
    delete ui;
}

QColor VEShapeSettings::color_1() const
{
    return m_color_1;
}

QColor VEShapeSettings::color_2() const
{
    return m_color_2;
}

QColor VEShapeSettings::borderColor() const
{
    return m_borderColor;
}

int VEShapeSettings::borderWidth() const
{
    return m_borderWidth;
}

void VEShapeSettings::setColor_1(const QColor &color)
{
    m_color_1 = color;
    ui->color_1->setColor(color);
    if(currentShape != nullptr){
        if(ui->comboBox->currentIndex() == 0){
            currentShape->setBrush(QBrush(m_color_1));
        } else {
             setGradient(currentShape);
        }
    }

    emit color_1Changed(m_color_1);
}

void VEShapeSettings::setColor_2(const QColor &color)
{
    m_color_2 = color;
    ui->color_2->setColor(color);
    if(currentShape != nullptr){
         setGradient(currentShape);
    }
    emit color_2Changed(m_color_2);
}

void VEShapeSettings::setBorderColor(const QColor &color)
{
    m_borderColor = color;
    ui->borderColor->setColor(color);
    if(currentShape != nullptr){
        QPen pen(color,currentShape->pen().width());
        currentShape->setPen(pen);
    }
    emit borderColorChanged(m_borderColor);
}

void VEShapeSettings::setBorderWidth(const int &width)
{
    m_borderWidth = width;
    if(currentShape != nullptr){
        if(width == 0){
            currentShape->setPen(Qt::NoPen);
        } else {
            QPen pen(currentShape->pen().color(), width);
            currentShape->setPen(pen);
        }
    }
    emit borderWidthChanged(m_borderWidth);
}

//void VERectangleSettings::newRectangle(VERectangle *rect)
//{
//    if(ui->comboBox->currentIndex() == 0){
//        rect->setBrush(QBrush(m_color_1));
//    } else {
//        setGradient(rect);
//    }
//    if(m_borderWidth == 0){
//        rect->setPen(Qt::NoPen);
//    } else {
//        rect->setPen(QPen(m_borderColor, m_borderWidth));
//    }
//}

//void VERectangleSettings::newEllipse(VEEllipse *ellipse)
//{
//    if(ui->comboBox->currentIndex() == 0){
//        ellipse->setBrush(QBrush(m_color_1));
//    } else {
//        setGradient(ellipse);
//    }
//    if(m_borderWidth == 0){
//        ellipse->setPen(Qt::NoPen);
//    } else {
//        ellipse->setPen(QPen(m_borderColor, m_borderWidth));
//    }
//}

void VEShapeSettings::newShape(QAbstractGraphicsShapeItem *shape)
{
    if(ui->comboBox->currentIndex() == 0){
        shape->setBrush(QBrush(m_color_1));
    } else {
        setGradient(shape);
    }
    if(m_borderWidth == 0){
        shape->setPen(Qt::NoPen);
    } else {
        shape->setPen(QPen(m_borderColor, m_borderWidth));
    }
}

//void VERectangleSettings::loadRectangle(VERectangle *rect)
//{
//    currentRectangle = rect;

//    m_color_1 = currentRectangle->brush().color();
//    m_borderColor = currentRectangle->pen().color();
//    if(currentRectangle->pen().style() == Qt::NoPen){
//        m_borderWidth = 0;
//    } else {
//        m_borderWidth = currentRectangle->pen().width();
//    }
//    ui->color_1->setColor(m_color_1);
//    ui->borderColor->setColor(m_borderColor);
//    ui->borderWidth->setValue(m_borderWidth);
//}

//void VERectangleSettings::loadEllipse(VEEllipse * ellipse)
//{
//    currentEllipse = ellipse;

//    m_color_1 = currentEllipse->brush().color();
//    m_borderColor = currentEllipse->pen().color();
//    if(currentEllipse->pen().style() == Qt::NoPen){
//        m_borderWidth = 0;
//    } else {
//        m_borderWidth = currentEllipse->pen().width();
//    }
//    ui->color_1->setColor(m_color_1);
//    ui->borderColor->setColor(m_borderColor);
//    ui->borderWidth->setValue(m_borderWidth);
//}

void VEShapeSettings::loadShape(QAbstractGraphicsShapeItem *shape)
{
    currentShape = shape;

    m_color_1 = currentShape->brush().color();
    m_borderColor = currentShape->pen().color();
    if(currentShape->pen().style() == Qt::NoPen){
        m_borderWidth = 0;
    } else {
        m_borderWidth = currentShape->pen().width();
    }
    ui->color_1->setColor(m_color_1);
    ui->borderColor->setColor(m_borderColor);
    ui->borderWidth->setValue(m_borderWidth);
}

void VEShapeSettings::deselect()
{
    currentShape = nullptr;
}

void VEShapeSettings::setVisible(bool visible)
{
    if(!visible && currentShape != nullptr){
        QWidget::setVisible(true);
    } else {
        QWidget::setVisible(visible);
    }
}

void VEShapeSettings::comboBoxIndexChanged(int index)
{
    switch (index) {
    case 1:
        ui->color_2->setVisible(true);
        ui->labelColor_2->setVisible(true);
        ui->labelColor_1->setText(trUtf8("color 1"));
        if(currentShape != nullptr){
            setGradient(currentShape);
        }
        break;
    default:
        ui->color_2->setVisible(false);
        ui->labelColor_2->setVisible(false);
        ui->labelColor_1->setText(trUtf8("background"));
        if(currentShape != nullptr){
            currentShape->setBrush(QBrush(m_color_1));
        }
        break;
    }
}

//void VERectangleSettings::setGradient(VERectangle *rectangle)
//{
//    auto tmpRect = rectangle->rect();
//    QLinearGradient gradient(tmpRect.left() + tmpRect.width()/2,
//                             tmpRect.top(),
//                             tmpRect.left() + tmpRect.width()/2,
//                             tmpRect.bottom());
//    gradient.setColorAt(0.0, m_color_1);
//    gradient.setColorAt(1.0, m_color_2);
//    rectangle->setBrush(QBrush(gradient));
//}

//void VERectangleSettings::setGradient(VEEllipse *ellipse)
//{
//    auto tmpRect = ellipse->rect();
//    QLinearGradient gradient(tmpRect.left() + tmpRect.width()/2,
//                             tmpRect.top(),
//                             tmpRect.left() + tmpRect.width()/2,
//                             tmpRect.bottom());
//    gradient.setColorAt(0.0, m_color_1);
//    gradient.setColorAt(1.0, m_color_2);
//    ellipse->setBrush(QBrush(gradient));
//}

void VEShapeSettings::setGradient(QAbstractGraphicsShapeItem * shape)
{
    QRectF tmpRect;
    switch(shape->type()){
    case VERectangle::Type:
        tmpRect = qgraphicsitem_cast<VERectangle *>(shape)->rect();
        break;
    case VEEllipse::Type:
        tmpRect = qgraphicsitem_cast<VEEllipse *>(shape)->rect();
        break;
    default:
        return;
    }
    QLinearGradient gradient(tmpRect.left() + tmpRect.width()/2,
                             tmpRect.top(),
                             tmpRect.left() + tmpRect.width()/2,
                             tmpRect.bottom());
    gradient.setColorAt(0.0, m_color_1);
    gradient.setColorAt(1.0, m_color_2);
    shape->setBrush(QBrush(gradient));
}
