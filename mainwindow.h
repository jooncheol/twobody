#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMap>
#include <QStandardItemModel>
#include <QWidget>
#include <QProgressDialog>
#include <QTimer>
#include "common.h"

namespace Ui
{
    class MainWindow;
}

class QDragEnterEvent;
class QDropEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void addPictures(QStringList fileList);
private slots:
    void aboutTwobody();
    void addPictures();
    void clearPictures();
    void slotLeftChanged(int);
    void slotRightChanged(int);
    void slotPictureIndexChanged(const QModelIndex &);
    void slotSync();
    void slotSyncTimer();
    void slotSyncCanceled();
    void slotAddTimer();

private:
    Ui::MainWindow *ui;
    QMap<QString, QStandardItemModel*> mModelMap;
    QProgressDialog *mPD;
    QTimer          *mTimer;
    int             mProgress ;
    QTimer          *mAddTimer;
    int             mAddedNum ;
    int             mSkippedNum ;
    QStringList         mFileList;

 protected:
     void dragEnterEvent(QDragEnterEvent *event);
     void dropEvent(QDropEvent *event);

};

#endif // MAINWINDOW_H
