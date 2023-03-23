#!/bin/sh
MAJOR=$(grep VERSION_MAJOR ct6k-gui/ct6k-info.h | cut -f 2 -d "\"")
MINOR=$(grep VERSION_MINOR ct6k-gui/ct6k-info.h | cut -f 2 -d "\"")
SUBMINOR=$(grep VERSION_SUB ct6k-gui/ct6k-info.h | cut -f 2 -d "\"")
REV=$(grep VERSION_EXTRA ct6k-gui/ct6k-info.h | cut -f 2 -d "\"")
if [ -z ${REV} ]; then
	REV=1
fi
DEBNAME="ct6k_$MAJOR.$MINOR.$SUBMINOR-$REV"_x86_64
mkdir -p "$DEBNAME/usr/bin"
mkdir -p "$DEBNAME/usr/share/doc/ct6k-examples"
mkdir -p "$DEBNAME/usr/share/doc/ct6k/"
mkdir -p "$DEBNAME/usr/share/applications"
mkdir -p "$DEBNAME/usr/share/pixmaps"
mkdir -p "$DEBNAME/tmp"
mkdir -p "$DEBNAME/DEBIAN"
CONTROL="$DEBNAME/DEBIAN/control"
cat > ${CONTROL} << CEOF
Package: ct6k
Version: ${MAJOR}.${MINOR}.${SUBMINOR}
Architecture: amd64
Maintainer: Mitch Williams <cluemerchant@gmail.com>
Description: An emulator for the classic (fictional) Comp-o-Tron 6000 from 1956.
 The emulator is both a nerd toy and a tool for learning about basic computer
 architecture and assembly language concepts. The package includes the gui
 emulator (ct6k-gui) as well as a command-line assembler and virtual card punch
 tool for creating punch card files to feed the emulator.
Depends: libc6 (>= 2.34), libgcc-s1 (>= 3.0), libqt6core6 (>= 6.2.0), libqt6gui6 (>= 6.1.2), libqt6widgets6 (>= 6.1.2), libstdc++6 (>= 5.2)
CEOF
cp build-ct6k-gui-Desktop-Release/ct6k-gui "$DEBNAME/usr/bin"
cp ct6k-gui/ct6k-gui.png "$DEBNAME/usr/share/pixmaps"
cp src/asm6k "$DEBNAME/usr/bin"
cp src/punch "$DEBNAME/usr/bin"
chmod 755 ${DEBNAME}/usr/bin/*
cp examples/* "$DEBNAME/usr/share/doc/ct6k-examples"
cp ct6k.desktop "$DEBNAME/tmp"
cat > "$DEBNAME/usr/share/doc/ct6k/copyright" << LEOF
    The Comp-o-Tron 6000 software is Copyright (C) 2022-2023 Mitch Williams.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
LEOF
cp COPYING OFL.txt README HISTORY TODO "$DEBNAME/usr/share/doc/ct6k/"
pandoc -t gfm -o "$DEBNAME/usr/share/doc/ct6k/ug.md" "Comp-o-Tron 6000 User Guide.odt"
cat > "$DEBNAME/DEBIAN/postinst" << PIEOF
#!/bin/sh
desktop-file-install --dir /usr/share/applications --delete-original /tmp/ct6k.desktop
update-desktop-database /usr/share/applications
PIEOF
chmod 755 "$DEBNAME/DEBIAN/postinst"
cat > "$DEBNAME/DEBIAN/postrm" << PREOF
#!/bin/sh
rm -f /usr/share/applications/ct6k.desktop
PREOF
chmod 755 "$DEBNAME/DEBIAN/postrm"
dpkg-deb --build --root-owner-group "$DEBNAME"




