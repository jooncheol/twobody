#include <QtGui/QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString locale = QLocale::system().name();
    qDebug() << "Locale " << locale << endl;
    QTranslator translator;
    translator.load(QString("twobody_") + locale);
    a.installTranslator(&translator);
    MainWindow w;
    w.show();
    return a.exec();
}
