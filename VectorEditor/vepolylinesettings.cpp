#include "vepolylinesettings.h"
#include "ui_vepolylinesettings.h"
#include <QColorDialog>
#include <QBrush>
#include <QPen>
#include <QDebug>
#include "vepolyline.h"

VEPolyLineSettings::VEPolyLineSettings(Data *data, QWidget *parent) :
    QWidget(parent),
    mData(data),
    ui(new Ui::VELineSettings)
{
    ui->setupUi(this);
    setLineColor(QColor(Qt::black));
    setLineWidth(1);

    connect(ui->lineColor, &ColorLabel::clicked,
            [=](){
        QColorDialog dialog;
        connect(&dialog, &QColorDialog::colorSelected, this, &VEPolyLineSettings::setLineColor);
        dialog.exec();
    });

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index){comboBoxIndexChanged(index);});

    connect(ui->lineWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &VEPolyLineSettings::setLineWidth);
}

VEPolyLineSettings::~VEPolyLineSettings()
{
    delete ui;
}

QColor VEPolyLineSettings::lineColor() const
{
    return mLineColor;
}

int VEPolyLineSettings::lineWidth() const
{
    return mLineWidth;
}

//void VEPolyLineSettings::setLineColor(const QColor &color)
//{
//    m_lineColor = color;
//    ui->lineColor->setColor(m_lineColor);
////    if(currentPolyline != nullptr){
////        QPen pen(color,currentPolyline->pen().width());
////        currentPolyline->setPen(pen);
////    }

//    if(currentPolyline != nullptr){
//        Drawing drawing = currentPolyline->toDrawing();
//        drawing.setBorderColor(m_lineColor);
//        emit lineUpdated(drawing);
////        emit lineColorChanged(currentPolyline->uniqueId(), m_lineColor);

//    }
//}

void VEPolyLineSettings::setLineColor(const QColor &color)
{
    mLineColor = color;
    ui->lineColor->setColor(mLineColor);
    //    if(currentPolyline != nullptr){
    //        QPen pen(color,currentPolyline->pen().width());
    //        currentPolyline->setPen(pen);
    //    }

    int uniqueId = mCurrentDrawing.uniqueId();
    if(mData->drawings().contains(uniqueId)){
        mCurrentDrawing = *(mData->drawings()[mCurrentDrawing.uniqueId()]);
        mCurrentDrawing.setBorderColor(mLineColor);
        emit lineUpdated(mCurrentDrawing);
    }

    //        emit lineColorChanged(currentPolyline->uniqueId(), m_lineColor);

}

//void VEPolyLineSettings::setLineWidth(const int &width)
//{
//    m_lineWidth = width;
//    ui->lineWidth->setValue(m_lineWidth);
//    if(currentPolyline != nullptr){
////        QPen pen(currentPolyline->pen().color(),width);
////        currentPolyline->setPen(pen);
//        Drawing drawing = currentPolyline->toDrawing();
//        drawing.setWidth(m_lineWidth);
//        emit lineUpdated(drawing);
//    }
//    //    emit lineWidthChanged(m_lineWidth);
//}

void VEPolyLineSettings::setLineWidth(const int &width)
{
    mLineWidth = width;
    ui->lineWidth->setValue(mLineWidth);
    //    if(currentPolyline != nullptr){
    //        QPen pen(currentPolyline->pen().color(),width);
    //        currentPolyline->setPen(pen);
    //        Drawing drawing = currentPolyline->toDrawing();


    int uniqueId = mCurrentDrawing.uniqueId();
    if(mData->drawings().contains(uniqueId)){
        mCurrentDrawing = *(mData->drawings()[mCurrentDrawing.uniqueId()]);
        mCurrentDrawing.setWidth(mLineWidth);
        emit lineUpdated(mCurrentDrawing);
    }
    //    }
    //    emit lineWidthChanged(m_lineWidth);
}

void VEPolyLineSettings::comboBoxIndexChanged(int index)
{
    switch(index){
    case 1:
        setLineWidth(3);
        setLineColor(Qt::blue);
        break;
    case 2:
        setLineWidth(2);
        setLineColor(Qt::blue);
        break;
    case 3:
        setLineWidth(1);
        setLineColor(Qt::blue);
        break;
    default:
        break;
    }
}

Drawing VEPolyLineSettings::newPolyline(Drawing drawing)
{
    mCurrentDrawing = drawing;
    mCurrentDrawing.setBorderColor(mLineColor);
    mCurrentDrawing.setWidth(mLineWidth);

    return mCurrentDrawing;
    //    polyline->setBrush(QBrush(Qt::transparent));
    //    polyline->setPen(QPen(m_lineColor, m_lineWidth));
}

void VEPolyLineSettings::loadPolyline(Drawing drawing)
{
    mCurrentDrawing = drawing;
    mLineColor = mCurrentDrawing.borderColor();
    mLineWidth = mCurrentDrawing.width();
    ui->lineColor->setColor(mLineColor);
    ui->lineWidth->setValue(mLineWidth);

    ui->comboBox->setCurrentIndex(0);
    if(mLineColor == Qt::blue){
        switch(mLineWidth){
        case 3:
            ui->comboBox->setCurrentIndex(1);
            break;
        case 2:
            ui->comboBox->setCurrentIndex(2);
            break;
        case 1:
            ui->comboBox->setCurrentIndex(3);
            break;
        }
    }
}

//void VEPolyLineSettings::newPolyline(VEPolyline *polyline)
//{
//    polyline->setBrush(QBrush(Qt::transparent));
//    polyline->setPen(QPen(m_lineColor, m_lineWidth));
//}

//void VEPolyLineSettings::loadPolyline(VEPolyline *polyline)
//{
//    currentPolyline = polyline;
//    m_lineColor = currentPolyline->pen().color();
//    m_lineWidth = currentPolyline->pen().width();
//    ui->lineColor->setColor(m_lineColor);
//    ui->lineWidth->setValue(m_lineWidth);
//}

void VEPolyLineSettings::deselect()
{
    mCurrentDrawing = Drawing();
    //    currentPolyline = nullptr;
}

Drawing VEPolyLineSettings::currentDrawing() const
{
    return mCurrentDrawing;
}

void VEPolyLineSettings::setCurrentDrawing(const Drawing &currentDrawing)
{
    mCurrentDrawing = currentDrawing;
}

//void VEPolyLineSettings::setVisible(bool visible)
//{
//    if(!visible && currentPolyline != nullptr){
//        QWidget::setVisible(true);
//    } else {
//        QWidget::setVisible(visible);
//    }
//}


