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

// These values are determined by checking window geometry after show() is called.
#define CP_FIXED_WIDTH 1509
#define CP_FIXED_HEIGHT 1011

// Button and icon geometry
#define CP_ICON_X 200
#define CP_ICON_Y 50
// Border in pixels, have to add it twice for top/bottom and left/right
#define CP_BUTTON_BORDER 3
#define CP_BUTTON_X (CP_ICON_X + (CP_BUTTON_BORDER * 2))
#define CP_BUTTON_Y (CP_ICON_Y + (CP_BUTTON_BORDER * 2))

// Constructor. Does the meat of laying out items on the screen.
ControlPanel::ControlPanel(QWidget *parent)
    : QWidget{parent}
{
    Q_UNUSED(parent);
    BG = new QFrame(this);
    BG->setStyleSheet("background-color:#78B5C6;");
    BG->setAutoFillBackground(true);
    BG->setFixedSize(CP_FIXED_WIDTH, CP_FIXED_HEIGHT);
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

    BtnStopImg = new QIcon(":/ct6k/STOP.png");
    BtnStop = new QPushButton();
    BtnStop->setIcon(*BtnStopImg);
    BtnStop->setIconSize(QSize(CP_ICON_X, CP_ICON_Y));
    BtnStop->setFixedSize(QSize(CP_BUTTON_X, CP_BUTTON_Y));
    BtnStop->setAutoFillBackground(true);
    BHL->addWidget(BtnStop);

    BtnStepImg = new QIcon(":/ct6k/STEP.png");
    BtnStep = new QPushButton();
    BtnStep->setIcon(*BtnStepImg);
    BtnStep->setIconSize(QSize(CP_ICON_X, CP_ICON_Y));
    BtnStep->setFixedSize(QSize(CP_BUTTON_X, CP_BUTTON_Y));
    BtnStep->setAutoFillBackground(true);
    BHL->addWidget(BtnStep);

    Btn1HzImg = new QIcon(":/ct6k/SLOW.png");
    Btn1Hz= new QPushButton();
    Btn1Hz->setIcon(*Btn1HzImg);
    Btn1Hz->setIconSize(QSize(CP_ICON_X, CP_ICON_Y));
    Btn1Hz->setFixedSize(QSize(CP_BUTTON_X, CP_BUTTON_Y));
    Btn1Hz->setAutoFillBackground(true);
    BHL->addWidget(Btn1Hz);

    Btn10HzImg = new QIcon(":/ct6k/QUICK.png");
    Btn10Hz= new QPushButton();
    Btn10Hz->setIcon(*Btn10HzImg);
    Btn10Hz->setIconSize(QSize(CP_ICON_X, CP_ICON_Y));
    Btn10Hz->setFixedSize(QSize(CP_BUTTON_X, CP_BUTTON_Y));
    Btn10Hz->setAutoFillBackground(true);
    BHL->addWidget(Btn10Hz);

    Btn60HzImg = new QIcon(":/ct6k/FAST.png");
    Btn60Hz= new QPushButton();
    Btn60Hz->setIcon(*Btn60HzImg);
    Btn60Hz->setIconSize(QSize(CP_ICON_X, CP_ICON_Y));
    Btn60Hz->setFixedSize(QSize(CP_BUTTON_X, CP_BUTTON_Y));
    Btn60Hz->setAutoFillBackground(true);
    BHL->addWidget(Btn60Hz);

    BtnFullImg = new QIcon(":/ct6k/RUN.png");
    BtnFull= new QPushButton();
    BtnFull->setIcon(*BtnFullImg);
    BtnFull->setIconSize(QSize(CP_ICON_X, CP_ICON_Y));
    BtnFull->setFixedSize(QSize(CP_BUTTON_X, CP_BUTTON_Y));
    BtnFull->setAutoFillBackground(true);
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
