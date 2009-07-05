#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QtGui>
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

    ui->leftListView->setIconSize(QSize(160, 120));
    ui->rightListView->setIconSize(QSize(160, 120));
    //ui->logEdit->hide();

    connect(ui->actionAbout_Twobody, SIGNAL(activated()), this, SLOT(aboutTwobody()));
    connect(ui->action_Add_pictures, SIGNAL(activated()), this, SLOT(addPictures()));
    connect(ui->actionClear, SIGNAL(activated()), this, SLOT(clearPictures()));
    connect(ui->addPicturesButton, SIGNAL(clicked()), this, SLOT(addPictures()));
    connect(ui->leftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLeftChanged(int)));
    connect(ui->rightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRightChanged(int)));

    statusBar()->showMessage(tr("Ready"));

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
void MainWindow::clearPictures()
{
    disconnect(ui->leftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLeftChanged(int)));
    disconnect(ui->rightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRightChanged(int)));
    ui->leftListView->setModel(NULL);
    ui->rightListView->setModel(NULL);
    ui->leftComboBox->clear();
    ui->rightComboBox->clear();
    for(int i=0; i<mModelMap.keys().count();i++) {
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
        QFileDialog::getOpenFileNames(this, tr("Add pictures"), ".", "Image files (*.jpg)");
    addPictures(filelist);
}
void MainWindow::addPictures(QStringList filelist)
{
    if(filelist.count()==0)
        return;

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
            mModelMap[model]->setColumnCount(2);
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


        ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_DATE_TIME);
        exif_entry_get_value(ee, value, sizeof(value));
        QString datetime(value);

        ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
        QString orient("top - left");
        if(ee!=NULL) {
            exif_entry_get_value(ee, value, sizeof(value));
            //qDebug() << "Orientation: " << value << endl;
            orient = value;
        }

        QStandardItem *item = new QStandardItem(fi.fileName()+"\n"+datetime);
        item->setColumnCount(3);

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
        mModelMap[model]->setItem(lastrow, 1, new QStandardItem (filepath));
        mModelMap[model]->setItem(lastrow, 2, new QStandardItem (datetime));

        exif_data_unref(ed);

        ++addedNum;
    }

    qDebug() << "model count:" << mModelMap.keys().count() << endl;

    for(int i=0; i<mModelMap.keys().count(); i++) {
        qDebug() << "model " << i << ": " <<  mModelMap[mModelMap.keys()[i]]->rowCount() << endl;
        mModelMap[mModelMap.keys()[i]]->sort(2);
        if(ui->leftComboBox->findText(mModelMap.keys()[i])<0)
            ui->leftComboBox->addItem(mModelMap.keys()[i]);
    }
    qDebug() << "model end" << endl;
    if(ui->leftComboBox->count()>0)
        slotLeftChanged(0);

    QString message;
    if(skippedNum==0)
        message = tr("%1 files added").arg(addedNum);
    else
        message = tr("%1 files added, %2 files skipped").arg(addedNum).arg(skippedNum);
    qDebug() << message << endl;
    this->statusBar()->showMessage(message);

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
