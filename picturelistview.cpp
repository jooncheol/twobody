#include "picturelistview.h"
#include <QDebug>

PictureListView::PictureListView()
    : QListView(NULL)
{
}
PictureListView::PictureListView(QWidget *parent)
    : QListView(parent)
{
}

void PictureListView::currentChanged ( const QModelIndex & current, const QModelIndex & previous ) {
    QListView::currentChanged(current, previous);
    emit currentIndexChanged(current);
}
