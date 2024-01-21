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

// registerset.cpp - function definitions for the RegisterSet class

#include "registerset.hpp"
#include <QString>

// Constructor, sets up 16 DWords in a vertical layout
RegisterSet::RegisterSet(QWidget *Parent, bool SmallScreen) : QWidget{Parent}
{
    Locked = true;
    VL = new QVBoxLayout;
    setLayout(VL);
    for (int i = 0; i < NUMREGS; i++) {
        QString Title;
        switch (i) {
        case REG_IP:
            Title = "R15/IP";
            break;
        case REG_SP:
            Title = "R14/SP";
            break;
        case REG_FLG:
            Title = "R13/FLG";
            break;
        default:
            Title = QString("R%1").arg(i);
            break;
        }

        DWord *Rtmp = new DWord(nullptr, Title, SmallScreen);
        Regs[i] = Rtmp;
        VL->addWidget(Rtmp); // this sets parent for the Indicator objects
    }
}

// The remainder of these methods are just passthroughs to DWord methods.

uint32_t RegisterSet::GetValue(int Index)
{
    if ((Index >= 0) && (Index < NUMREGS))
        return Regs[Index]->GetValue();
    return 0;
}

void RegisterSet::SetValue(int Index, uint32_t NewVal)
{
    if ((Index >= 0) && (Index < NUMREGS))
        Regs[Index]->SetValue(NewVal);
}


void RegisterSet::Lock()
{
    for (int i = 0; i < NUMREGS; i++)
        Regs[i]->Lock();
}

void RegisterSet::Unlock()
{
    for (int i = 0; i < NUMREGS; i++)
        Regs[i]->Unlock();
}

void RegisterSet::EnableBitmaps(QPixmap *OffPic, QPixmap *OnPic)
{
    for (int i = 0; i < NUMREGS; i++)
        Regs[i]->EnableBitmaps(OffPic, OnPic);
}


