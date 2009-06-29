#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMap>
#include <QStandardItemModel>

namespace Ui
{
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void aboutTwobody();
    void addPictures();

private:
    Ui::MainWindow *ui;
    QMap<QString, QStandardItemModel> mModelMap;



};

#endif // MAINWINDOW_H
