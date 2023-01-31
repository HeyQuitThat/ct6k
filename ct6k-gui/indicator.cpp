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

// indicator.cpp - function definitions for the Indicator class

#include "indicator.hpp"
#include <QPainter>

// Constructor
Indicator::Indicator(QWidget *parent)
    : QWidget{parent}
{
    OffImage = nullptr;
    OnImage = nullptr;
    State = false;
    IsLocked = true;
    setFixedSize(BASE_INDICATOR_SIZE, BASE_INDICATOR_SIZE);
}

// Return state of this bit. Used after an unlock/lock cycle to update
// the bit value to match the screen.
bool Indicator::GetState()
{
    return State;
}

// Change display state from external input.
void Indicator::SetState(bool NewState)
{
    bool redraw = (NewState != State);
    State = NewState;
    if (redraw)
        repaint();
}

// Prevent mouse clicks from changing state of the indicator
void Indicator::Lock()
{
    IsLocked = true;
}

// Allow mouse clicks to change indicator state
void Indicator::Unlock()
{
    IsLocked = false;
}

// Enable display of a picture, not just a block of color
void Indicator::EnableBitmaps(QPixmap *OffPic, QPixmap *OnPic)
{
    OffImage = OffPic;
    OnImage = OnPic;
}

// Actually display the indicator on the screen
void Indicator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (State) {
        if (OnImage != nullptr) {
            painter.drawPixmap(0, 0, this->width(), this->height(), *OnImage);
        } else {
            painter.setBrush(QColorConstants::Green);
            painter.drawRoundedRect(0, 0, this->width(), this->height(), 5.0, 5.0, Qt::AbsoluteSize);
        }
    } else {
        if (OffImage != nullptr) {
            painter.drawPixmap(0, 0, this->width(), this->height(), *OffImage);
        } else {
            painter.setBrush(QColorConstants::Red);
            painter.drawRoundedRect(0, 0, this->width(), this->height(), 5.0, 5.0, Qt::AbsoluteSize);
        }
    }
}

// Respond to a mouse click, change state if unlocked
void Indicator::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (IsLocked)
        return;
    // else
    State = !State;
    repaint();
    // caller should query after locking registers
}


