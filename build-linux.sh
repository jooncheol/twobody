VERSION=1.1
QTDIR=$HOME/qtsdk-2009.03/qt
QTDIR=$HOME/qt-x11-opensource-src-4.5.2

$QTDIR/bin/qmake CONFIG+=release CCFLAGS+=-static LIBS+=/usr/lib/libexif.a LIBS+=/usr/lib/libjpeg.a QTPLUGIN+=qjpeg

lrelease-qt4 twobody_ko_KR.ts 

make


rm -rf output
mkdir output
cp twobody output
cp *.qm output
cp COPYING output
#cp -a $QTDIR/lib/libQtGui.so* output/
#mkdir output/imageformats
#cp -a $QTDIR/plugins/imageformats/libqjpeg.so output/imageformats
strip output/*
#strip output/imageformats/*
makeself output twobody-$VERSION.bin "Twobody $VERSION, Jooncheol's open source software" "LD_LIBRARY_PATH=. ./twobody > /dev/null 2> /dev/null"
