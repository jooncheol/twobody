VERSION=1.0
QTDIR=$HOME/qtsdk-2009.03/qt

rm -rf output
mkdir output
cp twobody output
cp *.qm output
cp COPYING output
cp -a $QTDIR/lib/libQtGui.so* output/
strip output/*
makeself output twobody-$VERSION.bin "Twobody $VERSION, Jooncheol's open source software" "LD_LIBRARY_PATH=. ./twobody > /dev/null 2> /dev/null"
