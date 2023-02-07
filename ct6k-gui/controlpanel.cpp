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

// controlpanel.ccp - function definitions for ControlPanel class

#include <QObject>
#include "controlpanel.hpp"
#include "../src/arch.h"
#include "../src/cpu.hpp"

// Constructor. Does the meat of laying out items on the screen.
ControlPanel::ControlPanel(QWidget *parent)
    : QWidget{parent}
{
    Q_UNUSED(parent);
    BG = new QFrame(this);
    BG->setStyleSheet("background-color:#78B5C6;");
    BG->setAutoFillBackground(true);
    BG->setFixedSize(1509, 1011);
    BG->move(0,0);

    // RHL is Register Horizontal Layout, puts the register set and flags at the top
    // of the panel, with flags on the right.
    RHL = new QHBoxLayout;
    RS = new RegisterSet(this);
    FD = new FlagDisplay(this);
    RHL->addWidget(RS);
    RHL->addWidget(FD);
    // BHL is Button Horizontal Layout. These go in a horizontal line at the
    // bottom of the panel
    BHL = new QHBoxLayout;
    BtnStop = new QPushButton("STOP");
    BHL->addWidget(BtnStop);
    BtnStep = new QPushButton("STEP");
    BHL->addWidget(BtnStep);
    Btn1Hz= new QPushButton("SLOW");
    BHL->addWidget(Btn1Hz);
    Btn10Hz= new QPushButton("QUICK");
    BHL->addWidget(Btn10Hz);
    Btn60Hz= new QPushButton("FAST");
    BHL->addWidget(Btn60Hz);
    BtnFull= new QPushButton("RUN");
    BHL->addWidget(BtnFull);
    // Vertical Layout, Registers/Flags on top, buttons below. This is the outermost
    // layout for the widget.
    VL = new QVBoxLayout;
    VL->addLayout(RHL);
    VL->addLayout(BHL);
    setLayout(VL);
}

// No destructor. QT frees all children, so no need to delete everything.
// This feels wrong.

// Slot - called when the CPU Spinner wants to update the screen.
void ControlPanel::UpdateFromCPU(CPUInternalState *NewState)
{
    for (int i = 0; i < NUMREGS; i++)
        RS->SetValue(i, NewState->Registers[i]);
    // In theory, the flag display should be updated directly from the
    // register set, but in practice it's just too complicated to deal
    // with.
    FD->SetValue(NewState->Registers[REG_FLG]);
    FD->SetHaltedState(NewState->Halted);
    LastIHAP = NewState->IHAP_Base;
    LastFHAP = NewState->FHAP_Base;
}

// Called by method in mainwindow class in response to a menu click.
// Caller must stop the Spinner thread to ensure accuate results.
uint32_t ControlPanel::getLastFHAP() const
{
    return LastFHAP;
}

// Called by method in mainwindow class in response to a menu click.
// Caller must stop the Spinner thread to ensure accuate results.
uint32_t ControlPanel::getLastIHAP() const
{
    return LastIHAP;
}
