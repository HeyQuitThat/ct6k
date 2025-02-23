/*
	The Comp-o-Tron 6000 software is Copyright (C) 2024 Mitch Williams.

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

// storotron.cpp - definitions for the Card-o-Tron 3CS emulator
#include "storotron.hpp"
#include <cstring>
#include <iostream>
#define SOT_STATUS_BASE ((SOT_NUM_HEADS << SOT_HEAD_COUNT_SHIFT) | \
						 (SOT_NUM_POS << SOT_POS_COUNT_SHIFT))

// Constructor
StorOTron::StorOTron(std::fstream *SOTFile)
{
	if ((SOTFile != nullptr) && SOTFile->is_open()) {
		DataFile=SOTFile;
		State = SOT_STATE_IDLE;
		Buffer = new uint32_t[SOT_BUFFER_LEN];
	} else {
		DataFile = nullptr;
		State = SOT_STATE_FAIL;
		Buffer = nullptr;
	}
}

//Destructor. Caller will close the file.
StorOTron::~StorOTron()
{
	delete[] Buffer;
}


// Get the required IO memory size. Used during registration to make sure we have enough space.
// (In real life, this is ignored by the CPU which always gives 64k words.)
uint32_t StorOTron::GetMemSize()
{
	return SOT_MEM_SIZE;
}

// Device class is used the UI to know how to handle the device
DeviceClass StorOTron::GetDeviceClass()
{
	return DC_RAS;
}

// Digital Device Name - used by apps to find the printer device
uint32_t StorOTron::GetDDN()
{
	return SOT_DDN;
}

// Defines how the device responds to memory reads by applications.
uint32_t StorOTron::ReadIOMem(uint32_t Offset)
{
	CheckTimer();
	switch (Offset) {
		case SOT_REG_STATUS:
			return SOT_STATUS_BASE | (uint32_t)State;
			break;
		case SOT_REG_COMMAND:
			return MEM_READ_INVALID;
			break;
		case SOT_REG_HEADSEL:
			return CurrentHead;
			break;
		case SOT_REG_POSSEL:
			return CurrentPos;
			break;
	  
		default:
			if ((Offset >= SOT_BUFFER) && (Offset < SOT_BUFFER + SOT_BUFFER_LEN)) {
					
					return (Buffer ? Buffer[Offset-SOT_BUFFER] : 0);
			}
			// Any other offset is invalid
			return MEM_READ_INVALID;
			break;
	}
}

// Defines how the device responds to memory writes by applications.
// Behavior described in hw.h
void StorOTron::WriteIOMem(uint32_t Offset, uint32_t Value)
{
	CheckTimer();
	switch (Offset) {
		case SOT_REG_COMMAND:
			if (State == SOT_STATE_IDLE) {
				State = SOT_STATE_BUSY;
				switch (Value) {
					case SOT_COMMAND_SEEK:
						StartTimer(SEEK_MSEC);
						break;
					case SOT_COMMAND_READ:
						StartTimer(READ_MSEC);
						ReadFromFile();
						break;
					case SOT_COMMAND_WRITE:
						StartTimer(WRITE_MSEC);
						WriteToFile();
						break;
					case SOT_COMMAND_RESET:
						PowerOnReset();
						break;
					defaut:
						// invalid command, ignored
						break;
				}
			}
			// if state is not idle, we do nothing
			break;
		case SOT_REG_HEADSEL:
			if ((State == SOT_STATE_IDLE) && (Value < SOT_NUM_HEADS))
				CurrentHead = Value;
			break;
		case SOT_REG_POSSEL:
			if ((State == SOT_STATE_IDLE) && (Value < SOT_NUM_POS))
				CurrentPos = Value;
			break;
		case SOT_REG_STATUS:
			break;
		default:
			if ((Offset >= SOT_BUFFER) && (Offset < SOT_BUFFER + SOT_BUFFER_LEN)) {
				Buffer[Offset-SOT_BUFFER] = Value;
			}
			// All others are read-only registers
			break;
	}
}


// Reset the device as though a power cycle had happened.
void StorOTron::PowerOnReset()
{
	if (DataFile != nullptr) {
		State = SOT_STATE_IDLE;
		CurrentHead = 0;
		CurrentPos = 0;
		if (Buffer)
            std::memset(Buffer, 0, SOT_SECTOR_BYTES);
	} else {
		State = SOT_STATE_FAIL;
	}
}

void StorOTron::StartTimer(uint32_t NumMsec)
{
	Start = std::chrono::steady_clock::now();
	MsecDelay = NumMsec;
}


void StorOTron::CheckTimer()
{
	auto Now = std::chrono::steady_clock::now();
	auto Since = std::chrono::duration_cast<std::chrono::milliseconds>(Now - Start);
	if (Since.count() > MsecDelay)
		State = SOT_STATE_IDLE;
}

void StorOTron::ReadFromFile()
{
    DataFile->seekg(CurrentHead * CurrentPos * SOT_SECTOR_BYTES);
    DataFile->read((char *)(Buffer), SOT_SECTOR_BYTES);
}

void StorOTron::WriteToFile()
{
    DataFile->seekp(CurrentHead * CurrentPos * SOT_SECTOR_BYTES);
    DataFile->write((char *)(Buffer), SOT_SECTOR_BYTES);
}
