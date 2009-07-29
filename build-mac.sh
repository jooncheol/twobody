#!/bin/bash
qmake-4.5 -config static
open twobody.xcodeproj
lrelease twobody_ko_KR.ts 
cp twobody_*.qm twobody.app/Contents/Resources/
macdeployqt-4.5 twobody.app -dmg
