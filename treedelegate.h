#ifndef TREEDELEGATE_H
#define TREEDELEGATE_H

#include <QStyledItemDelegate>

class Data;
QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

class App6Icon{
public:
    App6Icon(QString app6 = QStringLiteral(""));
    void paint(QPainter *painter, const QRect &rect,
               const QPalette &palette) const;

    QSize sizeHint() const;

  private:
    double pixmapRatio;
    QString app6String;
    QPixmap pixmap;
    QImage image;

public:
    static double iconSize;
};

Q_DECLARE_METATYPE(App6Icon)


class TreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TreeDelegate(Data * data, QWidget *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

//    void paintUnit(QPainter *painter, const QStyleOptionViewItem &option,
//               const QModelIndex &index) const;

public:
//    static QMap<QString, QPixmap*> initIcons();
public:
//    static QMap<QString, QPixmap*> icons;
    static int iconSize;

private:
    Data * mData;
};



#endif // TREEDELEGATE_H
