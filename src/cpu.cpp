// cpu.cpp - definitions for CPU class
// First successful code execution: October 24, 2021
// First loop executed November 4, 2021

#include <cstdint>
#include <new>
#include <cassert>
#include <climits>
#include "arch.h"
#include "cpu.hpp"

CPU::CPU()
{
    mem = new Memory(); /* using DEFAULT_SIZE */
};
CPU::~CPU()
{
    delete mem;
};

uint32_t CPU::read_reg(uint8_t reg_index)
{
    assert (reg_index < NUMREGS);
    return reg[reg_index];
};

void CPU::write_reg(uint8_t reg_index, uint32_t value)
{
    assert (reg_index < NUMREGS);
    reg[reg_index] = value;
}
void CPU::incr_IP()
{
    reg[REG_IP]++;
};

cpu_internal_state CPU::dump_internal_state()
{
    cpu_internal_state retval;
    for (int i = 0; i < NUMREGS; i++)
        retval.registers[i] = reg[i];
    retval.halted = !running;
    retval.FHAP_base = FHAPAddr;
    retval.IHAP_base = IHAPAddr;
    return retval;
}
// no bounds checking for memory access, address space is 32 bits.
// reads beyond populated memory return all F
// writes beyond populated memory are sent to the 12th dimension, never to be seen again
uint32_t CPU::read_mem(uint32_t address)
{
    return mem->MemRead(address);
};

void CPU::write_mem(uint32_t address, uint32_t value)
{
    mem->MemWrite(address, value);
};

void CPU::set_flag(uint32_t flag)
{
    uint32_t tmp = read_reg(REG_FLG);
    tmp |= flag;
    write_reg(REG_FLG, tmp);
};

void CPU::clear_flag(uint32_t flag)
{
    uint32_t tmp = read_reg(REG_FLG);
    tmp &= ~flag;
    write_reg(REG_FLG, tmp);
};

void CPU::clear_math_flags()
{
    uint32_t tmp = read_reg(REG_FLG);
    tmp &= ~(FLG_OVER | FLG_UNDER | FLG_ZERO);
    write_reg(REG_FLG, tmp);
};

void CPU::indicate_zero(uint32_t val)
{
        if (val == 0)
            set_flag(FLG_ZERO);
        else
            clear_flag(FLG_ZERO);
}

bool CPU::is_flag_set(uint32_t flag)
{
    uint32_t tmp = read_reg(REG_FLG);
    return !!(tmp & flag);
};


void CPU::set_FHAP(uint32_t addr)
{
    FHAPAddr = addr;
};

void CPU::set_IHAP(uint32_t addr)
{
    // error checking in instruction handling, should it move here?
    IHAPAddr = addr;
};

// This is where things actually happen! Simulates a single clock cycle of the processor.
void CPU::step()
{
    if (!running)
        // we are halted; don't do anything
        return;
    // TODO check for interrupts here
    uint32_t iaddr = read_reg(REG_IP);
    incr_IP();
    CurrentInst = new Instruction(read_mem(iaddr));
    uint32_t ftype = execute();
    delete CurrentInst;
    if (ftype)
        fault(ftype);
};

void CPU::fault(uint32_t fault_type)
{
    uint32_t newIP;

    if (is_flag_set(FLG_FAULT)) {
        // already in a fault, this is a double-fault
        halt();
        return;
    }
    set_flag(FLG_FAULT);
    push_state();
    newIP = read_mem(FHAPAddr + fault_type);
    write_reg(REG_IP, newIP);
};

int CPU::push_state()
{
    // All registers pushed to stack, including SP, then SP updated. Fault if we hit top of memory.
    // Bad Things will happen if SP + space for 16 words are not located in populated memory, but not a fault.
    // No flags are updated here.
    uint32_t mem_base = read_reg(REG_SP);
    if (mem_base > MAX_STATE_PUSH) { // no space for state
        return FAULT_STACK;
    }
    for (int i = 0; i < NUMREGS; i++)
        write_mem(mem_base + i, read_reg(i));
    write_reg(REG_SP, mem_base + NUMREGS);
    return FAULT_NO_FAULT;
};

int CPU::pop_state()
{
    // All registers popped from stack, including SP, which simplifies things because we don't need to
    // roll SP back explicitly.
    uint32_t mem_base = read_reg(REG_SP);
    if (mem_base < MIN_STATE_POP) { // stack underflow
        return FAULT_STACK;
    }
    mem_base -= NUMREGS;
    for (int i = 0; i < NUMREGS; i++)
        write_reg(i, read_mem(mem_base + i));
    return FAULT_NO_FAULT;
};

int CPU::push_word(uint32_t word)
{
    uint32_t mem_base = read_reg(REG_SP);
    if (mem_base == MAX_ADDR) { // overflow
        return FAULT_STACK;
    }
    write_mem(mem_base, word);
    mem_base++;
    write_reg(REG_SP, mem_base);
    return FAULT_NO_FAULT;
}

int CPU::pop_word(uint32_t &word)
{
    uint32_t mem_base = read_reg(REG_SP);

    if (mem_base == 0) { // stack underflow
        return FAULT_STACK;
    }

    mem_base--;
    word = read_mem(mem_base);
    return FAULT_NO_FAULT;
};

// Execute the current opcode
uint32_t CPU::execute()
{
    uint32_t retval {FAULT_NO_FAULT};
    
    if (CurrentInst->get_opcode() == OP_INVALID) {
        return FAULT_BAD_INSTR;
    }
    // For ease of comprehension, this is all open-coded. It would be possible to
    // set up a bunch of classes and do some polymorphic magic and dynamic casts,
    // but that would get (IMO) too ugly and confusing very quickly.

    switch (CurrentInst->get_type()) {
        case op_no_args:
            retval = execute_no_args();
            break;
        case op_src_only:
            retval = execute_src_only();
            break;
        case op_src_dest:
            retval = execute_src_dest();
            break;
        case op_dest_only:
            retval = execute_dest_only();
            break;
        case op_control_flow:
            retval = execute_control_flow();
            break;
        case op_2src_dest:
            retval = execute_2src_dest();
            break;
        default:
            retval = FAULT_BAD_INSTR;
            break;
    }
    return retval;
};

uint32_t CPU::put_to_dest(uint32_t value)
{
    auto dest = CurrentInst->get_dest();
    assert(dest.get_type() != rt_unused);

    switch (dest.get_type())
    {
        case rt_indirect:
        {
            uint32_t addr = read_reg(dest.get_num());
            write_mem(addr, value);
            break;
        }
        case rt_value:
            write_reg(dest.get_num(), value);
            break;
        default:
            return FAULT_BAD_INSTR;
    }
    return FAULT_NO_FAULT;
};

uint32_t CPU::get_from_reg(RegisterArg srcreg, uint32_t &val)
{
    assert(srcreg.get_type() != rt_unused);

    switch (srcreg.get_type())
    {
        case rt_indirect:
        {
            uint32_t memaddr = read_reg(srcreg.get_num());
            val = read_mem(memaddr);
            break;
        }
        case rt_value:
            val = read_reg(srcreg.get_num());
            break;
        default:
            return FAULT_BAD_INSTR;
    }
    return FAULT_NO_FAULT;
};

uint32_t CPU::retrieve_direct_value()
{
    
    uint32_t addr = read_reg(REG_IP);
    uint32_t retval = read_mem(addr);
    incr_IP();
    return retval;
};

void CPU::halt()
{
    running = false;
    return;
}

bool CPU::is_halted()
{
    return (!running);
}

uint32_t CPU::execute_no_args()
{
    uint32_t faultval {FAULT_NO_FAULT};

    switch(CurrentInst->get_opcode()) {
        case OP_SSTATE:
            faultval = push_state();
            break;
        case OP_LSTATE:
        {
            // save off IP and R0 so we can restore them after we read the stack
            uint32_t tmpIP = read_reg(REG_IP);
            uint32_t tmpR0 = read_reg(REG_R0);
            faultval = pop_state();
            write_reg(REG_IP, tmpIP);
            write_reg(REG_R0, tmpR0);
            break;
        }
        case OP_RETURN:
        {
            uint32_t newIP;
            faultval = pop_word(newIP);
            write_reg(REG_IP, newIP);
            break;
        }
        case OP_IRET:
            faultval = pop_state(); // IP restored to previous position
            clear_flag(FLG_IN_INT);
            break;
        case OP_SIGNED:
            set_flag(FLG_SIGNED);
            break;
        case OP_UNSIGNED:
            clear_flag(FLG_SIGNED);
            break;
        case OP_INTENA:
            set_flag(FLG_INTENA);
            break;
        case OP_INTDIS:
            clear_flag(FLG_INTENA);
            break;
        case OP_NOP:
            // do nothing
            break;
        case OP_HALT:
            halt();
            break;
        default: // should never get here, but make the compiler happy
            faultval = FAULT_BAD_INSTR;
            break;
    }
    return faultval;
}

uint32_t CPU::execute_src_dest()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint8_t opcode = CurrentInst->get_opcode();
    
    if ((opcode != OP_MOVE) && CurrentInst->is_direct_val_instr())
        return FAULT_BAD_INSTR;

    if (opcode == OP_MOVE) {
        // First, the special case: MOV 0x000ff000, R0
        if (CurrentInst->is_direct_val_instr()) {
            faultval = put_to_dest(retrieve_direct_value());
        } else {
            uint32_t v;
            faultval = get_from_reg(CurrentInst->get_src1(), v);
            if (faultval == FAULT_NO_FAULT)
                faultval = put_to_dest(v);
        }
    } else if (opcode == OP_CMP) {
        uint32_t srcval, destval;
        clear_math_flags();
        // TODO good place for throw/catch, logic is ugly
        faultval = get_from_reg(CurrentInst->get_src1(), srcval);
        if (faultval == FAULT_NO_FAULT)
            faultval = get_from_reg(CurrentInst->get_dest(), destval);
        if (faultval == FAULT_NO_FAULT) {
            if (srcval == destval)
                set_flag(FLG_ZERO);
            else if (srcval < destval)
                set_flag(FLG_UNDER);
            else
                set_flag(FLG_OVER);
        }
    }

    return faultval;
}

uint32_t CPU::execute_src_only()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint32_t tmp;
    uint8_t opcode = CurrentInst->get_opcode();

    faultval = get_from_reg(CurrentInst->get_src1(), tmp);
    if (faultval == FAULT_NO_FAULT)
        switch(opcode) {
            case OP_PUSH:
                faultval = push_word(tmp);
                break;
            case OP_SETFHAP:
                if (tmp > MAX_FHAP) {
                    faultval = FAULT_BAD_INSTR;
                    break;
                }
                set_FHAP(tmp);
                break;
            case OP_SETIHAP:
                if (tmp > MAX_IHAP) {
                    faultval = FAULT_BAD_INSTR;
                    break;
                }
                set_IHAP(tmp);
                break;
            default:
                faultval = FAULT_BAD_INSTR;
                break;
        }
    return faultval;
}

uint32_t CPU::execute_dest_only()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint32_t tmp;
    uint8_t opcode = CurrentInst->get_opcode();

    if (opcode == OP_POP) {
        faultval = pop_word(tmp);
        if (faultval == FAULT_NO_FAULT)
            faultval = put_to_dest(tmp);
    } else {
        faultval = get_from_reg(CurrentInst->get_dest(), tmp);
        if (faultval == FAULT_NO_FAULT)
            switch (opcode) {
                case OP_NOT:
                    clear_math_flags();
                    tmp = ~tmp;
                    faultval = put_to_dest(tmp);
                    indicate_zero(tmp);
                    break;
                case OP_INCR:
                    clear_math_flags();
                    if (is_flag_set(FLG_SIGNED)) {
                        int32_t stmp = tmp;
                        stmp++; // will properly handle negative numbers;
                        if (stmp == INT_MIN) // overflow
                            set_flag(FLG_OVER);
                        tmp = stmp;
                    } else {
                        tmp++;
                        if (tmp == 0) // overflow
                            set_flag(FLG_OVER);
                    }
                    faultval = put_to_dest(tmp);
                    indicate_zero(tmp);
                    break;
                case OP_DECR:
                    clear_math_flags();
                    if (is_flag_set(FLG_SIGNED)) {
                        int32_t stmp = tmp;
                        stmp--; // will properly handle negative numbers;
                        if (stmp == INT_MAX) // underflow
                            set_flag(FLG_UNDER);
                        tmp = stmp;
                    } else {
                        tmp--;
                        if (tmp == 0xFFFFFFFF)
                            set_flag(FLG_UNDER);
                    }
                    faultval = put_to_dest(tmp);
                    indicate_zero(tmp);
                    break;
                default:
                    faultval = FAULT_BAD_INSTR;
                    break;
            }
    }
    return faultval;
}


uint32_t CPU::execute_control_flow()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint32_t tmp;
    uint8_t opcode = CurrentInst->get_opcode();

    if (CurrentInst->is_direct_val_instr())
        tmp = retrieve_direct_value(); // cannot fault
    else
        faultval = get_from_reg(CurrentInst->get_dest(), tmp);

    if (faultval == FAULT_NO_FAULT) {
        switch (opcode) {
            case OP_JZERO:
                if (is_flag_set(FLG_ZERO))
                    write_reg(REG_IP, tmp);
                break;
            case OP_JNZERO:
                if (!is_flag_set(FLG_ZERO))
                    write_reg(REG_IP, tmp);
                break;
            case OP_JOVER:
                if (is_flag_set(FLG_OVER))
                    write_reg(REG_IP, tmp);
                break;
            case OP_JNOVER:
                if (!is_flag_set(FLG_OVER))
                    write_reg(REG_IP, tmp);
                break;
            case OP_JUNDER:
                if (is_flag_set(FLG_UNDER))
                    write_reg(REG_IP, tmp);
                break;
            case OP_JNUNDER:
                if (!is_flag_set(FLG_UNDER))
                    write_reg(REG_IP, tmp);
                break;
            case OP_JMP:
                write_reg(REG_IP, tmp);
                break;
            case OP_CALL:
                // push current IP, then jump
                faultval = push_word(read_reg(REG_IP));
                if (faultval == FAULT_NO_FAULT)
                    write_reg(REG_IP, tmp);
                break;
            default:
                faultval = FAULT_BAD_INSTR;
                break;
        }
    }
    return faultval;
}

uint32_t CPU::execute_2src_dest()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint32_t src1val, src2val, destval;
    uint8_t opcode = CurrentInst->get_opcode();

    faultval = get_from_reg(CurrentInst->get_src1(), src1val);
    if (faultval == FAULT_NO_FAULT)
        faultval = get_from_reg(CurrentInst->get_src2(), src2val);
    if (faultval == FAULT_NO_FAULT) {
        clear_math_flags();
        switch (opcode) {
            case OP_ADD:
                if (is_flag_set(FLG_SIGNED)) {
                    int32_t s1 = src1val;
                    int32_t s2 = src2val;
                    int32_t d = s1 + s2;
                    if (d < s1 || d < s2)
                        set_flag(FLG_OVER);
                    destval = d;
                } else {
                    destval = src1val + src2val;
                    if (destval < src1val || destval < src2val)
                        set_flag(FLG_OVER);
                }
                break;
            case OP_SUB:
                if (is_flag_set(FLG_SIGNED)) {
                    int32_t s1 = src1val;
                    int32_t s2 = src2val;
                    int32_t d = s1 - s2;
                    if (d > s1 || d > s2)
                        set_flag(FLG_UNDER);
                    destval = d;
                } else {
                    destval = src1val - src2val;
                    if (destval > src1val || destval > src2val)
                        set_flag(FLG_UNDER);
                }
                break;
            case OP_AND:
                destval = src1val & src2val;
                break;
            case OP_OR:
                destval = src1val | src2val;
                break;
            case OP_XOR:
                destval = src1val ^ src2val;
                break;
            case OP_SHIFTR:
                destval = src1val >> src2val;
                break;
            case OP_SHIFTL:
                destval = src1val << src2val;
                break;
            default:
                faultval = FAULT_BAD_INSTR;
                break;
        }
    }
    indicate_zero(destval);
    faultval = put_to_dest(destval);
    return faultval;
}

