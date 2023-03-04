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

// cotwindow.hpp - declarations for the COTWindow class

#ifndef COTWINDOW_HPP
#define COTWINDOW_HPP

#include <QObject>
#include <QLabel>
#include <QDialog>
#include <QPixmap>
#include <fstream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "indicator.hpp"

class COTWindow : public QObject
{
    Q_OBJECT
public:
    explicit COTWindow(QObject *parent = nullptr);
    ~COTWindow();
    void Show();
    void Hide();
    // Buttons need to be public so we can connect them to actions
    QPushButton *PBP;
    QPushButton *PBS;

public slots:
    // Slot from CPU Spinner
    void UpdateBlinkyLights(bool Writing, bool Reading);
    // Slots for buttons
    void OpenInputFile();
    void OpenOutputFile();

signals:
    // Signals to CPU Worker
    void SetCOTSInput(std::ifstream *InFile);
    void SetCOTPOutput(std::ofstream *OutFile);

private:
    QDialog *COTBox;
    std::ofstream *PunchOut {nullptr};
    std::ifstream *ScanIn {nullptr};
    QPixmap *OnImg;
    QPixmap *OffImg;
    QVBoxLayout *VLP;  // Vertical layout for punch
    QVBoxLayout *VLS;  // Vertical layout for scanner
    QHBoxLayout *HL;   // Horizontal layout for the whole dang thing
    Indicator *IndP;
    Indicator *IndS;
    Indicator *EmptyP;
    Indicator *EmptyS;

};

#endif // COTWINDOW_HPP
