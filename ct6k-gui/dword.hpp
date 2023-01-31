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

// dword.hpp - defintions for the DWord class
#ifndef DWORD_H
#define DWORD_H
#include <cstdint>
#include "indicator.hpp"
#include <QHBoxLayout>
#include <QString>

// DWord class - displays/inputs individual bits of a 32-bit word using
// the Indicator class for each bit.
class DWord : public QWidget
{
    Q_OBJECT
public:
    explicit DWord(QWidget *Parent = nullptr, QString Name = "");
    uint32_t GetValue();
    void SetValue(uint32_t NewVal);
    void Lock();
    void Unlock();
    void EnableBitmaps(QPixmap *OffPic, QPixmap *OnPic);

    // No paint event for this widget! The individual bits paint themselves.
private:
    uint32_t Value;
    bool Locked;
    QHBoxLayout *LeLay;
    Indicator *Bits[32];
};

#endif // DWORD_H
