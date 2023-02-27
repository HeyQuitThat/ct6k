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

// controlpanel.h - declarations for the ControlPanel class
#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include "registerset.hpp"
#include "flagdisplay.hpp"
#include "../src/cpu.hpp"

// This is the overarching class for the visible control panel, including
// all of the lights, buttons, and accompanying text.
class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ControlPanel(QWidget *parent = nullptr);
    RegisterSet *RS;
    FlagDisplay *FD;


public slots:
    // signal comes from CPU Spinner class
    void UpdateFromCPU(CPUInternalState *NewState);

private:
    QFrame *BG;       // background color
    QVBoxLayout *VL;  // overarching vertical layout
    QHBoxLayout *RHL; // registers and flags
    QHBoxLayout *BHL; // buttons
    uint32_t LastIHAP; // IHAP and FHAP values from CPU, cached for display
    uint32_t LastFHAP;


public: // buttons are public so we can hook up the signals they emit easily
    QPushButton *BtnStop;
    QPushButton *BtnStep;
    QPushButton *Btn1Hz;
    QPushButton *Btn10Hz;
    QPushButton *Btn60Hz;
    QPushButton *BtnFull;
    QIcon *BtnStopImg;
    QIcon *BtnStepImg;
    QIcon *Btn1HzImg;
    QIcon *Btn10HzImg;
    QIcon *Btn60HzImg;
    QIcon *BtnFullImg;
    uint32_t getLastIHAP() const;
    uint32_t getLastFHAP() const;
};

#endif // CONTROLPANEL_H
