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

// cardotron.cpp - definitions for the Card-o-Tron 3CS emulator
#include "cardotron.hpp"
#include "hw.h"

// ------------------------------------------ Scanner side _________________________________________

// Constructor
CardOTronScan::CardOTronScan()
{
    ReadBuf = new uint32_t[MAX_CARD_LEN];
    StatusReg = COTS_STATUS_EMPTY;
}

//Destructor
CardOTronScan::~CardOTronScan()
{
    if ((InFile != nullptr) && InFile->is_open())
        InFile->close();
    delete[] ReadBuf;
}


// Get the required IO memory size. Used during registration to make sure we have enough space.
// (In real life, this is ignored by the CPU which always gives 64k words.)
uint32_t CardOTronScan::GetMemSize()
{
    return COTS_MEM_SIZE;
}

// Device class is used the UI to know how to handle the device
DeviceClass CardOTronScan::GetDeviceClass()
{
    return DC_CARD_READER;
}

// Digital Device Name - used by apps to find the printer device
uint32_t CardOTronScan::GetDDN()
{
    return COTS_DDN;
}

// Defines how the device responds to memory reads by applications.
uint32_t CardOTronScan::ReadIOMem(uint32_t Offset)
{
    CheckReadTimer();
    switch (Offset) {
        case COTS_REG_STATUS:
            return StatusReg;
            break;
        case COTS_REG_CARD_INFO:
            if (!Reading)
                return CardInfoReg;
            else
                return 0;
            break;
        case COTS_REG_COMMAND:
            return 0xffffffff;
            break;
        default:
            if ((Offset >= COTS_REG_READ_BUF) && (Offset < COTS_REG_READ_BUF + COTS_READ_BUF_LEN)) {
                if (!Reading)
                    return ReadBuf[Offset - COTS_REG_READ_BUF];
                else
                    return 0;
            }
            // Any other offset is invalid
            return 0xffffffff;
            break;
    }
}

// Defines how the device responds to memory writes by applications.
// Behavior described in hw.h
void CardOTronScan::WriteIOMem(uint32_t Offset, uint32_t Value)
{
    CheckReadTimer();
    switch (Offset) {
        case COTS_REG_COMMAND:
            if (Value & COTS_CMD_READ) {
                if ((StatusReg & COTS_STATUS_READY) == COTS_STATUS_READY)
                    ReadNextCard();
            } else if (Value & COTS_CMD_ABORT) {
                PowerOnReset();
            }
            break;
        case COTS_REG_STATUS:
        case COTS_REG_CARD_INFO:
        case COTS_REG_READ_BUF:
        default:
            // Read-only registers
            break;
    }
}


// Reset the device as though a power cycle had happened.
void CardOTronScan::PowerOnReset()
{
    if ((InFile != nullptr) && InFile->is_open())
        InFile->close();
    Reading = false;
    StatusReg = COTS_STATUS_EMPTY;
}

// Get a ref to the input file. The must already be open for
// reading and must be positioned at the start.
void CardOTronScan::SetInFile(std::ifstream *File)
{
    if ((File != nullptr) && File->is_open())
        InFile = File;
    else
        InFile = nullptr;
    StatusReg = COTS_STATUS_READY;
}

bool CardOTronScan::IsReading()
{
    CheckReadTimer();
    return Reading;
}

void CardOTronScan::ReadNextCard()
{

    if ((StatusReg & COTS_STATUS_READY) != COTS_STATUS_READY)
        return;
    ReadStart = std::chrono::steady_clock::now();
    Reading = true;
    StatusReg = COTS_STATUS_READING;
    InFile->exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
    try {
        InFile->ignore(std::numeric_limits<std::streamsize>::max(), '<');
    } catch (std::ifstream::failure &e) {
        InFile->close();
        Reading = false;
        if (InFile->eof()) {
            // this is OK, we're out of cards
            StatusReg = COTS_STATUS_COMPLETE;
            return;
        } else {
            StatusReg = COTS_STATUS_ERR_MECH;
            return;
        }
    }
    try {
        char TypeFlag;
        *InFile >> TypeFlag;
        switch (TypeFlag) {
            case 'C':
                CardInfoReg = COTS_INFO_CODE;
                break;
            case 'L':
                CardInfoReg = COTS_INFO_TXTPL;
                break;
            case 'M':
                CardInfoReg = COTS_INFO_TXTPM;
                break;
            case 'U':
                CardInfoReg = COTS_INFO_TXTU;
                break;
            case 'B':
                CardInfoReg = COTS_INFO_BIN;
                break;
            default:
                CardInfoReg = 0;
                throw 0;
                break;
        }
        InFile->ignore(1, '>');
        uint32_t Len;
        *InFile >> std::dec >> Len;
        if (Len > MAX_CARD_LEN) {
            throw 0;
        }
        CardInfoReg |= (Len & COTS_INFO_LEN_MASK);
        for (unsigned int i = 0 ; i < Len ; i++) {
            *InFile >> std::hex >> ReadBuf[i];
        }
    } catch (std::ifstream::failure &e) {
        InFile->close();
        Reading = false;
        StatusReg = COTS_STATUS_ERR_CSUM;
    }
}

void CardOTronScan::CheckReadTimer()
{
    if (Reading) {
        auto ReadNow = std::chrono::steady_clock::now();
        auto Since = std::chrono::duration_cast<std::chrono::milliseconds>(ReadNow - ReadStart);
        if (Since.count() > SCAN_MSEC) {
            Reading = false;
            StatusReg = COTS_STATUS_READY | COTS_STATUS_COMPLETE;
        }
    }
}
// ------------------------------------------ Puncher side _________________________________________
// Constructor
CardOTronPunch::CardOTronPunch()
{
    WriteBuf = new uint32_t[MAX_CARD_LEN];
    StatusReg = COTP_STATUS_EMPTY;
}

//Destructor
CardOTronPunch::~CardOTronPunch()
{
    if ((OutFile != nullptr) && OutFile->is_open())
        OutFile->close();
    delete[] WriteBuf;
}

// Get the required IO memory size. Used during registration to make sure we have enough space.
// (In real life, this is ignored by the CPU which always gives 64k words.)
uint32_t CardOTronPunch::GetMemSize()
{
    return COTP_MEM_SIZE;
}

// Device class is used the UI to know how to handle the device
DeviceClass CardOTronPunch::GetDeviceClass()
{
    return DC_CARD_PUNCH;
}

// Digital Device Name - used by apps to find the printer device
uint32_t CardOTronPunch::GetDDN()
{
    return COTP_DDN;
}

// Defines how the device responds to memory writes by applications.
// Behavior described in hw.h
void CardOTronPunch::WriteIOMem(uint32_t Offset, uint32_t Value)
{
    CheckWriteTimer();
    if (Writing)
        return;
    switch (Offset) {
        case COTP_REG_STATUS:
            // read only
            break;
        case COTP_REG_COMMAND:
            if (Value & COTP_CMD_WRITE)
                WriteCard();
            else if (Value & COTP_CMD_FLUSH)
                PowerOnReset();
            break;
        case COTP_REG_CARD_INFO:
            InfoReg = Value;
            break;
        default:
            if ((Offset >= COTP_REG_WRITE_BUF) && (Offset < COTP_REG_WRITE_BUF + COTP_WRITE_BUF_LEN))
                WriteBuf[Offset - COTP_REG_WRITE_BUF] = Value;
            // Ignore other values
            break;
    }
}

// Defines how the device responds to memory reads by applications.
uint32_t CardOTronPunch::ReadIOMem(uint32_t Offset)
{
    CheckWriteTimer();
    if (Offset == COTP_REG_STATUS)
        return StatusReg;
    else
        return 0xffffffff;
}

// Reset the device as though a power cycle had happened.
void CardOTronPunch::PowerOnReset()
{
    if ((OutFile != nullptr) && (OutFile->is_open()))
        OutFile->close();
    Writing = false;
    StatusReg = COTP_STATUS_EMPTY;
}

void CardOTronPunch::SetOutFile(std::ofstream *File)
{
    if ((File != nullptr) && (File->is_open()))  {
        OutFile = File;
        StatusReg = COTP_STATUS_READY;
        *OutFile << std::showbase;
    } else
        OutFile = nullptr;
}

bool CardOTronPunch::IsPunching()
{
    CheckWriteTimer();
    return Writing;
}

void CardOTronPunch::WriteCard()
{
    if (StatusReg != COTP_STATUS_READY)
        return;
    WriteStart = std::chrono::steady_clock::now();
    Writing = true;
    StatusReg = COTP_STATUS_BUSY;
    // Write type, surrounded by brackets
    *OutFile << '<';
    switch (InfoReg & COTP_INFO_TYPE_MASK) {
        case COTP_INFO_CODE:
            *OutFile << 'C';
            break;
        case COTP_INFO_TXTPL:
            *OutFile << 'L';
            break;
        case COTP_INFO_TXTPM:
            *OutFile << 'M';
            break;
        case COTP_INFO_TXTU:
            *OutFile << 'U';
            break;
        case COTP_INFO_BIN:
        default:
            *OutFile << 'B';
            break;
    }
    *OutFile << "> ";
    // Write length, in decimal
    *OutFile << std::dec << (InfoReg & COTP_INFO_LEN_MASK) << '\n';
    *OutFile << std::hex;
    for (unsigned int i = 0; i < (InfoReg & COTP_INFO_LEN_MASK); i++) {
        *OutFile <<  WriteBuf[i];
        if ((i > 0) && ((i % 8) == 0))
            *OutFile << '\n';
        else
            *OutFile << ' ';
    }
    *OutFile << '\n';
}

void CardOTronPunch::CheckWriteTimer()
{
    if (Writing) {
        auto WriteNow = std::chrono::steady_clock::now();
        auto Since = std::chrono::duration_cast<std::chrono::milliseconds>(WriteNow - WriteStart);
        if (Since.count() > PUNCH_MSEC) {
            Writing = false;
            StatusReg = COTP_STATUS_READY;
        }
    }
}
