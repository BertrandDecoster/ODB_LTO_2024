#include "treedelegate.h"

#include <QFile>
#include <QPainter>
#include <QDebug>
#include <QPushButton>
#include <QIcon>
#include <QApplication>
#include <QPixmap>

#include "data.h"

double App6Icon::iconSize = 128;
int TreeDelegate::iconSize = 128;
//QMap<QString, QPixmap *> TreeDelegate::icons = QMap<QString, QPixmap *>();


App6Icon::App6Icon(QString app6):
    pixmapRatio(1.0),
    app6String(app6)
{
    QString filepath = ":/files/app6/";
    filepath += app6String;
    filepath += ".svg";
    QFile file{filepath};
    if (!file.open(QIODevice::ReadOnly)) {
        filepath = ":/files/app6/SFGPU------" + app6.back() +  ".svg";
    }
    image = QImage(filepath);
    //    qDebug() << "IMAGE SIZE " << image.size();
    pixmapRatio = (1.0*image.width())/(1.0*image.height());
    pixmap = QIcon(filepath).pixmap(sizeHint());

    //    QRect pixmapRect = pixmap.rect();
    //    pixmapRatio = (1.0 *pixmapRect.width())/(1.0*pixmapRect.height());
    //    lastRectGiven = QRect(0, 0, App6Icon::iconSize*pixmapRatio, App6Icon::iconSize);
    //    qDebug() << "Pixmap " << pixmapRect << " " << QString::number(pixmapRatio);
}

void App6Icon::paint(QPainter *painter, const QRect &rect, const QPalette &palette) const
{
    Q_UNUSED(palette)


    //    painter->setPen(Qt::NoPen);

    // If there is just no room
    if(rect.width()<=0){
        return;
    }
    QRect pixmapRect = pixmap.rect();
    if(pixmapRect.height() == 0 || pixmapRect.width() == 0){
        return;
    }

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    // Keep the ratio
    QSize size;
    if(1.0*rect.width() > rect.height()*pixmapRatio){
        size.setWidth(rect.height()*pixmapRatio);
        size.setHeight(rect.height());
    }
    else{
        size.setWidth(rect.width());
        size.setHeight(rect.width()/pixmapRatio);
    }
    //    int side = (int)(qMin(1.0*rect.width(), rect.height()*pixmapRatio));

    painter->drawPixmap(QRect(rect.topLeft(), size), pixmap, pixmap.rect());
    painter->restore();
}

QSize App6Icon::sizeHint() const
{

    int delta = app6String.back().toLatin1() - 'A';
    double factor = qBound(0.6, (1.0+delta)/10.0, 1.0);

    if(app6String.back() == '-'){
        factor = 0.6;
    }


    //    qDebug() << "Sizehint " << QString::number(factor) << " " << app6String;
    return  QSize(App6Icon::iconSize * factor * pixmapRatio, App6Icon::iconSize * factor);
    //        return QSize(App6Icon::iconSize, App6Icon::iconSize);
}



/////////////////////////////////////////////////////////////////
/// TREE DELEGATE
/////////////////////////////////////////////////////////////////

TreeDelegate::TreeDelegate(Data *data, QWidget *parent):
    QStyledItemDelegate (parent),
    mData(data)
{
    //    if(TreeDelegate::icons.size() == 0){
    //        TreeDelegate::icons = initIcons();
    //    }
}


//QMap<QString, QPixmap*> TreeDelegate::initIcons()
//{
//    QMap<QString, QPixmap*> answer;
//    answer[QStringLiteral("add")] = new QPixmap(":/files/icon/plus.png");
//    answer[QStringLiteral("delete")] = new QPixmap(":/files/icon/bin.png");
//    return answer;
//}


void TreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == 0){
        //        paintUnit(painter, option, index);
        int uniqueId = index.data(Qt::DisplayRole).toInt();
        Unit * unit = mData->units()[uniqueId];
        if(unit){
            QIcon icon = unit->getIcon();
            icon.paint(painter, option.rect);

        }
        else{
            QStyledItemDelegate::paint(painter, option, index);
        }
        //        if(index.data(Qt::UserRole+1).canConvert<App6Icon>()){
        //            const App6Icon & app6Icon = qvariant_cast<App6Icon>(index.data(Qt::UserRole+1));
        //            app6Icon.paint(painter, option.rect, option.palette);
        //            return;
        //        }
    }
    else{
        QStyledItemDelegate::paint(painter, option, index);
    }
}

//void TreeDelegate::paintUnit(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    if(index.data(Qt::UserRole+1).canConvert<App6Icon>()){
//        const App6Icon & app6Icon = qvariant_cast<App6Icon>(index.data(Qt::UserRole+1));
//        app6Icon.paint(painter, option.rect, option.palette);
//        return;
//    }

//}

//QSize TreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    if(index.column() == 0){
//        if(index.data(Qt::UserRole+1).canConvert<App6Icon>()){
//            App6Icon app6Icon = qvariant_cast<App6Icon>(index.data(Qt::UserRole+1));
//            return app6Icon.sizeHint();
//        }


//        int uniqueId = index.data(Qt::DisplayRole).toString();
//        Unit * unit = mData->units()[uniqueId];
//        int delta = unit->app6().back().toLatin1() - 'A';
//        double factor = qBound(0.6, (1.0+delta)/10.0, 1.0);
//        if(unit->app6().back() == '-'){
//            factor = 0.6;
//        }
//        QIcon icon = unit->getIcon();
//        return icon.actualSize(QSize(TreeDelegate::iconSize * factor, TreeDelegate::iconSize * factor));

//        //    qDebug() << "Sizehint " << QString::number(factor) << " " << app6String;
//    }
//    return QStyledItemDelegate::sizeHint(option, index);
//}

QSize TreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == 0){
                int uniqueId = index.data(Qt::DisplayRole).toInt();
                Unit * unit = mData->units()[uniqueId];
                if(unit){
                    int delta = unit->app6().back().toLatin1() - 'A';
                    double factor = qBound(0.6, (1.0+delta)/10.0, 1.0);
                    if(unit->app6().back() == '-'){
                        factor = 0.6;
                    }
                    return QSize(Unit::iconTotalWidth*factor/2, Unit::iconTotalHeight*factor/2);
                }
        //        QIcon icon = unit->getIcon();
        //        return icon.actualSize(QSize(TreeDelegate::iconSize * factor, TreeDelegate::iconSize * factor));
        //        //    qDebug() << "Sizehint " << QString::number(factor) << " " << app6String;
    }
    return QStyledItemDelegate::sizeHint(option, index);
}



