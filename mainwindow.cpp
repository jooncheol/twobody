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

    ui->leftListView->setIconSize(QSize(160, 160));
    ui->rightListView->setIconSize(QSize(160, 160));

    connect(ui->actionAbout_Twobody, SIGNAL(activated()), this, SLOT(aboutTwobody()));
    connect(ui->action_Add_pictures, SIGNAL(activated()), this, SLOT(addPictures()));
    connect(ui->addPicturesButton, SIGNAL(clicked()), this, SLOT(addPictures()));
    connect(ui->leftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLeftChanged(int)));

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
    if(filelist.count()==0)
        return;

    for(int i=0; i<filelist.count(); i++) {
        QString filepath = filelist.at(i);
        QFile f(filepath);
        QFileInfo fi(filepath);
        if(!fi.exists())
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
            mModelMap[model] = new QStandardItemModel();
        }
        QPixmap pixmap;
        pixmap.loadFromData((const uchar*)ed->data, ed->size, "jpeg");
        qDebug() << "pixmap " << pixmap.width() << "x" << pixmap.height() << endl;

        QIcon icon(pixmap);

        QStandardItem *item = new QStandardItem(icon, fi.completeBaseName()+"<BR>haha\nhoho");
        ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_DATE_TIME);
        exif_entry_get_value(ee, value, sizeof(value));
        qDebug() << "File: " << fi.completeBaseName() << ", Date time: " << value << endl;
        item->setData(QVariant(QString(value)));

        ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_X_RESOLUTION);
        if(ee!=NULL) {
            exif_entry_get_value(ee, value, sizeof(value));
            qDebug() << "X Res: " << value << endl;
        }
        ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_Y_RESOLUTION);
        if(ee!=NULL) {
            exif_entry_get_value(ee, value, sizeof(value));
            qDebug() << "Y Res: " << value << endl;
        }
        exif_data_dump(ed);

        mModelMap[model]->appendRow(item);

        exif_data_unref(ed);
    }

    for(int i=0; i<mModelMap.keys().count(); i++) {
        ui->leftComboBox->addItem(mModelMap.keys()[i]);
    }
    if(ui->leftComboBox->count()>0)
        slotLeftChanged(0);


}

void MainWindow::slotLeftChanged(int index) {
    ui->rightComboBox->clear();
    for(int i=0; i<ui->leftComboBox->count(); i++) {
        if(i==index)
            continue;
        ui->rightComboBox->addItem(ui->leftComboBox->itemText(i));
    }
    ui->leftListView->setModel(mModelMap[ui->leftComboBox->currentText()]);
    if(ui->rightComboBox->count()>0)
        slotRightChanged(0);
}
void MainWindow::slotRightChanged(int index) {
    ui->rightListView->setModel(mModelMap[ui->rightComboBox->currentText()]);
}
