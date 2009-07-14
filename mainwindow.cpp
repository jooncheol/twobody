#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QMap>
#include <QtGui>
#include <Qtimer>
#include "ui_mainwindow.h"
#include "thumbdelegate.h"
extern "C" {
#include "jpeglib.h"
#include <setjmp.h>
}


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



extern "C" {
static const int max_buf = 4096;

struct my_jpeg_destination_mgr : public jpeg_destination_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    QIODevice *device;
    JOCTET buffer[max_buf];

public:
    my_jpeg_destination_mgr(QIODevice *);
};
static void qt_init_destination(j_compress_ptr)
{
}

static boolean qt_empty_output_buffer(j_compress_ptr cinfo)
{
    my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;

    int written = dest->device->write((char*)dest->buffer, max_buf);
    if (written == -1)
        (*cinfo->err->error_exit)((j_common_ptr)cinfo);

    dest->next_output_byte = dest->buffer;
    dest->free_in_buffer = max_buf;

#if defined(Q_OS_UNIXWARE)
    return B_TRUE;
#else
    return true;
#endif
}

static void qt_term_destination(j_compress_ptr cinfo)
{
    my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
    qint64 n = max_buf - dest->free_in_buffer;

    qint64 written = dest->device->write((char*)dest->buffer, n);
    if (written == -1)
        (*cinfo->err->error_exit)((j_common_ptr)cinfo);
}


inline my_jpeg_destination_mgr::my_jpeg_destination_mgr(QIODevice *device)
{
    jpeg_destination_mgr::init_destination = qt_init_destination;
    jpeg_destination_mgr::empty_output_buffer = qt_empty_output_buffer;
    jpeg_destination_mgr::term_destination = qt_term_destination;
    this->device = device;
    next_output_byte = buffer;
    free_in_buffer = max_buf;
}

struct my_error_mgr : public jpeg_error_mgr {
    jmp_buf setjmp_buffer;
};
static void my_error_exit (j_common_ptr cinfo)
{
    my_error_mgr* myerr = (my_error_mgr*) cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    qWarning("%s", buffer);
    longjmp(myerr->setjmp_buffer, 1);
}
static bool write_jpeg_image(const QImage &sourceImage, QIODevice *device, uint8_t *exif_buf, uint8_t exif_buf_len)
{
    bool success = false;
    const QImage image = sourceImage;
    const QVector<QRgb> cmap = image.colorTable();

    struct jpeg_compress_struct cinfo;
    JSAMPROW row_pointer[1];
    row_pointer[0] = 0;

    struct my_jpeg_destination_mgr *iod_dest = new my_jpeg_destination_mgr(device);
    struct my_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = my_error_exit;

    if (!setjmp(jerr.setjmp_buffer)) {
        // WARNING:
        // this if loop is inside a setjmp/longjmp branch
        // do not create C++ temporaries here because the destructor may never be called
        // if you allocate memory, make sure that you can free it (row_pointer[0])
        jpeg_create_compress(&cinfo);

        cinfo.dest = iod_dest;

        cinfo.image_width = image.width();
        cinfo.image_height = image.height();

        bool gray=false;
        switch (image.format()) {
        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
        case QImage::Format_Indexed8:
            gray = true;
            for (int i = image.numColors(); gray && i--;) {
                gray = gray & (qRed(cmap[i]) == qGreen(cmap[i]) &&
                               qRed(cmap[i]) == qBlue(cmap[i]));
            }
            cinfo.input_components = gray ? 1 : 3;
            cinfo.in_color_space = gray ? JCS_GRAYSCALE : JCS_RGB;
            break;
        default:
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;
        }

        jpeg_set_defaults(&cinfo);

        qreal diffInch = qAbs(image.dotsPerMeterX()*2.54/100. - qRound(image.dotsPerMeterX()*2.54/100.))
                         + qAbs(image.dotsPerMeterY()*2.54/100. - qRound(image.dotsPerMeterY()*2.54/100.));
        qreal diffCm = (qAbs(image.dotsPerMeterX()/100. - qRound(image.dotsPerMeterX()/100.))
                        + qAbs(image.dotsPerMeterY()/100. - qRound(image.dotsPerMeterY()/100.)))*2.54;
        if (diffInch < diffCm) {
            cinfo.density_unit = 1; // dots/inch
            cinfo.X_density = qRound(image.dotsPerMeterX()*2.54/100.);
            cinfo.Y_density = qRound(image.dotsPerMeterY()*2.54/100.);
        } else {
            cinfo.density_unit = 2; // dots/cm
            cinfo.X_density = (image.dotsPerMeterX()+50) / 100;
            cinfo.Y_density = (image.dotsPerMeterY()+50) / 100;
        }

        int quality = 100;
#if defined(Q_OS_UNIXWARE)
        jpeg_set_quality(&cinfo, quality, B_TRUE /* limit to baseline-JPEG values */);
        jpeg_start_compress(&cinfo, B_TRUE);
#else
        jpeg_set_quality(&cinfo, quality, true /* limit to baseline-JPEG values */);
        jpeg_start_compress(&cinfo, true);
#endif

        jpeg_write_marker (&cinfo, 0xe1, exif_buf, exif_buf_len);

        row_pointer[0] = new uchar[cinfo.image_width*cinfo.input_components];
        int w = cinfo.image_width;
        while (cinfo.next_scanline < cinfo.image_height) {
            uchar *row = row_pointer[0];
            switch (image.format()) {
            case QImage::Format_Mono:
            case QImage::Format_MonoLSB:
                if (gray) {
                    const uchar* data = image.scanLine(cinfo.next_scanline);
                    if (image.format() == QImage::Format_MonoLSB) {
                        for (int i=0; i<w; i++) {
                            bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
                            row[i] = qRed(cmap[bit]);
                        }
                    } else {
                        for (int i=0; i<w; i++) {
                            bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
                            row[i] = qRed(cmap[bit]);
                        }
                    }
                } else {
                    const uchar* data = image.scanLine(cinfo.next_scanline);
                    if (image.format() == QImage::Format_MonoLSB) {
                        for (int i=0; i<w; i++) {
                            bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
                            *row++ = qRed(cmap[bit]);
                            *row++ = qGreen(cmap[bit]);
                            *row++ = qBlue(cmap[bit]);
                        }
                    } else {
                        for (int i=0; i<w; i++) {
                            bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
                            *row++ = qRed(cmap[bit]);
                            *row++ = qGreen(cmap[bit]);
                            *row++ = qBlue(cmap[bit]);
                        }
                    }
                }
                break;
            case QImage::Format_Indexed8:
                if (gray) {
                    const uchar* pix = image.scanLine(cinfo.next_scanline);
                    for (int i=0; i<w; i++) {
                        *row = qRed(cmap[*pix]);
                        ++row; ++pix;
                    }
                } else {
                    const uchar* pix = image.scanLine(cinfo.next_scanline);
                    for (int i=0; i<w; i++) {
                        *row++ = qRed(cmap[*pix]);
                        *row++ = qGreen(cmap[*pix]);
                        *row++ = qBlue(cmap[*pix]);
                        ++pix;
                    }
                }
                break;
            case QImage::Format_RGB888:
                memcpy(row, image.scanLine(cinfo.next_scanline), w * 3);
                break;
            case QImage::Format_RGB32:
            case QImage::Format_ARGB32:
            case QImage::Format_ARGB32_Premultiplied: {
                QRgb* rgb = (QRgb*)image.scanLine(cinfo.next_scanline);
                for (int i=0; i<w; i++) {
                    *row++ = qRed(*rgb);
                    *row++ = qGreen(*rgb);
                    *row++ = qBlue(*rgb);
                    ++rgb;
                }
                break;
            }
            default:
                qWarning("QJpegHandler: unable to write image of format %i",
                         image.format());
                break;
            }
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        success = true;
    } else {
        jpeg_destroy_compress(&cinfo);
        success = false;
    }

    delete iod_dest;
    delete [] row_pointer[0];
    return success;
}
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

    ExifData *ed = exif_data_new_from_file (filePath.toLocal8Bit().constData());
    ExifEntry *ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_DATE_TIME);
    sprintf((char*)ee->data, "%s", userData["after"].toString().toUtf8().constData());
    qDebug() << "date time: "  << (const char*)ee->data;
    ee = NULL;

    ee = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_DATE_TIME_ORIGINAL);
    if(ee)
        sprintf((char*)ee->data, "%s", userData["after"].toString().toUtf8().constData());
    ee = NULL;

    ee = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_DATE_TIME_DIGITIZED);
    if(ee)
        sprintf((char*)ee->data, "%s", userData["after"].toString().toUtf8().constData());
    ee = NULL;


    userData["date time"] = QVariant(userData["after"].toString());
    userData["after"] = QVariant(userData["date time"].toString());
    userData["delta"] = QVariant("+ 00:00:00");
    item->setData(userData, Qt::UserRole);

    uint8_t *exif_buf = NULL;
    uint32_t exif_buf_len = 0;
    exif_data_save_data (ed, &exif_buf, &exif_buf_len);
    qDebug() << "exif buf len: " << exif_buf_len << endl;

    QImage image(filePath, "JPEG");
    QFile f(filePath+".jpg");
    f.open(QIODevice::WriteOnly);
    write_jpeg_image(image, &f, exif_buf, exif_buf_len);
    f.close();
    QFile f2(filePath+".exif");
    f2.open(QIODevice::WriteOnly);
    f2.write((const char*)exif_buf, exif_buf_len);
    f2.close();

    free(exif_buf);
    exif_data_unref(ed);

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
