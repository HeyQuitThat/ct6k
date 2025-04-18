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

// instruction.cpp - definitions for Instruction class
#include <cstdint>
#include <cctype>
#include <string>
#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <iomanip>
#include "arch.h"
#include "instruction.hpp"

using namespace std::literals::string_literals;

// Constructor for RegisterArg class
// Takes a uint8 and decodes it into a register argument, ready for processing
RegisterArg::RegisterArg(uint8_t Reg)
{
    RegNum = Reg & REGNUM_MASK;
    // Special case for direct value instructions; not decoded further. Indicates need for CPU to fetch next
    // word from memory to act as the argument to the function.
    if (Reg == REG_NULL) {
        Type = rt_null;
        return;
    }

    if ((Reg & REG_ERR) || !(Reg & REG_VALID)) {
        Type = rt_invalid;
    } else {
        switch (Reg & REGTYPE_MASK) {
        case REG_UNUSED:
            Type = rt_unused;
            break;
        case REG_IND:
            Type = rt_indirect;
            break;
        case REG_VAL:
            Type = rt_value;
            break;
        default:
            Type = rt_invalid;
            break;
        }
    }
};

// Getter functions for register type, number, validity. There are no setters, these values are not changed
// after construction.
_reg_type RegisterArg::GetType()
{
    return Type;
};

uint8_t RegisterArg::GetNum()
{
    return RegNum;
};

bool RegisterArg::IsValid()
{
    return ((Type == rt_indirect) || (Type == rt_value));
}

// Prints the register argument. Called in context of decoding and printing the entire instruction.
void RegisterArg::Print(std::string &Out)
{
    if (Type == rt_value) {
        Out += "R";
        Out += std::to_string(RegNum);
    } else if (Type == rt_indirect) {
        Out += "I";
        Out += std::to_string(RegNum);
    } else {
        Out += "ERROR";
    }
};



// We could use an STL list function here, instead of open-coding the search, but we need to be able
// to search on two different keys: uint8 if executing and string if assembling or debugging.
// Because of this, and because it's a short, static list, we will open-code the search functions
// and just use fixed-length C strings.

OpMap opcode_map[] = {
{"MOVE", OP_MOVE, op_src_dest},
{"CMP", OP_CMP, op_src_dest},
{"ADD", OP_ADD, op_2src_dest},
{"SUB", OP_SUB, op_2src_dest},
{"NOT", OP_NOT, op_dest_only},
{"AND", OP_AND, op_2src_dest},
{"OR", OP_OR, op_2src_dest},
{"XOR", OP_XOR, op_2src_dest},
{"SHIFTR", OP_SHIFTR, op_2src_dest},
{"SHIFTL", OP_SHIFTL, op_2src_dest},
{"PUSH", OP_PUSH, op_src_only},
{"POP", OP_POP, op_dest_only},
{"INCR", OP_INCR, op_dest_only},
{"DECR", OP_DECR, op_dest_only},
{"SSTATE", OP_SSTATE, op_no_args},
{"LSTATE", OP_LSTATE, op_no_args},
{"JZERO", OP_JZERO, op_control_flow},
{"JNZERO", OP_JNZERO, op_control_flow},
{"JOVER", OP_JOVER, op_control_flow},
{"JNOVER", OP_JNOVER,op_control_flow },
{"JUNDER", OP_JUNDER, op_control_flow},
{"JNUNDER", OP_JNUNDER, op_control_flow},
{"JMP", OP_JMP, op_control_flow},
{"CALL", OP_CALL, op_control_flow},
{"RETURN", OP_RETURN, op_no_args},
{"IRET", OP_IRET, op_no_args},
{"SIGNED", OP_SIGNED, op_no_args},
{"UNSIGNED", OP_UNSIGNED, op_no_args},
{"INTENA", OP_INTENA, op_no_args},
{"INTDIS", OP_INTDIS, op_no_args},
{"BRK", OP_BRK, op_no_args},
{"HALT", OP_HALT, op_no_args},
{"NOP", OP_NOP, op_no_args},
{"SETFHAP", OP_SETFHAP, op_src_only},
{"SETIHAP", OP_SETIHAP, op_src_only},
{"", OP_INVALID, op_invalid},
};

// Given an opcode, find the map. This lets us print the name and tells us the type so we can
// execute the opcode.
OpMap *FindFromOpcode(uint8_t Op)
{
    int i = 0;

    while (opcode_map[i].Opcode > OP_INVALID) {
        if (Op == opcode_map[i].Opcode)
            break;
        i++;
    }
    return &opcode_map[i];
};

// Given a name, find the map. This is used for assembly.
OpMap *FindFromString(std::string Instr)
{
    int i = 0;
    std::string tmpstr;

    // Convert input string to uppercase without destroying it.
    for(auto& c : Instr)
        tmpstr += toupper(c);

    while (opcode_map[i].Opcode) {
        if (tmpstr.compare(opcode_map[i].Name) == 0)
            return &opcode_map[i];
        i++;
    }
    return &opcode_map[i];
};

// Functions for class Instruction

// Constructor from single word - used during execution. If the instruction calls for a direct value,
// the CPU will fetch it as it executes.
Instruction::Instruction(uint32_t Inst)
{
    Raw = Inst;
    Opcode = GET_OP(Inst);
    Map = FindFromOpcode(Opcode);

    Src1 = new RegisterArg(GET_SRC1(Inst));
    Src2 = new RegisterArg(GET_SRC2(Inst));
    Dest = new RegisterArg(GET_DEST(Inst));
    // Check for direct value. This will fault when executed if opcode doesn't support direct data.
    // Actual direct value to be retrieved later when executed or printed.
    if ((Opcode == OP_MOVE) || (Opcode == OP_CMP))
        DirectValInUse = (Src1->GetType() == rt_null) && (Src2->GetType() == rt_null);
    else
        DirectValInUse = (Map->Type == op_control_flow) && (Dest->GetType() == rt_null);

};

// Constructor with two words - used for disassembly and printing. The second word is provided
// opportunistically, in case a direct value is required.
Instruction::Instruction(uint32_t Inst, uint32_t Prefetch)
{
    Raw = Inst;
    Opcode = GET_OP(Inst);
    Map = FindFromOpcode(Opcode);

    Src1 = new RegisterArg(GET_SRC1(Inst));
    Src2 = new RegisterArg(GET_SRC2(Inst));
    Dest = new RegisterArg(GET_DEST(Inst));
    // Check for direct value. This will fault when executed if opcode doesn't support direct data.
    if ((Opcode == OP_MOVE) || (Opcode == OP_CMP))
        DirectValInUse = (Src1->GetType() == rt_null) && (Src2->GetType() == rt_null);
    else
        DirectValInUse = (Map->Type == op_control_flow) && (Dest->GetType() == rt_null);

    DirectVal = Prefetch;
    DirectValProvided = true;

};

// Destructor.
Instruction::~Instruction()
{
    delete Src1;
    delete Src2;
    delete Dest;
};

// Test validity of the instruction, based on its type and the register arguments presented.
// Attempting to execute an invalid instruction will fault!
bool Instruction::IsValidInstruction()
{
    bool retval {false};
    if (!IsRaw)
        switch(Map->Type) {
        case op_no_args:
            retval = true;
            break;
        case op_src_only:
            retval = Src1->IsValid();
            break;
        case op_src_dest:
            retval = Dest->IsValid() && (Src1->IsValid() ||
                            (Src1->GetType() == rt_null && (Src2->GetType() == rt_null)));
            break;
        case op_dest_only:
            retval = Dest->IsValid();
            break;
        case op_control_flow:
            retval = Dest->IsValid() || DirectValInUse;
            break;
        case op_2src_dest:
            retval = Src1->IsValid() && Src2->IsValid() && Dest->IsValid();
            break;
        default:
            break; // retval is false by default
        }
    return retval;

}

// Print the name of the opcode. Called in context of printing the entire instruction.
void Instruction::PrintOpstr(std::string &Out)
{
    Out += Map->Name;
    Out += " ";
}

// Utility function. Add a formatted hexadecimal digit to the given string.
// Output will be as if we used 0x%8.8x in C
void AddHexValue(std::string &Line, uint32_t Value)
{
    std::stringstream buffer;

    buffer << std::showbase;
    buffer << std::hex;
    buffer << std::setw(8);
    buffer << Value;
    Line += buffer.str();
}

// Print the entire instruction, including register arguments and direct value if available.
void Instruction::Print(std::string &Out)
{
    if (IsValidInstruction() == false) {
        // assume it's raw data
        AddHexValue(Out, Raw);
    } else {
        PrintOpstr(Out);
        switch(Map->Type) {
        case op_no_args:
            break;
        case op_src_only:
            Src1->Print(Out);
            break;
        case op_src_dest:
            if (DirectValInUse)
                if (DirectValProvided)
                    AddHexValue(Out, DirectVal);
                else
                    Out += "<direct data>";
            else
                Src1->Print(Out);
            Out += ", ";
            Dest->Print(Out);
            break;
        case op_dest_only:
            Dest->Print(Out);
            break;
        case op_control_flow:
            if (DirectValInUse)
                if (DirectValProvided)
                    AddHexValue(Out, DirectVal);
                else
                    Out += "<direct data>";
            else
                Dest->Print(Out);
            break;
        case op_2src_dest:
            Src1->Print(Out);
            Out += ", ";
            Src2->Print(Out);
            Out += ", ";
            Dest->Print(Out);
            break;
        default:
            // should never get here
            break;
        }
    }
    return;
};

// Getter functions. Like with Register class, there are no setters; these values are not changed at runtime.
uint8_t Instruction::GetOpcode()
{
    return Opcode;
};

_op_type Instruction::GetType()
{
    return Map->Type;
};

RegisterArg Instruction::GetSrc1Reg()
{
    return *Src1;
};

RegisterArg Instruction::GetSrc2Reg()
{
    return *Src2;
};

RegisterArg Instruction::GetDestReg()
{
    return *Dest;
};

bool Instruction::IsDirectValInstr()
{
    return DirectValInUse;
};

bool Instruction::IsDirectValPresent()    // only used when assembling or debugging
{
    return DirectValInUse && DirectValProvided;
};

uint32_t Instruction::GetDirectVal()
{
    return DirectVal;
};

uint32_t Instruction::SizeInMemory()
{
    if (DirectValInUse && DirectValProvided)
        return 2;
    else
        return 1;
}

// Build binary register argument based on string input.
// This function can throw an exception!
uint8_t BuildReg(std::string RegArg)
{
    uint8_t retval {0};
    uint32_t tmp;

    if (RegArg.empty())
        throw("Invalid argument");
    if (RegArg[0] == 'R') {
        retval = REG_VAL;
    } else if (RegArg[0] == 'I') {
        retval = REG_IND;
    } else {
        throw("Invalid argument");
    }
    RegArg.erase(0,1);
    // check for aliases
    if (RegArg == "IP") {
        retval |= REG_IP;
        return retval;
    }
    if (RegArg == "SP") {
        retval |= REG_SP;
        return retval;
    }
    if (RegArg == "FLG") {
        retval |= REG_FLG;
        return retval;
    }
    if (!isdigit(RegArg[0]))
        throw("Invalid argument");

    tmp = std::strtoul(RegArg.c_str(), nullptr, 10);
    if (tmp > 15)
        throw("Invalid argument");
    retval |= (uint8_t)(tmp & REGNUM_MASK);
    return retval;
};


// Get next token from the In string, place it in Out. Erase In string up to end of
// token. Tokens are simple combinations of letters and numbers. All other characters
// are skipped.
void GetNextToken(std::string &In, std::string &Out)
{
    unsigned int i {0};
    bool found {false};

    Out.clear();
    while (i < In.length()) {
        unsigned char tmp = In[i++];

        if (isalnum(tmp)) {
            found = true;
            Out += tmp;
        } else {
            if (found)
                break;
        }
    }
    In.erase(0, i);

}

// BuildInstruction - quite possibly the worst parser in the history of the world.
// We can get away with this terrible excuse for a parser because the language
// is so very simple. No macros, no assemble-time math, and symbols get handled by the caller
// of this function.
// Can throw exception!
uint32_t BuildInstruction(std::string In, uint32_t &ExtraWord, bool &ExtraWordPresent)
{
    uint32_t retval {0};
    OpMap *map;
    std::string tmp;

    ExtraWordPresent = false;
    GetNextToken(In, tmp);
    if (tmp.empty())
        return retval;

    if (isalpha(tmp[0])) {
        // Possibly a keyword, possibly junk. If an error happens while processing
        // register arguments, the BuildReg() function will throw an exception, which
        // we will not catch here. Thus the caller will catch it and deal accordingly.
        map = FindFromString(tmp);
        retval = OP_LOAD(map->Opcode);
        switch (map->Type) {
            case op_no_args:
                // done; retval is complete
                break;
            case op_src_only:
                GetNextToken(In, tmp);
                retval |= S1_LOAD(BuildReg(tmp));
                break;
            case op_src_dest:
                GetNextToken(In, tmp);
                if (isdigit(tmp[0])) {
                    retval |= (S1_LOAD(REG_NULL) | S2_LOAD(REG_NULL));
                    ExtraWord = std::stoul(tmp, nullptr, 0);
                    ExtraWordPresent = true;
                } else
                    retval |= S1_LOAD(BuildReg(tmp));
                GetNextToken(In, tmp);
                retval |= DEST_LOAD(BuildReg(tmp));
                break;
            case op_dest_only:
                GetNextToken(In, tmp);
                retval |= DEST_LOAD(BuildReg(tmp));
                break;
            case op_control_flow:
                GetNextToken(In, tmp);
                if (isdigit(tmp[0])) {
                    retval |= DEST_LOAD(REG_NULL);
                    ExtraWord = std::stoul(tmp,nullptr,0);
                    ExtraWordPresent = true;
                } else
                    retval |= DEST_LOAD(BuildReg(tmp));
                break;
            case op_2src_dest:
                GetNextToken(In, tmp);
                retval |= S1_LOAD(BuildReg(tmp));
                GetNextToken(In, tmp);
                retval |= S2_LOAD(BuildReg(tmp));
                GetNextToken(In, tmp);
                retval |= DEST_LOAD(BuildReg(tmp));
                break;
            default:
                throw("Invalid instruction");
                break;
        }
    } else if (isdigit(tmp[0])) {
        // Easiest case - just raw data in decimal or hex format
        retval = std::stoul(tmp, nullptr, 0);
        // Testing with g++ shows that neither strtoul or stoul successfully catches
        // an overflow - errno never gets set. So this exception never throws.
        // Leaving the code in place anyway just in case another compiler does it differently.
        if (errno == ERANGE)
            throw("Invalid numeric value");
    } else {
        throw("No valid instruction found");
    }
    return retval;
};
