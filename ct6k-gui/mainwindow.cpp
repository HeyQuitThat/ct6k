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

// mainwindow.cpp - function definitions for the MainWindow class
// This class is based on a QT Creator template

#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include <cstdint>
#include <QFile>
#include <QFileDialog>
#include <QDataStream>
#include <QFontDatabase>
#include <QString>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include "../src/arch.h"

#include "controlpanel.hpp"
#include "ct6k-info.h"

// background color RGB 120,181,198 0x78B5C6 as of 1/30/23. This  will likely change.


// Constructor. Creates the UI, hooks up the signals for the buttons.
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    PW = new PrinterWindow(this);
    CW = new COTWindow(this);
    OffImg = new QPixmap(":/ct6k/ib-off.jpg");
    OnImg = new QPixmap(":/ct6k/ib-on.jpg");

    // Create and configure the Control Panel
    CP = new ControlPanel(this);
    CP->setStyleSheet("font-family: \"Cabin Bold\";"
                      "color: white;"
                      "font-size: 12pt;");

    CP->RS->EnableBitmaps(OffImg, OnImg);
    CP->FD->EnableBitmaps(OffImg, OnImg);
    setCentralWidget(CP);

    // Control panel must be created and set to CentralWidget before the Worker
    // is created or the connect call from the Spinner to the CP will fail.
    Worker = new CPUWorker(this);
    setWindowTitle(WINDOW_TITLE);

    // Hook up the buttons to the Worker
    QObject::connect(CP->BtnStep, SIGNAL(clicked()), Worker, SLOT(StepOnce()));
    QObject::connect(CP->BtnStop, SIGNAL(clicked()), Worker, SLOT(Pause()));
    QObject::connect(CP->Btn1Hz, SIGNAL(clicked()), Worker, SLOT(Run1Hz()));
    QObject::connect(CP->Btn10Hz, SIGNAL(clicked()), Worker, SLOT(Run10Hz()));
    QObject::connect(CP->Btn60Hz, SIGNAL(clicked()), Worker, SLOT(Run60Hz()));
    QObject::connect(CP->BtnFull, SIGNAL(clicked()), Worker, SLOT(RunFull()));
    QObject::connect (CW, SIGNAL(SetCOTSInput(std::ifstream*)), Worker, SLOT(SetCOTSInput(std::ifstream*)));
    QObject::connect (CW, SIGNAL(SetCOTPOutput(std::ofstream*)), Worker, SLOT(SetCOTPOutput(std::ofstream*)));
    RegistersLocked = true;
}

// Destructor. Destroys all of the widgets and their offspring.
MainWindow::~MainWindow()
{
    PW->Hide();
    delete PW;
    CW->Hide();
    delete CW;
    delete ui;
}


// Menu item slot - File/Exit
void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

// Convenience function to fill a 32-bit value from a byte-oriented file
uint32_t FillWordFromMSB(uint8_t *Buf)
{
    uint32_t retval {0};
    for (auto i = 24, j = 0; i >= 0; i -= 8, j++) // ugly magic numbers
        retval |= (uint32_t)Buf[j] << i;
    return retval;
}

// Menu item slot File/Load Binary
void MainWindow::on_actionLoad_Binary_triggered()
{
    QString Filename = QFileDialog::getOpenFileName(this, tr("Open Binary File"), ".", tr("CT6K Binary (*.ct6 *.bin)"));
    QFile InFile(Filename);
    InFile.open(QIODevice::ReadOnly);
    int Len = InFile.size();
    if ((Len % 4) != 0) {
        QMessageBox Box;
        Box.setText("Error: invalid file. The file will not be loaded.");
        Box.setIcon(QMessageBox::Warning);
        Box.exec();
        InFile.close();
        return;
    }

    // Read the data from the file and make sure it's in the correct byte order
    // The Comp-o-Tron 6000 is word-oriented and doesn't have a concept of
    // endianness - but the host system sure does!
    QDataStream InStream(&InFile);
    auto Buffer = new char[Len];
    auto Buf32 = new uint32_t[Len/4];
    InStream.readRawData(Buffer, Len);
    for (int i = 0; i < (Len/4); i++) {
        Buf32[i] = FillWordFromMSB((uint8_t *)Buffer + (i*4));
    }
    delete[] Buffer;

    // We've got the data and it's been swizzled. Now to get it into memory...
    Worker->ProgramBinary(Buf32, Len / 4);
    delete[] Buf32;
}


// Menu item slot Debug/Reset
// The worker takes care of stopping and restarting the thread.
void MainWindow::on_actionReset_triggered()
{
    Worker->ResetCPU();
}

// Convenience function to format a hex value with a lower-case 0x
// and uppercase hex digits, as it should be.
QString HexVal(uint32_t Value)
{
    QString retval = "0x";
    retval += QString("%1").arg(Value,8,16, QChar('0')).toUpper();
    return retval;
}

// Menu item slot Debug/Show FHAP
void MainWindow::on_actionShow_FHAP_triggered()
{
    Worker->Quiesce();
    uint32_t FHAP = CP->getLastFHAP();
    Worker->Go();

    QMessageBox Box;
    Box.setText(QString("FHAP = ") + HexVal(FHAP));
    Box.exec();

}


// Menu item slot Debug/Show IHAP
void MainWindow::on_actionShow_IHAP_triggered()
{
    Worker->Quiesce();
    uint32_t IHAP = CP->getLastIHAP();
    Worker->Go();
    QMessageBox Box;
    Box.setText(QString("IHAP = 0x") + HexVal(IHAP));
    Box.exec();
}

// Menu item slot Debug/Dump Memory
void MainWindow::on_actionDump_Memory_triggered()
{
    bool OK;
    uint32_t Base = GetAddrFromUser("Memory Dump", "Enter Start Address or Register:", "0", &OK);
    if (OK) {
        QString OutBuf = "<tt>";
        Worker->Quiesce();
        for (int i = 0; i < 16 ; i++) {
            uint32_t tmp = Worker->ReadMem(Base + i);
            OutBuf += HexVal(Base + i) + ": " + HexVal(tmp) + "<br>";
        }
        OutBuf += "</tt>";
        Worker->Go();
        QMessageBox MB;
        MB.setText(OutBuf);
        MB.exec();
    }
}

// Print the instruction based upon the value(s) given. If Count is specified, update the count of
// words used for the instruction.
QString FormatDisasm(uint32_t Val, uint32_t Val2, uint32_t *Count)
{
    std::string outstr;
    auto i = new Instruction(Val, Val2);
    i->Print(outstr);
    if (Count != nullptr)
        *Count = i->SizeInMemory();
    delete i;
    return QString::fromStdString(outstr);
}


// Menu item slot Debug/Disassemble
void MainWindow::on_actionDisassemble_triggered()
{
    bool OK;
    uint32_t Base = GetAddrFromUser("Disassembly", "Enter Start Address or Register:", "0", &OK);
    if (OK) {
        QString OutBuf = "<tt>";
        uint32_t Count = 0;
        Worker->Quiesce();
        for (int i = 0; i < 16 ; i++) {
            OutBuf += HexVal(Base) + ": ";
            OutBuf += FormatDisasm(Worker->ReadMem(Base),Worker->ReadMem(Base + 1), &Count);
            OutBuf += "<br>";
            Base += Count;
        }
        OutBuf += "</tt>";
        Worker->Go();
        QMessageBox MB;
        MB.setText(OutBuf);
        MB.exec();
    }
}



// Menu item slot Debug/Modify Registers
void MainWindow::on_actionModify_Registers_triggered(bool checked)
{
    // checked is the state of the check box next to the menu item
    if (checked == true) {
        // Halt CPU, unlock registers
        Worker->Quiesce();
        CP->RS->Unlock();
        QMessageBox M;
        M.setText("Registers unlocked for modification.");
        M.setInformativeText("CPU is halted until registers are locked.");
        M.exec();
    } else { // not checked
        CP->RS->Lock();
        for (int i = 0; i < NUMREGS; i++)
            Worker->WriteReg(i, CP->RS->GetValue(i));
        QMessageBox M;
        M.setText("Registers locked.");
        M.setInformativeText("CPU execution may continue.");
        M.exec();
        Worker->Go();
        // No need to force an update to the CP, it's already current.
    }
}

// Menu item slot Debug/Modify Memory
void MainWindow::on_actionModify_Memory_triggered()
{
    bool OK;

    Worker->Quiesce();
    // First, get a base address.
    uint32_t Base = GetValFromUser("Modify Memory", "Enter Start Address:", "0", &OK);
    while (OK){
        // As long as the user puts in valid values and doesn't cancel,
        // keep getting values for each subsequent memory location.
        uint32_t Value = Worker->ReadMem(Base);
        QString Label = "Address ";
        Label += HexVal(Base);
        QString ValS = HexVal(Value);
        uint32_t NewVal = GetValFromUser("Write Memory", Label, ValS, &OK);
        if (OK)
            Worker->WriteMem(Base, NewVal);
        Base++;
    }

    Worker->Go();
}

// Convenience function to display an input dialog and parse a uint32 value from it.
uint32_t MainWindow::GetValFromUser(QString Title, QString Message, QString Seed, bool *Success)
{
    bool OK;

    QString RawText = QInputDialog::getText(this, Title, Message, QLineEdit::Normal, Seed, &OK);
    if (OK && !RawText.isEmpty()) {
        uint32_t RawVal = RawText.toULong(&OK, 0);
        *Success = OK;
        return RawVal;
    } else {
        *Success = false;
        return 0;
    }
}

#define NO_REG 0xFF
uint8_t RegName(QString Input)
{
    uint8_t retval = NO_REG;
    if (Input.compare("IP", Qt::CaseInsensitive) == 0)
        retval = REG_IP;
    if (Input.compare("SP", Qt::CaseInsensitive) == 0)
        retval = REG_SP;
    if (Input.compare("FLG", Qt::CaseInsensitive) == 0)
        retval = REG_FLG;
    if (Input.startsWith('R', Qt::CaseInsensitive)) {
        Input.remove(0,1); // Strip first char
        bool OK;
        retval = Input.toUShort(&OK, 0);
        if ((retval > REG_IP) || !OK)
            retval = NO_REG;
    }
    return retval;
}

// Convenience function to display an input dialog and parse a uint32 value from it.
// This variation allows a regsiter argument, i.e. the user can type R10 or PC
uint32_t MainWindow::GetAddrFromUser(QString Title, QString Message, QString Seed, bool *Success)
{
    bool OK;

    QString RawText = QInputDialog::getText(this, Title, Message, QLineEdit::Normal, Seed, &OK);
    if (OK && !RawText.isEmpty()) {
        uint32_t RawVal {0};
        uint8_t Reg = RegName(RawText);
        if (Reg == NO_REG)
            RawVal = RawText.toULong(&OK, 0);
        else
            RawVal = Worker->ReadReg(Reg);
        *Success = OK;
        return RawVal;
    } else {
        *Success = false;
        return 0;
    }
}

// Menu item slot Help/Instructions
void MainWindow::on_actionInstructions_triggered()
{
    QString OutBuf = INSTRUCTION_TEXT;
    QMessageBox MB;
    MB.setText(OutBuf);
    MB.setWindowTitle("Comp-o-Tron 6000 Help");
    MB.exec();
}


// Menu item slot Help/About
void MainWindow::on_actionAbout_triggered()
{
    QString OutBuf = INFO_TEXT VERSION_MAJOR "." VERSION_MINOR "." VERSION_SUBMINOR VERSION_EXTRA;
    QMessageBox MB;
    MB.setText(OutBuf);
    MB.setWindowTitle("About The Comp-o-Tron 6000 Emulator");
    MB.exec();

}


// Menu item slot Help/Caution!!
void MainWindow::on_actionCaution_triggered()
{
    QString OutBuf = WARNING_TEXT;
    QMessageBox MB;
    MB.setText(OutBuf);
    MB.setWindowTitle("Achtung!!");
    MB.exec();

}



void MainWindow::on_actionPrint_O_Tron_XL_triggered()
{
    PW->Show();
}


void MainWindow::on_actionLog_Printer_Output_triggered(bool checked)
{
    if (checked == true)
        PW->StartLog();
    else
        PW->StopLog();
}


void MainWindow::on_actionCard_o_Tron_3CS_triggered()
{
    CW->Show();
}

