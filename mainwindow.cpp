#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include "ui_mainwindow.h"


#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-ifd.h>
#include <libexif/exif-content.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifndef Q_WS_MAC
    //menuBar()->hide();
#endif

    setWindowFlags(
        Qt::Window | Qt::WindowTitleHint |
        Qt::WindowMinimizeButtonHint |
        Qt::WindowCloseButtonHint |
        Qt::CustomizeWindowHint);

    connect(ui->actionAbout_Twobody, SIGNAL(activated()), this, SLOT(aboutTwobody()));
    connect(ui->action_Add_pictures, SIGNAL(activated()), this, SLOT(addPictures()));
    connect(ui->addPicturesButton, SIGNAL(clicked()), this, SLOT(addPictures()));


}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::aboutTwobody()
{
    QMessageBox::about(this, tr("About Twobody"),
         tr("The <b>Twobody</b> is time synchronize utility for between two DSLRs."));

}
void MainWindow::addPictures()
{

    QStringList filelist =
        QFileDialog::getOpenFileNames(this, tr("Add pictures"), ".", "Image (*.jpg)");
    if(filelist.length()==0)
        return;

    for(int i=0; i<filelist.length(); i++) {
        QString filepath = filelist.at(i);
        QFile f(filepath);

        if(!f.exists())
            continue;

        printf("file %s\n",  filepath.toLocal8Bit().constData());
        ExifData *ed = exif_data_new_from_file (filepath.toLocal8Bit().constData());
        ExifEntry *ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_MODEL);
        if(ee==NULL) {
            exif_data_unref(ed);
            continue;
        }
        char value[256]={0,};
        QString model(exif_entry_get_value(ee, value, sizeof(value)));
        if(!mModelMap.contains(model)) {  
            QStandardItemModel m;
            mModelMap[model] = m;
        }
        QStandardItemModel m = mModelMap[model];

        ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_MODEL);
        printf("Model: %s\n", exif_entry_get_value(ee, value, sizeof(value)));
        //exif_entry_free(ee);
        exif_data_free(ed);

    }

}
