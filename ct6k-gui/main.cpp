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
// main() is just a template created by QT Creator. A lot of examples
// show setup code here in main(), but nearly all of that code belongs
// in the constructor for MainWindow, which owns all of the widgets.
// You could put the init stuff for core logic here, but moving it
// to MainWindow makes it a lot easier to set up signal/slot connections.
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    return a.exec();
}