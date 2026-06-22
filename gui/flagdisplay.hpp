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

// flagdisplay.hpp - declarations for the FlagDisplay class
#ifndef FLAGDISPLAY_H
#define FLAGDISPLAY_H

#include <QWidget>
#include <QLabel>
#include <cstdint>
#include "indicator.hpp"
#include <QVBoxLayout>

#define NUM_FLAGS 8

// FlagDisplay class - a vertically-oriented display panel indicating the
// status of processor flags. Read-only.
class FlagDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit FlagDisplay(QWidget *parent = nullptr, bool SmallScreen = true);
    void SetValue(uint32_t NewVal);
    void EnableBitmaps(QPixmap *OffPic, QPixmap *OnPic);
    void SetHaltedState(bool State);


signals:

private:
    QVBoxLayout *VL;
    uint32_t Value;
    Indicator *Flags[NUM_FLAGS];
    QLabel *Titles[NUM_FLAGS];
};

#endif // FLAGDISPLAY_H
