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

// indicator.hpp - declarations for the Indicator class
#ifndef INDICATOR_H
#define INDICATOR_H

#include <QWidget>

#define BASE_INDICATOR_SIZE 32

// Indicator class - displays and controls the status of an individual bit
// using either a colored box or a bitmap. The bitmap is passed in at runtime.
class Indicator : public QWidget
{
    Q_OBJECT
public:
    explicit Indicator(QWidget *Parent = nullptr);
    bool GetState();
    void SetState(bool NewState);
    void Lock();
    void Unlock();
    void EnableBitmaps(QPixmap *OffPic, QPixmap *OnPic);
    void mouseReleaseEvent(QMouseEvent *event) override;
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    bool State;
    bool IsLocked;
    QPixmap *OffImage;
    QPixmap *OnImage;
signals:

};

#endif // INDICATOR_H
