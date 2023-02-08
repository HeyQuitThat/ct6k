/*
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
*/

// First code executed January 22, 2023.

#include "mainwindow.hpp"
#include <QObject>
#include <QApplication>
#include <QFontDatabase>

// main() is just a template created by QT Creator. A lot of examples
// show setup code here in main(), but nearly all of that code belongs
// in the constructor for MainWindow, which owns all of the widgets.
// You could put the init stuff for core logic here, but moving it
// to MainWindow makes it a lot easier to set up signal/slot connections.
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(QPixmap(":/ct6k/ib-on.jpg")));

    // Load resources from resource file
    QFontDatabase fontDB;
    fontDB.addApplicationFont(":/ct6k/Cabin-Regular.otf");
    fontDB.addApplicationFont(":/ct6k/Cabin-Bold.otf");
    fontDB.addApplicationFont(":/ct6k/enhanced_dot_digital-7.ttf");

    MainWindow w;

    w.show();
    // Size needs to set after show() is called so we know how big it is.
    w.setFixedSize(w.width(), w.height());

    return a.exec();
}
