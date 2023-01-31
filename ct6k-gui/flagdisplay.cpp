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

// flagdisplay.cpp - function definitions for the FlagDisplay class

#include <QFrame>
#include "flagdisplay.hpp"
#include "../src/arch.h"

#define MAX_FLAG_TITLE_LEN 10 // INTERRUPT + null
struct FlgMap {
    const char Name[MAX_FLAG_TITLE_LEN];
    const uint32_t BitPos;
};

// Map of flag names to flag bits in the flag register
FlgMap flag_map[] = {
    {"OVERFLOW", FLG_OVER},
    {"UNDERFLOW", FLG_UNDER},
    {"ZERO", FLG_ZERO},
    {"INTERRUPT", FLG_IN_INT},
    {"SIGNED", FLG_SIGNED},
    {"INT_ENA", FLG_INTENA},
    {"FAULT", FLG_FAULT},
    {"", 0},
};

// Constructor. All Indicators and Lables are added to a single
// vertical layout.
FlagDisplay::FlagDisplay(QWidget *parent)
    : QWidget{parent}
{
    VL = new QVBoxLayout;
    for (int i = 0; i < NUM_FLAGS; i++) {
        if (i > 0) {
            QFrame *tmpFrame = new QFrame;
            tmpFrame->setFrameShape(QFrame::HLine);
            tmpFrame->setFrameShadow(QFrame::Plain);
            tmpFrame->setLineWidth(3);
            VL->addWidget(tmpFrame);
        }

        Flags[i] = new Indicator(nullptr);
        VL->addWidget(Flags[i]);
        Titles[i] = new QLabel;
        Titles[i]->setText(flag_map[i].Name);
        VL->addWidget(Titles[i]);
    }
    setLayout(VL);
}

// Update the indicator bits.
void FlagDisplay::SetValue(uint32_t NewVal)
{
    for (int i = 0; i < NUM_FLAGS; i++) {
        Flags[i]->SetState((NewVal & flag_map[i].BitPos) ? true : false);
    }
}

// Passthrough function to set bitmaps for the indicators.
void FlagDisplay::EnableBitmaps(QPixmap *OffPic, QPixmap *OnPic)
{
    for (int i = 0; i < NUM_FLAGS; i++) {
        Flags[i]->EnableBitmaps(OffPic, OnPic);
    }

}
