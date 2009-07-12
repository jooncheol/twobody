#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QMap>
#include <QtGui>
#include <Qtimer>
#include "ui_mainwindow.h"
#include "thumbdelegate.h"


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
    mTimer = new QTimer(this);

    setWindowFlags(
        Qt::Window | Qt::WindowTitleHint |
        Qt::WindowMinimizeButtonHint |
        Qt::WindowCloseButtonHint |
        Qt::CustomizeWindowHint);

    ui->leftListView->setIconSize(QSize(160, 120));
    ui->rightListView->setIconSize(QSize(160, 120));
    //ui->logEdit->hide();

    setWindowTitle(tr(TITLE)+" "+VERSION);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(slotSyncTimer()));
    connect(ui->actionAbout_Twobody, SIGNAL(activated()), this, SLOT(aboutTwobody()));
    connect(ui->action_Add_pictures, SIGNAL(activated()), this, SLOT(addPictures()));
    connect(ui->actionClear, SIGNAL(activated()), this, SLOT(clearPictures()));
    connect(ui->addPicturesButton, SIGNAL(clicked()), this, SLOT(addPictures()));
    connect(ui->syncButton, SIGNAL(clicked()), this, SLOT(slotSync()));
    connect(ui->leftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLeftChanged(int)));
    connect(ui->rightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRightChanged(int)));
    connect(ui->leftListView, SIGNAL(clicked ( const QModelIndex & )), this, SLOT(slotPictureIndexChanged(const QModelIndex &)));
    connect(ui->rightListView, SIGNAL(clicked ( const QModelIndex & )), this, SLOT(slotPictureIndexChanged(const QModelIndex &)));
    connect(ui->leftListView, SIGNAL(currentIndexChanged ( const QModelIndex & )), this, SLOT(slotPictureIndexChanged(const QModelIndex &)));
    connect(ui->rightListView, SIGNAL(currentIndexChanged ( const QModelIndex & )), this, SLOT(slotPictureIndexChanged(const QModelIndex &)));
    ui->leftListView->setItemDelegate(new ThumbDelegate());
    ThumbDelegate *d = new ThumbDelegate();
    d->setRight();
    ui->rightListView->setItemDelegate(d);


    statusBar()->showMessage(tr("Ready"));

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::aboutTwobody()
{
    QMessageBox::about(this, tr("About %1 %2").arg(qApp->applicationName()).arg(qApp->applicationVersion()),
         tr("The <b>Twobody</b> is time synchronize utility for between two DSLRs.")+"<br/><br/>"+
         tr("Version:")+" " + qApp->applicationVersion()+ "<br/>"+
         tr("Homepage:")+" " HOMEPAGE "<br/><br/>"+
         tr("Copyright (C) 2009 Jooncheol Park All rights reserved"));

}
void MainWindow::clearPictures()
{
    disconnect(ui->leftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLeftChanged(int)));
    disconnect(ui->rightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRightChanged(int)));
    ui->leftListView->setModel(NULL);
    ui->rightListView->setModel(NULL);
    ui->leftComboBox->clear();
    ui->rightComboBox->clear();
    qDebug() << "model map clean :" << mModelMap.count() << endl;
    for(int i=(mModelMap.keys().count()-1); i>=0;i--) {
        qDebug() << "clear " << i << endl;
        QString key = mModelMap.keys().at(i);
        mModelMap[key]->clear();
        delete mModelMap[key];
    }
    mModelMap.clear();
    qDebug() << "model map clean :" << mModelMap.count() << endl;
    ui->leftComboBox->clear();
    ui->rightComboBox->clear();
    connect(ui->leftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLeftChanged(int)));
    connect(ui->rightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRightChanged(int)));
}

void MainWindow::addPictures()
{
    QStringList filelist =
        QFileDialog::getOpenFileNames(this, tr("Add pictures"), ".", tr("Image files ")
#ifdef Q_WS_WIN
                +"(*.jpg)"
#else
                +"(*.jpg *.JPG)"
#endif
                );
    addPictures(filelist);
}
void MainWindow::addPictures(QStringList filelist)
{
    setCursor(Qt::WaitCursor);
    if(filelist.count()==0) {
        setCursor(QCursor());
        return;
    }

    int addedNum = 0;
    int skippedNum = 0;
    for(int i=0; i<filelist.count(); i++) {
        QString filepath = filelist.at(i);
        QFile f(filepath);
        QFileInfo fi(filepath);
        if(!fi.exists() || !fi.isFile() || fi.suffix().toLower()!="jpg")
            continue;

        printf("file %s\n",  filepath.toLocal8Bit().constData());
        ExifData *ed = exif_data_new_from_file (filepath.toLocal8Bit().constData());
        ExifEntry *ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_MODEL);
        if(ee==NULL) {
            exif_data_unref(ed);
            ++skippedNum;
            continue;
        }
        char value[256]={0,};
        QString _model(exif_entry_get_value(ee, value, sizeof(value)));
        QString model = _model.trimmed();
        if(!mModelMap.contains(model)) {  
            mModelMap[model] = new QStandardItemModel();
            mModelMap[model]->setColumnCount(1);
            qDebug() << "new model: " << model << endl;
            qDebug() << "model count:" << mModelMap.keys().count() << endl;
        }
        /*
        qDebug() << "row count: " << mModelMap[model]->invisibleRootItem()->rowCount();
        for(int j=0; j<mModelMap[model]->invisibleRootItem()->rowCount(); j++)
            qDebug() << "j: " << j << " " <<mModelMap[model]->invisibleRootItem()->child(j, 1)->text();
            */

        if(mModelMap[model]->findItems(filepath, Qt::MatchExactly, 1).count()>0) {
            qDebug() << "already existed: " << filepath << endl;
            ++skippedNum;
            continue;
        }


        memset(value, 0, sizeof(value));
        ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_DATE_TIME);
        exif_entry_get_value(ee, value, sizeof(value));
        QString datetime(value);

        ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
        QString orient("top - left");
        if(ee!=NULL) {
            memset(value, 0, sizeof(value));
            exif_entry_get_value(ee, value, sizeof(value));
            //qDebug() << "Orientation: " << value << endl;
            orient = value;
        }

        QStandardItem *item = new QStandardItem(fi.fileName());
        item->setColumnCount(3);
        QDateTime dt = QDateTime::fromString(datetime, "yyyy:MM:dd HH:mm:ss");
        QMap<QString, QVariant> userData;
        userData["date time"] = QVariant(dt);
        userData["file path"] = QVariant(filepath);
        item->setData(QVariant(userData), Qt::UserRole);


        int rotate;
        if(orient == "top - left")
            rotate = 0;
        else if(orient == "left - bottom")
            rotate = -90;
        else if(orient == "right - top")
            rotate = 90;
        else
            rotate = 180;
        if(rotate==0 || rotate==180) {
            QPixmap pixmap;
            pixmap.loadFromData((const uchar*)ed->data, ed->size, "jpeg");
            if(rotate==0)
                item->setIcon(pixmap);
            else {
                QTransform tf;
                tf.rotate(180);
                item->setIcon(pixmap.transformed(tf));
            }
        } else {
            QImage image;
            image.loadFromData((const uchar*)ed->data, ed->size, "jpeg");
            QTransform tf = QTransform::fromScale(0.75, 0.75);
            tf.rotate(rotate);
            QImage timage = image.transformed(tf);
            QPixmap pixmap(160, 120);
            pixmap.fill(Qt::black);
            QPainter p(&pixmap);
            p.drawImage(35, 0, timage);
            item->setIcon(pixmap);
        }

        //exif_data_dump(ed);

        mModelMap[model]->appendRow(item);
        int lastrow = mModelMap[model]->rowCount()-1;
        mModelMap[model]->setItem(lastrow, 1, new QStandardItem (datetime));

        exif_data_unref(ed);

        ++addedNum;
    }

    qDebug() << "model count:" << mModelMap.keys().count() << endl;

    for(int i=0; i<mModelMap.keys().count(); i++) {
        qDebug() << "model " << i << ": " <<  mModelMap[mModelMap.keys()[i]]->rowCount() << endl;
        mModelMap[mModelMap.keys()[i]]->sort(1);
        if(ui->leftComboBox->findText(mModelMap.keys()[i])<0)
            ui->leftComboBox->addItem(mModelMap.keys()[i]);
    }
    qDebug() << "model end" << endl;
    qDebug() << "1 model count:" << mModelMap.keys().count() << endl;
    if(ui->leftComboBox->count()>0)
        slotLeftChanged(0);
    qDebug() << "2 model count:" << mModelMap.keys().count() << endl;

    QString message;
    if(skippedNum==0)
        message = tr("%1 files added").arg(addedNum);
    else
        message = tr("%1 files added, %2 files skipped").arg(addedNum).arg(skippedNum);
    qDebug() << message << endl;
    qDebug() << "model count:" << mModelMap.keys().count() << endl;
    this->statusBar()->showMessage(message);
    
    setCursor(QCursor());
}

void MainWindow::slotLeftChanged(int index) {
   ui->rightComboBox->clear();
   for(int i=0; i<ui->leftComboBox->count(); i++) {
       if(i==index)
           continue;
       ui->rightComboBox->addItem(ui->leftComboBox->itemText(i));
   }

   QStandardItemModel *model = mModelMap[ui->leftComboBox->currentText()];

   ui->leftListView->setModel(model);
   if(ui->rightComboBox->count()>0)
       slotRightChanged(0);

    ui->syncButton->setEnabled(
        ui->leftListView->currentIndex().row()>=0 && ui->rightListView->currentIndex().row()>=0);
    ui->actionTime_synchronize->setEnabled(
        ui->leftListView->currentIndex().row()>=0 && ui->rightListView->currentIndex().row()>=0);
}
void MainWindow::slotRightChanged(int index) {
    if(index>=0)
        ui->rightListView->setModel(mModelMap[ui->rightComboBox->currentText()]);
    ui->syncButton->setEnabled(
        ui->leftListView->currentIndex().row()>=0 && ui->rightListView->currentIndex().row()>=0);
    ui->actionTime_synchronize->setEnabled(
        ui->leftListView->currentIndex().row()>=0 && ui->rightListView->currentIndex().row()>=0);
}
void MainWindow::slotPictureIndexChanged(const QModelIndex &mi) {
    if(ui->leftListView->currentIndex().row()>=0 && ui->rightListView->currentIndex().row()>=0) {
        ui->syncButton->setEnabled(true);
        ui->actionTime_synchronize->setEnabled(true);
        QStandardItemModel *model = mModelMap[ui->leftComboBox->currentText()];
        QStandardItem *item = model->item(ui->leftListView->currentIndex().row());
        QMap<QString, QVariant> userData = item->data(Qt::UserRole).toMap();
        QDateTime baseTime = userData["date time"].toDateTime();
        qDebug() << "Base DT: " << baseTime.toString("yyyy:MM:dd HH:mm:ss") << endl;

        model = mModelMap[ui->rightComboBox->currentText()];
        QStandardItem *ritem = model->item(ui->rightListView->currentIndex().row());
        QMap<QString, QVariant> rUserData = ritem->data(Qt::UserRole).toMap();
        QDateTime dt = rUserData["date time"].toDateTime();
        int delta =  dt.secsTo(baseTime);

        for(int i=0; i<model->rowCount(); i++) {
            QStandardItem *item = model->item(i);
            QMap<QString, QVariant> userData = item->data(Qt::UserRole).toMap();
            QDateTime dt = userData["date time"].toDateTime();
            QDateTime deltaDateTime =  dt.addSecs(delta);
            userData["after"] = QVariant(deltaDateTime.toString("yyyy:MM:dd HH:mm:ss"));
            int secs =  dt.secsTo(deltaDateTime);
            QString secTo="+";
            if(secs<0)
                secTo = "-";
            secs = abs(secs);
            if(secs>(86400*365)) {
                secTo += " "+tr("%1 year").arg(secs/(86400*365));
                secs = secs%(86400*365);
            }
            if(secs>(86400*30)) {
                secTo += " "+tr("%1 month").arg(secs/(86400*30));
                secs = secs%(86400*30);
            }
            if(secs>86400) {
                secTo += " "+tr("%1 day").arg(secs/(86400));
                secs = secs%(86400);
            }
            int hour, min;
            hour = secs/3600;
            secs = secs%(3600);
            min = secs/60;
            secs = secs%(60);
            char interval[32] = {0, };
            sprintf(interval, " %02d:%02d:%02d", hour, min, secs);
            secTo += interval;
            userData["delta"] = QVariant(secTo);
            item->setData(QVariant(userData), Qt::UserRole);
        }
    } else {
        ui->syncButton->setEnabled(false);
        ui->actionTime_synchronize->setEnabled(false);
   }
}


void MainWindow::dragEnterEvent ( QDragEnterEvent * event ) {
    qDebug() << __FUNCTION__ << endl;
    qDebug() << event->mimeData()->formats() << endl;
    if (event->mimeData()->hasFormat("text/uri-list"))
         event->acceptProposedAction();
}
void MainWindow::dropEvent(QDropEvent *event) {
    qDebug() << __FUNCTION__ << endl;
    qDebug() << event->mimeData()->text() << endl;

    QStringList fileList;
    if (event->mimeData()->hasUrls()) {
        foreach (QUrl url, event->mimeData()->urls()) {
            fileList << url.toLocalFile();
            qDebug() << url.toLocalFile() << endl;
        }
    }
    addPictures(fileList);
    event->acceptProposedAction();
}


void MainWindow::slotSync() {
    QStandardItemModel *model = mModelMap[ui->rightComboBox->currentText()];
    mPD = new QProgressDialog(this);
    connect(mPD, SIGNAL(canceled()), this, SLOT(slotSyncCanceled()));
    mPD->setMaximum(model->rowCount()-1);
    mPD->setWindowTitle(tr("Synchronize"));
    mPD->setModal(true);
    mProgress = 0;
    mTimer->start(100);
}
void MainWindow::slotSyncCanceled() {
    mTimer->stop();
    if(mPD!=NULL) {
        mPD->close();
        mPD = NULL;
    }
}
void MainWindow::slotSyncTimer() {
    QStandardItemModel *model = mModelMap[ui->rightComboBox->currentText()];
    QStandardItem *item = model->item(mProgress);
    QMap<QString, QVariant> userData = item->data(Qt::UserRole).toMap();
    QString filePath = userData["file path"].toString();
    mPD->setLabelText(filePath);
    mPD->show();
    mPD->setValue(mProgress++);
    if(mProgress<model->rowCount())
        mTimer->start(100);
    else {
        mPD->close();
        delete mPD;
        mPD = NULL;
   }
}
