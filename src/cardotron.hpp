/*
    The Comp-o-Tron 6000 software is Copyright (C) 2022 Mitch Williams.

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

// cardotron.cpp - class declaration for Card-o-Tron 3CS combination card scanner/puncher

#ifndef __CARDOTRON_HPP__
#define __CARDOTRON_HPP__
#include <cstdint>
#include <fstream>
#include <string>
#include <chrono>
#include "periph.hpp"

#define MAX_CARD_LEN 32
#define SCAN_MSEC 100
#define PUNCH_MSEC 200

class CardOTronScan: public Periph {
public:
    CardOTronScan();
    ~CardOTronScan();
    uint32_t GetMemSize(); // should be called before setting base
    uint32_t ReadIOMem(uint32_t Offset);
    void WriteIOMem(uint32_t Offset, uint32_t Value);
    DeviceClass GetDeviceClass();
    uint32_t GetDDN();
    void PowerOnReset();
    void SetInFile(std::ifstream *File);  // load punched cards into hopper
    // for UI to display blinking lights
    bool IsReading();

private:
    uint32_t StatusReg;
    uint32_t *ReadBuf;
    uint32_t CardInfoReg;
    uint8_t ReadBufSize;
    bool Reading;
    std::chrono::time_point<std::chrono::steady_clock> ReadStart;
    std::ifstream *InFile;
    void ReadNextCard();
    void CheckReadTimer();
};

class CardOTronPunch: public Periph {
public:
    CardOTronPunch();
    ~CardOTronPunch();
    uint32_t GetMemSize(); // should be called before setting base
    uint32_t ReadIOMem(uint32_t Offset);
    void WriteIOMem(uint32_t Offset, uint32_t Value);
    DeviceClass GetDeviceClass();
    uint32_t GetDDN();
    void PowerOnReset();
    void SetOutFile(std::ofstream *File); // load blank cards into hopper
    // for UI to display blinking lights
    bool IsPunching();

private:
    uint32_t *WriteBuf;
    uint32_t StatusReg;
    uint32_t InfoReg;
    bool Writing;
    std::chrono::time_point<std::chrono::steady_clock> WriteStart;
    std::ofstream *OutFile; // file should be open before calling SetOutFile
    void WriteCard();
    void CheckWriteTimer();
};


#endif  // __CARDOTRON_HPP__
