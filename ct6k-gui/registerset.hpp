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

// registerset.hpp - declarations for the RegisterSet class.
#ifndef REGISTERSET_H
#define REGISTERSET_H
#include "dword.hpp"
#include "../src/arch.h"

// RegisterSet class - controls the layout and display/input of 16 DWord
// objects representing the 16 registers of the CPU.
class RegisterSet : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterSet(QWidget *Parent = nullptr);
    uint32_t GetValue(int Index);
    void SetValue(int Index, uint32_t NewVal);
    void Lock();
    void Unlock();
    void EnableBitmaps(QPixmap *OffPic, QPixmap *OnPic);
    // No paint event for this widget!
private:
    bool Locked;
    QVBoxLayout *VL;
    DWord *Regs[NUMREGS];
};

#endif // REGISTERSET_H
