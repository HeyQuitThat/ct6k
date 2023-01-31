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

// dword.cpp - function definitions for the DWord class

#include "dword.hpp"
#include <QLabel>
#include <QFrame>

// Constructor
DWord::DWord(QWidget *Parent, QString Name)
{
    Q_UNUSED(Parent)
    Value = 0;
    Locked = true;
    LeLay = new QHBoxLayout;
    QLabel *Title = new QLabel; // No need to hang on to this, the layout will own it.
    Title->setText(Name);
    LeLay->addWidget(Title);
    setLayout(LeLay);
    // This loop runs from high to low so the most-signifcant bit is on the left.
    for (int i = 31; i >= 0; i--) {
        Bits[i] = new Indicator(nullptr);
        LeLay->addWidget(Bits[i]); // sets parent pointer
        if ((i > 0) && ((i & 0x3) == 0)) {
            QFrame *Spacer = new QFrame(nullptr);
            Spacer->setFrameShape(QFrame::VLine);
            Spacer->setFrameShadow(QFrame::Plain);
            Spacer->setLineWidth(3);
            LeLay->addWidget(Spacer); // sets parent pointer
        }
    }
}

// Return the cached value rather than call each indicator.
uint32_t DWord::GetValue()
{
    return Value;
}

// For this one, we need to update each indicator.
void DWord::SetValue(uint32_t NewVal)
{
    for (int i = 31; i >= 0; i--)
        Bits[i]->SetState(!!(NewVal & (1 << i)));
    Value = NewVal;
}

// When locked, clicking on the indicator does nothing. Because this is called
// after the user has presumably changed one or more bits, read and cache them
// all.
void DWord::Lock()
{
    uint32_t NewVal = 0;
    Locked = true;
    for (int i = 31; i >= 0; i--) {
        Bits[i]->Lock();
        if (Bits[i]->GetState())
            NewVal |= (1 << i);
    }
    Value = NewVal;
}

// Unlock the indicators for modification. The caller should ensure that the CPU
// is stopped while registers are unlocked.
void DWord::Unlock()
{
    Locked = false;
    for (int i = 31; i >= 0; i--)
        Bits[i]->Unlock();
}

// Passthrough function to set the blinkenlights.
void DWord::EnableBitmaps(QPixmap *OffPic, QPixmap *OnPic)
{
    for (int i = 31; i >= 0; i--)
        Bits[i]->EnableBitmaps(OffPic, OnPic);
}

