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

// printerwindow.cpp - declarations for class PrinterWindow
#include "printerwindow.hpp"
#include <QMessageBox>

// Print-o-Tron XL standard pinkbar pink is color 0xee86eb

// Constructor
PrinterWindow::PrinterWindow(QObject *parent)
    : QObject{parent}
{
    DMF = new QFont("Merchant Copy", 14, QFont::Normal);
    POTBox = new QDialog();
    POTBox->setWindowFlags(Qt::Window);
    POTBox->setWindowTitle("Print-o-Tron XL");
    POTBox->setModal(false);
    POTBox->setSizeGripEnabled(false);
    POTBox->setFixedSize(1100, LINE_HEIGHT * NUM_VISIBLE_LINES); // width is pure guesswork
    // Style sheets for the labels, one for pink, one for white background
    QString SSP = "color:black;"
                  "background-color:#de93e6;"
                  "font-family: \"Merchant Copy\";"
                  "font-size: 14pt";
    QString SSW = "color:black;"
                  "background-color:white;"
                  "font-family: \"Merchant Copy\";"
                  "font-size: 14pt";
    for (int i = 0; i < NUM_VISIBLE_LINES; i++) {
        Pinkbar[i] = new QLabel(POTBox, Qt::Widget);
        if (i & 0x4)
            Pinkbar[i]->setStyleSheet(SSP);
        else
            Pinkbar[i]->setStyleSheet(SSW);
        Pinkbar[i]->setAutoFillBackground(true);
        Pinkbar[i]->setFixedSize(1080, LINE_HEIGHT);
        Pinkbar[i]->move(8, i * LINE_HEIGHT);
    }
    BottomIndex = 0;
}

// Destructor
PrinterWindow::~PrinterWindow()
{
    POTBox->hide();
    delete DMF;
    delete POTBox;
    if (PrinterLog.is_open())
        PrinterLog.close();
}

// Slots from the menu to show and hide the window
void PrinterWindow::Show()
{
    POTBox->show();
}

void PrinterWindow::Hide()
{
    POTBox->hide();
}

// Start logging to file Print-o-Tron_XL.log
// Appends to existing file
void PrinterWindow::StartLog()
{
    try {
        PrinterLog.open(OUTFILE_NAME, std::ios::out | std::ios::app);
    } catch (const char* msg) {
        QMessageBox MB;
        MB.setText("Error opening log file:");
        MB.setDetailedText(msg);
        MB.setIcon(QMessageBox::Warning);
        MB.exec();
    }

}

// Stop logging
void PrinterWindow::StopLog()
{
    PrinterLog.close();
}

// We always print at the bottom and scroll up like a printer.
// The Comp-O-Tron Corporation specifies pinkbar paper for all
// Print-O-Tron models.
void PrinterWindow::UpdatePrinterWindow(QString OutLine)
{

    ScrollUp();
    Pinkbar[BottomIndex]->setText(OutLine);
    if (PrinterLog.is_open())
        PrinterLog << OutLine.toStdString() << std::endl;

}


// Scroll up by moving the label widgets. We don't need to keep track
// of the contents of each line; the labels do that.
void PrinterWindow::ScrollUp()
{
    for (int i = 0; i < NUM_VISIBLE_LINES; i++) {
        int y = Pinkbar[i]->y();
        y -= LINE_HEIGHT;
        if (y < 0) {
            y = LINE_HEIGHT * (NUM_VISIBLE_LINES - 1);
            BottomIndex = i;
        }
        Pinkbar[i]->move(Pinkbar[i]->x(), y);
    }
}
