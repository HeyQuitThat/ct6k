#!/bin/bash
# This script is designed to be run from the Cygwin environemnt on Windows
# using the Qt 6.4.2 toolchain and the Qt Installer Frameword version
# 4.5.1. Yes, the paths are hard-coded. Yes, they should be fixed.
MAJOR=$(grep VERSION_MAJOR ct6k-gui/ct6k-info.h | cut -f 2 -d "\"")
MINOR=$(grep VERSION_MINOR ct6k-gui/ct6k-info.h | cut -f 2 -d "\"")
SUBMINOR=$(grep VERSION_SUB ct6k-gui/ct6k-info.h | cut -f 2 -d "\"")
REV=$(grep VERSION_EXTRA ct6k-gui/ct6k-info.h | cut -f 2 -d "\"")
if [ -z ${REV} ]; then
	REV=1
fi
WINNAME="ct6k-install-$MAJOR.$MINOR.$SUBMINOR.exe"
mkdir pkg/packages/org.ct6k.ct6k/data
cp *Release/*.exe pkg/packages/org.ct6k.ct6k/data
pushd pkg/packages/org.ct6k.ct6k/data
/cygdrive/c/Qt/6.4.2/msvc2019_64/bin/windeployqt.exe ct6k-gui.exe
mkdir tools
mkdir examples
popd
cp src/*.exe pkg/packages/org.ct6k.ct6k/data/tools
cp examples/* pkg/packages/org.ct6k.ct6k/data/examples
cp ct6k-gui/appicon.ico pkg/config/Comp-o-Tron_6000.ico
cp pkg/config/*.ico pkg/packages/org.ct6k.ct6k/data
cp COPYING pkg/packages/org.ct6k.ct6k/meta/
cp OFL.txt pkg/packages/org.ct6k.ct6k/meta/
cd pkg
/cygdrive/c/Qt/QtIFW-4.5.1/bin/binarycreator.exe --offline-only -c config/config.xml -p packages/ ${WINNAME}
