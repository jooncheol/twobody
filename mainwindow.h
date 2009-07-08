#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMap>
#include <QStandardItemModel>
#include <QWidget>

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

private:
    Ui::MainWindow *ui;
    QMap<QString, QStandardItemModel*> mModelMap;

 protected:
     void dragEnterEvent(QDragEnterEvent *event);
     void dropEvent(QDropEvent *event);

};

#endif // MAINWINDOW_H
