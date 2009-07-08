#ifndef PICTURELISTVIEW_H
#define PICTURELISTVIEW_H

#include <QListView>

class PictureListView : public QListView
{
    Q_OBJECT
public:
    PictureListView();
    PictureListView(QWidget *parent=0);
protected:
    void currentChanged ( const QModelIndex & current, const QModelIndex & previous );
signals:
    void currentIndexChanged(const QModelIndex &mi);


};

#endif // PICTURELISTVIEW_H
