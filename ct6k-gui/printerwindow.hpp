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

// printerwindow.hpp - declarations for the PrinterWindow class
#ifndef PRINTERWINDOW_HPP
#define PRINTERWINDOW_HPP

#include <QObject>
#include <QLabel>
#include <QDialog>

#define NUM_VISIBLE_LINES 24

class PrinterWindow : public QObject
{
    Q_OBJECT
public:
    explicit PrinterWindow(QObject *parent = nullptr);
    ~PrinterWindow();
    void Show();
    void Hide();

public slots:
    void UpdatePrinterWindow(QString OutLine);

private:
    QDialog *POTBox;
    QLabel *Pinkbar[NUM_VISIBLE_LINES];
    int BottomIndex;
    QFont *DMF; // Dot Matrix Font
    void ScrollUp();
};

#endif // PRINTERWINDOW_HPP
