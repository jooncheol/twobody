#ifndef THUMBDELEGATE_H
#define THUMBDELEGATE_H

#include <QItemDelegate>

class ThumbDelegate : public QItemDelegate
{
public:
    ThumbDelegate();
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QWidget * createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    void setRight();
private:
    bool mRight;
};

#endif // THUMBDELEGATE_H
