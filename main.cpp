#include <QtGui/QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include "mainwindow.h"
#include "common.h"
#include <unistd.h>
#include <stdlib.h>
#include <QtPlugin>

#ifdef Q_WS_MAC
#include <Carbon/Carbon.h>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(TITLE);
    a.setApplicationVersion(VERSION);
    QString locale = QLocale::system().name();
    QTranslator translator;

#ifdef Q_WS_WIN
    char cwd[256];
    getcwd(cwd, 256);
    qDebug() << "cwd: " << cwd;

    QStringList lp = a.libraryPaths();
    lp << QString(cwd);
    a.setLibraryPaths(lp);
#endif

#ifdef Q_WS_X11
    Q_IMPORT_PLUGIN(qjpeg)
#endif


#ifdef Q_WS_MAC
	CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef,
												  kCFURLPOSIXPathStyle);
	const char *pathPtr = CFStringGetCStringPtr(macPath,
												CFStringGetSystemEncoding());
	CFRelease(appUrlRef);
	CFRelease(macPath);
#endif
	
#ifdef Q_WS_MAC
	QDir dir(pathPtr);
	dir.cd("Contents");
	dir.cd("Resources");
	translator.load(QString("twobody_%1").arg(locale), dir.absolutePath());
#else
    translator.load(QString("twobody_%1").arg(locale));
#endif
	
	a.installTranslator(&translator);
    MainWindow w;
    w.show();
    return a.exec();
}
