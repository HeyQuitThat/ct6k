// instruction.cpp - definitions for Instruction class
#include <cstdint>
#include <string>
#include <algorithm>
#include <cassert>
#include <climits>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <iostream>
#include "arch.h"
#include "instruction.hpp"
#include "cpu.hpp"
#include "memory.hpp"

using namespace std::string_literals;

RegisterArg::RegisterArg(uint8_t reg)
{
    regnum = reg & REGNUM_MASK;
    if (reg == REG_NULL) {
        type = rt_null;
        return;
    }
    if ((reg & REG_ERR) || !(reg & REG_VALID)) {
        type = rt_invalid; // fault on its way
    } else {
        switch (reg & REGTYPE_MASK) {
        case REG_UNUSED:
            type = rt_unused;
            break;
        case REG_IND:
            type = rt_indirect;
            break;
        case REG_VAL:
            type = rt_value;
            break;
        default:
            type = rt_invalid; // zero or more than one bit set, fault
            break;
        }
    }
};

_reg_type RegisterArg::get_type()
{
    return type;
};

uint8_t RegisterArg::get_num()
{
    return regnum;
};

bool RegisterArg::is_valid()
{
    return ((type == rt_indirect) || (type == rt_value));
}

void RegisterArg::print(std::string &outstr)
{
    if (type == rt_value) {
        outstr += "R";
        outstr += std::__cxx11::to_string(regnum);
    } else if (type == rt_indirect) {
        outstr += "I";
        outstr += std::__cxx11::to_string(regnum);
    } else {
        outstr += "ERROR";
    }
};



// We could use a STL list function here, instead of open-coding the search, but we need to be able
// to search on two different keys: uint8 if executing and string if assembling or debugging.
// Because of this, and because it's a short, static list, we will open-code the search functions
// and just use fixed-lenght c strings.

struct opmap opcode_map[] ={
{"MOVE", OP_MOVE, op_src_dest},
{"CMP", OP_CMP, op_src_dest},
{"ADD", OP_ADD, op_2src_dest},
{"SUB", OP_SUB, op_2src_dest},
{"NOT", OP_NOT, op_2src_dest},
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
{"HALT", OP_HALT, op_no_args},
{"NOP", OP_NOP, op_no_args},
{"SETFHAP", OP_SETFHAP, op_src_only},
{"SETIHAP", OP_SETIHAP, op_src_only},
{"", OP_INVALID, op_invalid},
};

opmap *find_from_opcode(uint8_t op)
{
    int i = 0;

    while (opcode_map[i].opcode > OP_INVALID) {
        if (op == opcode_map[i].opcode)
            break;
        i++;
    }
    return &opcode_map[i];
};

opmap *find_from_string(std::string instr)
{
    int i = 0;

    for(auto& c : instr)
        c = toupper(c);

    while (opcode_map[i].opcode) {
        if (instr.compare(opcode_map[i].name) == 0)
            return &opcode_map[i];
        i++;
    }
    return &opcode_map[i];
};

// functions for class Instruction

// constructor with u32 decodes from memory
Instruction::Instruction(uint32_t inst)
{
    raw = inst;
    opcode = GET_OP(inst);
    map = find_from_opcode(opcode); // TODO check for nullptr and set a bool to indicate raw data

    src1 = new RegisterArg(GET_SRC1(inst));
    src2 = new RegisterArg(GET_SRC2(inst));
    dest = new RegisterArg(GET_DEST(inst));
    // Check for direct value. This will fault when executed if opcode is not MOVE.
    // Actual direct value to be retrieved later when executed or printed.
    if (opcode == OP_MOVE)
        direct_val_in_use = (src1->get_type() == rt_null) && (src2->get_type() == rt_null);
    else
        direct_val_in_use = (map->type == op_control_flow) && (dest->get_type() == rt_null);

};


Instruction::Instruction(uint32_t inst, uint32_t prefetch)
{
    raw = inst;
    opcode = GET_OP(inst);
    map = find_from_opcode(opcode);

    src1 = new RegisterArg(GET_SRC1(inst));
    src2 = new RegisterArg(GET_SRC2(inst));
    dest = new RegisterArg(GET_DEST(inst));
    // Check for direct value. This will fault when executed if opcode is not MOVE.
    if (opcode == OP_MOVE)
        direct_val_in_use = (src1->get_type() == rt_null) && (src2->get_type() == rt_null);
    else
        direct_val_in_use = (map->type == op_control_flow) && (dest->get_type() == rt_null);

    direct_val = prefetch;
    direct_val_provided = true;

};


Instruction::~Instruction()
{
    delete src1;
    delete src2;
    delete dest;
};

bool Instruction::is_valid_instruction()
{
    bool retval {false};
    if (!is_raw)
        switch(map->type) {
        case op_no_args:
            retval = true;
            break;
        case op_src_only:
            retval = src1->is_valid();
            break;
        case op_src_dest:
            retval = dest->is_valid() && (src1->is_valid() || 
                            (src1->get_type() == rt_null && (src2->get_type() == rt_null)));
            break;
        case op_dest_only:
            retval = dest->is_valid();
            break;
        case op_control_flow:
            retval = dest->is_valid() || direct_val_in_use;
            break;
        case op_2src_dest:
            retval = src1->is_valid() && src2->is_valid() && dest->is_valid();
            break;
        default:
            break; // retval is false by default
        }
    return retval;

}

void Instruction::print_opstr(std::string &outstr)
{
    outstr += "\t";
    outstr += map->name;
    outstr += " ";
}

void Instruction::print(std::string &outstr)
{
    if (is_valid_instruction() == false) {
        // assume it's raw data
        outstr += (boost::format("\t0x%08X\n") % raw).str();
    } else {
        print_opstr(outstr);
        switch(map->type) {
        case op_no_args:
            break;
        case op_src_only:
            src1->print(outstr);
            break;
        case op_src_dest:
            if (direct_val_in_use)
                if (direct_val_provided)
                    outstr += (boost::format("0x%08X") % direct_val).str();
                else
                    outstr += "<direct data>";
            else    
                src1->print(outstr);
            outstr += ", ";
            dest->print(outstr);
            break;
        case op_dest_only:
            dest->print(outstr);
            break;
        case op_control_flow:
            if (direct_val_in_use)
                if (direct_val_provided)
                    outstr += (boost::format("0x%08X") % direct_val).str();
                else
                    outstr += "<direct data>";
            else    
                dest->print(outstr);
            break;
        case op_2src_dest:
            src1->print(outstr);
            outstr += ", ";
            src2->print(outstr);
            outstr += ", ";
            dest->print(outstr);
            break;
        default:
            // should never get here
            break;
        }
        outstr += "\n";
    }
    return;
};


uint32_t Instruction::output_binary(bool extra_word_present, uint32_t& extra_word)
{   
    if (direct_val_in_use && direct_val_provided) {
        extra_word_present = true;
        extra_word = direct_val;
    }
    return raw;
}

uint8_t Instruction::get_opcode()
{
    return opcode;
};

_op_type Instruction::get_type()
{
    return map->type;
};

RegisterArg Instruction::get_src1()
{
    return *src1;
};

RegisterArg Instruction::get_src2()
{
    return *src2;
};

RegisterArg Instruction::get_dest()
{
    return *dest;
};

bool Instruction::is_direct_val_instr()
{
    return direct_val_in_use;
};

bool Instruction::is_direct_val_present()    // only used when assembling or debugging
{
    return direct_val_in_use && direct_val_provided;
};

uint32_t Instruction::get_direct_val()
{
    return direct_val;
};


#define CHECK_TOKENS(_n) {if (num_tokens < (_n)) throw("Insufficient input");}

uint8_t build_reg(std::string regarg)
{
    uint8_t retval {0};
    uint32_t tmp;
    if (regarg[0] == 'R') {
        retval = REG_VAL;
    } else if (regarg[0] == 'I') {
        retval = REG_IND;
    } else {
        throw("Invalid argument");
    }
    if (!isdigit(regarg[1]))    // TODO handle IP, SP, FLG registers;
        throw("Invalid argument");
        
    tmp = std::strtoul(&regarg[1], nullptr, 10);
    if (tmp > 15)
        throw("Invalid argument");
    retval |= (uint8_t)(tmp & REGNUM_MASK);
    return retval;
};

typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

// build_instruction - quite possibly the worst parser in the history of the world.
// We can get away with this terrible excuse for a parser because the language
// is so very simple. No macros, no assemble-time math, and symbols get handled by the caller
// of this function.
// Can throw exception!
uint32_t build_instruction(std::string instring, uint32_t& extra_word, bool& extra_word_present)
{
    tokenizer tok{instring};
    uint32_t retval;
    tokenizer::iterator it;
    int num_tokens {0};
    opmap *map;
    extra_word_present = false;
    std::string tmpstr;

    for (it = tok.begin(); it != tok.end(); ++it)
        num_tokens++;
    CHECK_TOKENS(1);
    it = tok.begin();
    tmpstr = *it;
    if (isalpha(tmpstr[0])) {
        // possibly a keyword, possibly junk
        map = find_from_string(tmpstr);
        retval = OP_LOAD(map->opcode);
        switch (map->type) {
            case op_no_args:
                // done; retval is complete
                break;
            case op_src_only:
                CHECK_TOKENS(2);
                it++; // next token should be src reg
                retval |= S1_LOAD(build_reg(*it));
                break;
            case op_src_dest:
                CHECK_TOKENS(4); // instruction, src, separator, dest
                it++; // next token should be src reg
                tmpstr = *it;
                if (isdigit(tmpstr[0])) {
                    retval |= (S1_LOAD(REG_NULL) | S2_LOAD(REG_NULL));
                    extra_word = std::stoul(tmpstr,nullptr,0);
                    extra_word_present = true;
                } else
                    retval |= S1_LOAD(build_reg(*it));
                it++; // skip separator
                it++;
                retval |= DEST_LOAD(build_reg(*it));
                break;
            case op_dest_only:
                CHECK_TOKENS(2);
                it++; // next token should be dest reg
                retval |= DEST_LOAD(build_reg(*it));
                break;
            case op_control_flow:
                CHECK_TOKENS(2);
                it++; // next token should be dest reg or direct val
                tmpstr = *it;
                if (isdigit(tmpstr[0])) {
                    retval |= DEST_LOAD(REG_NULL);
                    extra_word = std::stoul(tmpstr,nullptr,0);
                    extra_word_present = true;
                } else
                    retval |= DEST_LOAD(build_reg(*it));
                break;
            case op_2src_dest:
                CHECK_TOKENS(6); // instruction, src1, separator, src2, separator, dest
                it++;
                retval |= S1_LOAD(build_reg(*it));
                it++; // skip separator
                it++;
                retval |= S2_LOAD(build_reg(*it));
                it++; // skip separator
                it++;
                retval |= DEST_LOAD(build_reg(*it));
                break;
            default:
                throw("Invalid instruction");
                break;
        }
    } else if (isdigit(tmpstr[0])) {
        // Easiest case - just raw data in decimal or hex format
        retval = std::stoul(tmpstr, nullptr, 0);
        // Testing with g++ shows that neither strtoul or stoul successfully catches
        // an overflow - errno never gets set. So this exception never throws.
        // Leaving the code in place anyway just in case another compiler does it differently.
        if (errno == ERANGE)
            throw("Invalid numeric value");
    }
    return retval;
};
