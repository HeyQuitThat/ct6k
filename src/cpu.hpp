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

// cpu.hpp - declarations for CPU class - the core of the Comp-o-Tron 6000
#ifndef __CPU_HPP__
#define __CPU_HPP__

#include <cstdint>
#include "arch.h"
#include "memory.hpp"
#include "instruction.hpp"
#include "periph.hpp"
#include "hw.h"

// Passed to emulator for printing - allows a single function call to get info instead of 19.
struct CPUInternalState {
    uint32_t Registers[NUMREGS];
    bool Halted;
    uint32_t FHAP_Base; // coding style violation but FHAPBase looks bad
    uint32_t IHAP_Base;
};

struct IORegion {
    PeriphMapEntry Entry;
    Periph *Owner;
};

// Core class that actually executes instructions.
class CPU {
public:
    CPU();
    ~CPU();
    void Step();
    CPUInternalState DumpInternalState();
    uint32_t ReadReg(uint8_t);
    void WriteReg(uint8_t, uint32_t);
    uint32_t ReadMem(uint32_t);
    void WriteMem(uint32_t, uint32_t);
    void SetFlag(uint32_t);
    void ClearFlag(uint32_t);
    void ClearMathFlags();
    bool IsFlagSet(uint32_t);
    void Set_FHAP(uint32_t);
    void Set_IHAP(uint32_t);
    void Fault(uint32_t);
    int PushState();
    int PopState();
    int PushWord(uint32_t);
    int PopWord(uint32_t &);
    void Halt();
    bool IsHalted();
    void Reset();
    bool AddDevice(Periph *Dev);
    void RemoveDevice(Periph *Dev);
    bool IsBroken() const;

private:
    Memory *Mem;
    uint32_t Reg[NUMREGS] {0};
    bool Running {true};
    uint32_t FHAP_Addr {0}; // Fault Handler Pointer
    uint32_t IHAP_Addr {0}; // Interrupt Handler Pointer
    Instruction *CurrentInst {nullptr};
    IORegion Devices[PERIPH_MAP_SIZE] {{{0,}, nullptr,},};    // Allocate separately?
    bool Broken {false};

    uint32_t Execute(); // executes current instruction, returns fault value
    uint32_t RetrieveDirectValue();
    uint32_t PutToDest(uint32_t);
    uint32_t GetFromReg(RegisterArg, uint32_t &);
    uint32_t ExecuteNoArgs();
    uint32_t ExecuteSrcDest();
    uint32_t ExecuteSrcOnly();
    uint32_t ExecuteDestOnly();
    uint32_t ExecuteControlFlow();
    uint32_t Execute2SrcDest();
    void IndicateZero(uint32_t);
    void IncrIP();
    uint32_t ReadIO(uint32_t);
    void WriteIO(uint32_t, uint32_t);
    int FindPeriphTableEntry(Periph *Dev);
};

#endif // __CPU_HPP__
