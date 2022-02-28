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

// instruction.hpp - declarations for Instruction class
#ifndef __INSTRUCTION_HPP__
#define __INSTRUCTION_HPP__

#include <cstdint>
#include <string>

// Register type
enum _reg_type {
    rt_unused,
    rt_indirect,
    rt_value,
    rt_invalid,
    rt_null, // special case for direct value
};

// Opcode type - used to determine what type of args the opcode takes
enum _op_type {
    op_invalid = 0,
    op_no_args,
    op_src_only,
    op_dest_only,
    op_control_flow, // uses destination register only, but also can use direct data
    op_src_dest,     // allows direct data as well
    op_2src_dest,
};

// This is the class that handles the actual register argument bytes in each instruction word
class RegisterArg {
public:
    RegisterArg(uint8_t);
    _reg_type GetType();
    uint8_t GetNum();
    bool IsValid();
    void Print(std::string &);

private:
    enum _reg_type Type;
    uint8_t RegNum;
};

#define MAX_INSTR_TEXT_LEN 9    // "UNSIGNED" + null

// Opcode map - allows lookups based on both name and opcode value
struct OpMap {
    const char Name[MAX_INSTR_TEXT_LEN];
    const uint8_t Opcode;
    const enum _op_type Type;
};

// The meat of the file - instantiated by the CPU for each instruction it executes
class Instruction {
public:
    Instruction(uint32_t, uint32_t); // extra prefetch value for immdiate addressing,
    Instruction(uint32_t);           // decodes upon init
    ~Instruction();
    void Print(std::string&);
    uint8_t GetOpcode();
    _op_type GetType();
    RegisterArg GetSrc1Reg();
    RegisterArg GetSrc2Reg();
    RegisterArg GetDestReg();
    bool IsDirectValInstr();
    bool IsDirectValPresent();
    uint32_t GetDirectVal();
    uint32_t SizeInMemory();


private:
    uint32_t Raw;   // used for data
    bool IsRaw {false};
    uint8_t Opcode;
    struct OpMap *Map;
    RegisterArg *Src1;
    RegisterArg *Src2;
    RegisterArg *Dest;
    uint32_t DirectVal {0};
    bool DirectValInUse {false};
    bool DirectValProvided {false};

    bool IsValidInstruction();
    void PrintOpstr(std::string &Out);

};

// Not part of the class, but can be used to build one
// Originally this was part of the class but it didn't feel great
// due to single responsibility principle. So now it's a helper
// function. Could it go back in the class? Sure, if it needs to.
uint32_t BuildInstruction(std::string In, uint32_t& ExtraWord, bool& ExtraWordPresent);

#endif // __INSTRUCTION_HPP__
