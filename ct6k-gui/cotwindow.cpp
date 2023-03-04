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

// cotwindow.cpp - Function definitions for COTWindow class
#include "cotwindow.hpp"
#include "indicator.hpp"
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

// Constructor, sets up all of the visual aspcects of the window
COTWindow::COTWindow(QObject *parent)
    : QObject{parent}
{
    OnImg = new QPixmap(":/ct6k/ib-red.jpg");
    OffImg = new QPixmap(":/ct6k/ib-red-off.jpg");
    COTBox = new QDialog();
    COTBox->setWindowFlags(Qt::Dialog);
    COTBox->setWindowTitle("Card-o-Tron 3CS");
    COTBox->setModal(false);
    COTBox->setSizeGripEnabled(false);

    VLP = new QVBoxLayout;
    IndP = new Indicator(nullptr);
    IndP->EnableBitmaps(OffImg, OnImg);
    VLP->addWidget(IndP);
    VLP->setAlignment(IndP, Qt::AlignHCenter);
    QLabel *LabP = new QLabel("PUNCHING");
    VLP->addWidget(LabP);
    VLP->setAlignment(LabP, Qt::AlignHCenter);
    VLP->addSpacing(10);
    EmptyP = new Indicator(nullptr);
    EmptyP->EnableBitmaps(OffImg, OnImg);
    EmptyP->SetState(true);
    VLP->addWidget(EmptyP);
    VLP->setAlignment(EmptyP, Qt::AlignHCenter);
    QLabel *LabEP = new QLabel("EMPTY");
    VLP->addWidget(LabEP);
    VLP->setAlignment(LabEP, Qt::AlignHCenter);
    VLP->addSpacing(10);
    PBP = new QPushButton(nullptr);
    PBP->setText("LOAD BLANK CARDS");
    VLP->addWidget(PBP);

    VLS = new QVBoxLayout;
    IndS = new Indicator(nullptr);
    IndS->EnableBitmaps(OffImg, OnImg);
    VLS->addWidget(IndS);
    VLS->setAlignment(IndS, Qt::AlignHCenter);
    QLabel *LabS = new QLabel("SCANNING");
    VLS->addWidget(LabS);
    VLS->setAlignment(LabS, Qt::AlignHCenter);
    VLS->addSpacing(10);
    EmptyS = new Indicator(nullptr);
    EmptyS->EnableBitmaps(OffImg, OnImg);
    EmptyS->SetState(true);
    VLS->addWidget(EmptyS);
    VLS->setAlignment(EmptyS, Qt::AlignHCenter);
    QLabel *LabES = new QLabel("EMPTY");
    VLS->addWidget(LabES);
    VLS->setAlignment(LabES, Qt::AlignHCenter);
    VLS->addSpacing(10);
    PBS = new QPushButton(nullptr);
    PBS->setText("LOAD CARD DECK");
    VLS->addWidget(PBS);

    HL = new QHBoxLayout;
    HL->addLayout(VLP);
    QFrame *Spacer = new QFrame(nullptr);
    Spacer->setFrameShape(QFrame::VLine);
    Spacer->setFrameShadow(QFrame::Plain);
    Spacer->setLineWidth(2);
    HL->addWidget(Spacer); // sets parent pointer
    HL->addLayout(VLS);
    COTBox->setLayout(HL);

    // Wire up signals and slots
    QObject::connect(PBP, SIGNAL(clicked()), this, SLOT(OpenOutputFile()));
    QObject::connect(PBS, SIGNAL(clicked()), this, SLOT(OpenInputFile()));
}

// Destructor
COTWindow::~COTWindow()
{
    if ((PunchOut != nullptr) && PunchOut->is_open())
        PunchOut->close();
    if ((ScanIn != nullptr) && ScanIn->is_open())
        ScanIn->close();
    COTBox->hide();
    delete COTBox;
    delete OffImg;
    delete OnImg;
}

// Show and hide methods are called from the menu in the main window
void COTWindow::Show()
{
    COTBox->show();
}

void COTWindow::Hide()
{
    COTBox->hide();
}

// Slot for the CPU Spinner to call in order to...
void COTWindow::UpdateBlinkyLights(bool Writing, bool Reading)
{
    IndS->SetState(Reading);
    IndP->SetState(Writing);
    EmptyP->SetState((PunchOut == nullptr) || !PunchOut->is_open());
    EmptyS->SetState((ScanIn == nullptr) || !ScanIn->is_open());
}

// Open text file for reading as a card deck.
void COTWindow::OpenInputFile()
{
    QString Filename = QFileDialog::getOpenFileName(nullptr, tr("Open Card Deck File"), ".", tr("CT6K Cards (*.ctc *.txt)"));
    if (Filename.isEmpty())
        return;
    ScanIn = new std::ifstream(Filename.toStdString().data(), std::ios::in);
    if (!ScanIn->is_open()) {
        QMessageBox Box;
        Box.setText("Error: Unable to open file.");
        Box.setIcon(QMessageBox::Warning);
        Box.exec();
        delete ScanIn;
        ScanIn = nullptr;
        return;
    }
    emit SetCOTSInput(ScanIn);
    EmptyS->SetState((ScanIn == nullptr) || !ScanIn->is_open());
}

// Open empty file to be written as card deck
void COTWindow::OpenOutputFile()
{
    QString Filename = QFileDialog::getSaveFileName(nullptr, tr("Save Card Deck File"), ".", tr("CT6K Cards (*.ctc *.txt)"));
    if (Filename.isEmpty())
        return;
    PunchOut = new std::ofstream(Filename.toStdString().data(), std::ios::out | std::ios::trunc);
    if (!PunchOut->is_open()) {
        QMessageBox Box;
        Box.setText("Error: Unable to open file.");
        Box.setIcon(QMessageBox::Warning);
        Box.exec();
        delete PunchOut;
        PunchOut = nullptr;
        return;
    }
    emit SetCOTPOutput(PunchOut);
    EmptyP->SetState((PunchOut == nullptr) || !PunchOut->is_open());
}
