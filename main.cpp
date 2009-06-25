#include <QApplication>
#include <QDebug>

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-ifd.h>
#include <libexif/exif-content.h>


int main(int argc, char **argv)
{
    /*
    QApplication app(argc, argv);
    return app.exec();
    */
    //ExifData *ed = exif_data_new_from_file ("IMG_0033.JPG");
    ExifData *ed = exif_data_new_from_file ("IMGP1337.JPG");
    exif_data_dump(ed);

    ExifEntry *ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_MAKE);
    char value[256]={0,};
    printf("Make: %s\n", exif_entry_get_value(ee, value, sizeof(value)));
    ee = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_MODEL);
    printf("Model: %s\n", exif_entry_get_value(ee, value, sizeof(value)));
    return 0;
}
