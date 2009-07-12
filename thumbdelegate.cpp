#include "thumbdelegate.h"
#include <QPainter>
#include <QDebug>

static bool fontInitialized = false;
static QFont deltaFont;
#define FONTSIZE 16
ThumbDelegate::ThumbDelegate()
    : QItemDelegate()
{
    mRight = false;
    qDebug() << "new delegate !!" << endl;
    if(!fontInitialized) {
        deltaFont.setPointSize(FONTSIZE);
        deltaFont.setBold(true);
    }

}
void ThumbDelegate::setRight() {
    mRight = true;
}


void ThumbDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    drawBackground(painter, option, index);
    painter->save();
    //QItemDelegate::paint(painter, option, index);
    QVariant v = index.model()->data(index, Qt::DecorationRole);
    if(v.type()==QVariant::Icon) {
        QPixmap pixmap = qvariant_cast<QIcon>(v).pixmap(160, 160);
        painter->drawPixmap(option.rect.x(), option.rect.y(), pixmap);
    }



    QMap<QString, QVariant> userData = index.model()->data(index, Qt::UserRole).toMap();
    painter->drawText(option.rect.x()+170, option.rect.y()+20,
            index.model()->data(index, Qt::DisplayRole).toString());
    painter->drawText(option.rect.x()+170, option.rect.y()+40, userData["date time"].toString());

    if(mRight && userData.contains("after")) {
        QFont f = painter->font();
        f.setBold(true);
        painter->setFont(f);
        painter->drawText(option.rect.x()+170, option.rect.y()+60, "=>"+userData["after"].toString());
    }

    if(mRight && userData.contains("delta")) {
        QString delta = userData["delta"].toString();
        painter->setFont(deltaFont);
        int x =  option.rect.x()+option.rect.width()-painter->fontMetrics().width(delta);
        int y =  option.rect.y()+option.rect.height()-FONTSIZE;
        painter->setPen(Qt::white);
        painter->drawText(x-1, y-1, delta);
        painter->drawText(x+1, y-1, delta);
        painter->drawText(x-1, y+1, delta);
        painter->drawText(x+1, y+1, delta);
        /*
        if(delta.at(0)=='+')
            painter->setPen(Qt::darkGreen);
        else
            painter->setPen(Qt::darkRed);
            */
        painter->setPen(Qt::darkGreen);
        painter->drawText(x, y, delta);

    }
    painter->restore();
    drawFocus(painter, option, option.rect);
}
QWidget * ThumbDelegate::createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    return NULL;
}
