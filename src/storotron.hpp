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

// storotron.hpp - class declaration for Stor-o-Tron Longitudinal Storage Device

#ifndef __STOROTRON_HPP__
#define __STOROTRON_HPP__
#include <cstdint>
#include <fstream>
#include <chrono>
#include "hw.h"
#include "periph.hpp"

#define READ_MSEC 17
#define WRITE_MSEC 17
#define SEEK_MSEC 200

#define SOT_SECTOR_BYTES (SOT_SECTOR_SIZE * CT6K_WORD_SIZE_BYTES)
#define SOT_FILE_SIZE (SOT_NUM_POS * SOT_NUM_HEADS * SOT_SECTOR_BYTES)

enum StorOTronState {
	SOT_STATE_IDLE = SOT_STATUS_READY,
	SOT_STATE_BUSY = SOT_STATUS_BUSY,
	SOT_STATE_FAIL = SOT_STATUS_ERR,
};

class StorOTron: public Periph {
public:
	StorOTron(std::fstream *SOTFile);
	~StorOTron();
	uint32_t GetMemSize(); // should be called before setting base
	uint32_t ReadIOMem(uint32_t Offset);
	void WriteIOMem(uint32_t Offset, uint32_t Value);
	DeviceClass GetDeviceClass();
	uint32_t GetDDN();
	void PowerOnReset();
	// for UI to display blinking lights
//    bool IsWorking();

private:
	StorOTronState State;
uint32_t *Buffer;
	uint32_t CurrentPos;
	uint32_t NextPos;
	uint32_t CurrentHead;
	uint32_t NextHead;
	// No current or next sector since it's always 0
	std::chrono::time_point<std::chrono::steady_clock> Start;
	uint32_t MsecDelay;
	std::fstream *DataFile {nullptr};
	void StartTimer(uint32_t NumMsec);
	void CheckTimer();
	void ReadFromFile();
	void WriteToFile();
};


#endif  // __STOROTRON_HPP__
