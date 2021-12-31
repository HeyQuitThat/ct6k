// cpu.cpp - definitions for CPU class of Comp-o-Tron 6000
// First successful code execution: October 24, 2021
// First loop executed November 4, 2021

#include <cstdint>
#include <new>
#include <cassert>
#include <climits>
#include "arch.h"
#include "cpu.hpp"

// Constructor, takes no arguments. If needed, we could take one to set the memory size.
CPU::CPU()
{
    Mem = new Memory(); /* using DEFAULT_SIZE */
};

// Destructor
CPU::~CPU()
{
    delete Mem;
};

// Returns value of register at given index. This is public to allow inspection.
uint32_t CPU::ReadReg(uint8_t Index)
{
    assert (Index < NUMREGS);
    return Reg[Index];
};

void CPU::Reset()
{
    delete Mem;
    Mem = new Memory();
    for (int i = 0; i < 16; i++)
        Reg[i] = 0;
    Running = true;
    FHAP_Addr = 0;
    IHAP_Addr = 0;
}
// Write register with given value at given index.
void CPU::WriteReg(uint8_t Index, uint32_t Value)
{
    assert (Index < NUMREGS);
    Reg[Index] = Value;
}

// Convenience function to increment IP - used during execution.
void CPU::IncrIP()
{
    Reg[REG_IP]++;
};

// Getter function for complete internal state in one call.
CPUInternalState CPU::DumpInternalState()
{
    CPUInternalState retval;
    for (int i = 0; i < NUMREGS; i++)
        retval.Registers[i] = Reg[i];
    retval.Halted = !Running;
    retval.FHAP_Base = FHAP_Addr;
    retval.IHAP_Base = IHAP_Addr;
    return retval;
}

// Passthrough to memory read function
uint32_t CPU::ReadMem(uint32_t Address)
{
    return Mem->MemRead(Address);
};

// Passthrough to memory write function. Intended for debug use.
void CPU::WriteMem(uint32_t Address, uint32_t Value)
{
    Mem->MemWrite(Address, Value);
};

// Utility function to directly set a flag in the flag register. Does not check the state of the flag
// first, and does not modify other flags.
void CPU::SetFlag(uint32_t Flag)
{
    uint32_t tmp = ReadReg(REG_FLG);
    tmp |= Flag;
    WriteReg(REG_FLG, tmp);
};

// Utility function to clear a flag in the flag register. Does not check the state of the flag
// first, and does not modify other flags.
void CPU::ClearFlag(uint32_t Flag)
{
    uint32_t tmp = ReadReg(REG_FLG);
    tmp &= ~Flag;
    WriteReg(REG_FLG, tmp);
};

// Utility function to clear all math-related flags before any math function is performed. Affects overflow,
// underflow, and zero flags.
void CPU::ClearMathFlags()
{
    uint32_t tmp = ReadReg(REG_FLG);
    tmp &= ~(FLG_OVER | FLG_UNDER | FLG_ZERO);
    WriteReg(REG_FLG, tmp);
};

// Set the state of the zero flag based on the given word.
void CPU::IndicateZero(uint32_t Val)
{
        if (Val == 0)
            SetFlag(FLG_ZERO);
        else
            ClearFlag(FLG_ZERO);
}

// Convenience function to check the state of the given flag.
bool CPU::IsFlagSet(uint32_t Flag)
{
    uint32_t tmp = ReadReg(REG_FLG);
    return !!(tmp & Flag);
};


// Private function to set the the value of the Fault Handler Address Pointer.
void CPU::Set_FHAP(uint32_t Addr)
{
    FHAP_Addr = Addr;
};

// Private function to set the the value of the Interrupt Handler Address Pointer.
void CPU::Set_IHAP(uint32_t Addr)
{
    IHAP_Addr = Addr;
};

// This is where things actually happen! Simulates a single clock cycle of the processor.
void CPU::Step()
{
    if (!Running)
        // we are halted; don't do anything
        return;
    // TODO check for and deal with interrupts here (better have some I/O devices first!)
    uint32_t iaddr = ReadReg(REG_IP);
    IncrIP();
    CurrentInst = new Instruction(ReadMem(iaddr));
    uint32_t ftype = Execute();
    delete CurrentInst;
    if (ftype)
        Fault(ftype);
};

// Fault processing. When a fault is found, save the CPU state and jump to the registered fault
// handler in the FHAP. Note that there is no error checking, so if the FHAP isn't set up, the CPU
// will immediately double-fault on the next clock.
void CPU::Fault(uint32_t Type)
{
    uint32_t newIP;

    if (IsFlagSet(FLG_FAULT)) {
        // already in a fault, this is a double-fault
        Halt();
        return;
    }
    SetFlag(FLG_FAULT);
    PushState();
    newIP = ReadMem(FHAP_Addr + Type);
    WriteReg(REG_IP, newIP);
};

// Save the CPU state to the stack, in preparation for calling a fault handler, an interrupt handler, or a subroutine.
// Fault handlers can look at the stack to get the IP of the original fault location.
// Returns fault status.
int CPU::PushState()
{
    // All registers pushed to stack, including SP, then SP updated. Fault if we hit top of memory.
    // Bad Things will happen if SP + space for 16 words are not located in populated memory, but not a fault.
    // No flags are updated here.
    uint32_t mem_base = ReadReg(REG_SP);

    if (mem_base > MAX_STATE_PUSH) { // no space for state
        return FAULT_STACK;
    }
    for (int i = 0; i < NUMREGS; i++)
        WriteMem(mem_base + i, ReadReg(i));
    WriteReg(REG_SP, mem_base + NUMREGS);
    return FAULT_NO_FAULT;
};

// Restore machine state after call or interrupt/fault handler.
// Returns fault status.
int CPU::PopState()
{
    // All registers popped from stack, including SP, which simplifies things because we don't need to
    // roll SP back explicitly.
    uint32_t mem_base = ReadReg(REG_SP);
    if (mem_base < MIN_STATE_POP) { // stack underflow
        return FAULT_STACK;
    }
    mem_base -= NUMREGS;
    for (int i = 0; i < NUMREGS; i++)
        WriteReg(i, ReadMem(mem_base + i));
    return FAULT_NO_FAULT;
};

// Push the given word to the stack, adjusting SP in the process.
// Returns fault status.
int CPU::PushWord(uint32_t Word)
{
    uint32_t mem_base = ReadReg(REG_SP);
    if (mem_base == MAX_ADDR) { // overflow
        return FAULT_STACK;
    }
    WriteMem(mem_base, Word);
    mem_base++;
    WriteReg(REG_SP, mem_base);
    return FAULT_NO_FAULT;
}

// Pop the next word off the stack, adjusting SP in the process.
// Returns fault status.
int CPU::PopWord(uint32_t &Word)
{
    uint32_t mem_base = ReadReg(REG_SP);

    if (mem_base == 0) { // stack underflow
        return FAULT_STACK;
    }

    mem_base--;
    Word = ReadMem(mem_base);
    WriteReg(REG_SP, mem_base);
    return FAULT_NO_FAULT;
};

// Execute the current opcode. Basically just a switch to call the appropriate function for the given opcode.
// Returns fault status.
uint32_t CPU::Execute()
{
    uint32_t retval {FAULT_NO_FAULT};

    if (CurrentInst->GetOpcode() == OP_INVALID) {
        return FAULT_BAD_INSTR;
    }
    // For ease of comprehension, this is all open-coded. It would be possible to
    // set up a bunch of classes and do some polymorphic magic and dynamic casts,
    // but that would get ugly and confusing very quickly.

    switch (CurrentInst->GetType()) {
        case op_no_args:
            retval = ExecuteNoArgs();
            break;
        case op_src_only:
            retval = ExecuteSrcOnly();
            break;
        case op_src_dest:
            retval = ExecuteSrcDest();
            break;
        case op_dest_only:
            retval = ExecuteDestOnly();
            break;
        case op_control_flow:
            retval = ExecuteControlFlow();
            break;
        case op_2src_dest:
            retval = Execute2SrcDest();
            break;
        default:
            retval = FAULT_BAD_INSTR;
            break;
    }
    return retval;
};

// Update the destination register of the instruction with the given word. If the destination is marked as direct,
// then just update the specified register. If it's indirect, then update the memory word pointed to by the register.
// Returns fault status.
uint32_t CPU::PutToDest(uint32_t Value)
{
    auto dest = CurrentInst->GetDestReg();

    switch (dest.GetType())
    {
        case rt_indirect:
        {
            uint32_t addr = ReadReg(dest.GetNum());
            WriteMem(addr, Value);
            break;
        }
        case rt_value:
            WriteReg(dest.GetNum(), Value);
            break;
        default:
            return FAULT_BAD_INSTR;
    }
    return FAULT_NO_FAULT;
};

// Load the value specified by the given register. Since instructions can have one or two source registers, the actual
// register argument is passed here. If the register argument is marked as direct, then just read the specified
// register. If it's indirect, then read the memory word pointed to by the register.
// Returns fault status.
uint32_t CPU::GetFromReg(RegisterArg SrcReg, uint32_t &Value)
{

    switch (SrcReg.GetType())
    {
        case rt_indirect:
        {
            uint32_t memaddr = ReadReg(SrcReg.GetNum());
            Value = ReadMem(memaddr);
            break;
        }
        case rt_value:
            Value = ReadReg(SrcReg.GetNum());
            break;
        default:
            return FAULT_BAD_INSTR;
    }
    return FAULT_NO_FAULT;
};

// If we've determined that a direct value is needed, read it from memory and update IP so we skip it when
// we go to execute the next instruction.
// Returns fault status.
uint32_t CPU::RetrieveDirectValue()
{

    uint32_t addr = ReadReg(REG_IP);
    uint32_t retval = ReadMem(addr);
    IncrIP();
    return retval;
};

// Stop the CPU, forever.
void CPU::Halt()
{
    Running = false;
}

// Getter for halt state.
bool CPU::IsHalted()
{
    return (!Running);
}

// Subfunction to execute instructions with no arguments.
// Returns fault status. May change registers and memory.
uint32_t CPU::ExecuteNoArgs()
{
    uint32_t faultval {FAULT_NO_FAULT};

    switch(CurrentInst->GetOpcode()) {
        case OP_SSTATE:
            faultval = PushState();
            break;
        case OP_LSTATE:
        {
            // save off IP and R0 so we can restore them after we read the stack
            uint32_t tmpIP = ReadReg(REG_IP);
            uint32_t tmpR0 = ReadReg(REG_R0);
            faultval = PopState();
            WriteReg(REG_IP, tmpIP);
            WriteReg(REG_R0, tmpR0);
            break;
        }
        case OP_RETURN:
        {
            uint32_t newIP {0};
            faultval = PopWord(newIP);
            WriteReg(REG_IP, newIP);
            break;
        }
        case OP_IRET:
            faultval = PopState(); // IP restored to previous position
            ClearFlag(FLG_IN_INT);
            break;
        case OP_SIGNED:
            SetFlag(FLG_SIGNED);
            break;
        case OP_UNSIGNED:
            ClearFlag(FLG_SIGNED);
            break;
        case OP_INTENA:
            SetFlag(FLG_INTENA);
            break;
        case OP_INTDIS:
            ClearFlag(FLG_INTENA);
            break;
        case OP_NOP:
            // do nothing
            break;
        case OP_HALT:
            Halt();
            break;
        default: // should never get here, but make the compiler happy
            faultval = FAULT_BAD_INSTR;
            break;
    }
    return faultval;
}

// Subfunction to execute instructions with a single source register and destination register.
// Returns fault status. May change registers and memory.
uint32_t CPU::ExecuteSrcDest()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint8_t opcode = CurrentInst->GetOpcode();

    if ((opcode != OP_MOVE) && CurrentInst->IsDirectValInstr())
        return FAULT_BAD_INSTR;

    // There are only two instructions with this pattern, so no need to bother with a switch.
    if (opcode == OP_MOVE) {
        // First, the special case: MOV 0x000ff000, R0
        if (CurrentInst->IsDirectValInstr()) {
            faultval = PutToDest(RetrieveDirectValue());
        } else {
            uint32_t v;

            faultval = GetFromReg(CurrentInst->GetSrc1Reg(), v);
            if (faultval == FAULT_NO_FAULT)
                faultval = PutToDest(v);
        }
    } else if (opcode == OP_CMP) {
        uint32_t srcval, destval;

        ClearMathFlags();
        faultval = GetFromReg(CurrentInst->GetSrc1Reg(), srcval);
        if (faultval == FAULT_NO_FAULT)
            faultval = GetFromReg(CurrentInst->GetDestReg(), destval);
        if (faultval == FAULT_NO_FAULT) {
            if (srcval == destval)
                SetFlag(FLG_ZERO);
            else if (srcval < destval)
                SetFlag(FLG_UNDER);
            else
                SetFlag(FLG_OVER);
        }
    }

    return faultval;
}

// Subfunction to execute instructions with a single source register only.
// Returns fault status. May change registers and memory.
uint32_t CPU::ExecuteSrcOnly()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint32_t tmp;
    uint8_t opcode = CurrentInst->GetOpcode();

    faultval = GetFromReg(CurrentInst->GetSrc1Reg(), tmp);
    if (faultval == FAULT_NO_FAULT)
        switch(opcode) {
            case OP_PUSH:
                faultval = PushWord(tmp);
                break;
            case OP_SETFHAP:
                if (tmp > MAX_FHAP) {
                    faultval = FAULT_BAD_ADDR;
                    break;
                }
                Set_FHAP(tmp);
                break;
            case OP_SETIHAP:
                if (tmp > MAX_IHAP) {
                    faultval = FAULT_BAD_ADDR;
                    break;
                }
                Set_IHAP(tmp);
                break;
            default:
                faultval = FAULT_BAD_INSTR;
                break;
        }
    return faultval;
}

// Subfunction to execute instructions with destination register only.
// Returns fault status. May change registers and memory.
uint32_t CPU::ExecuteDestOnly()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint32_t tmp;
    uint8_t opcode = CurrentInst->GetOpcode();

    if (opcode == OP_POP) {
        faultval = PopWord(tmp);
        if (faultval == FAULT_NO_FAULT)
            faultval = PutToDest(tmp);
    } else {
        faultval = GetFromReg(CurrentInst->GetDestReg(), tmp);
        if (faultval == FAULT_NO_FAULT)
            switch (opcode) {
                case OP_NOT:
                    ClearMathFlags();
                    tmp = ~tmp;
                    faultval = PutToDest(tmp);
                    IndicateZero(tmp);
                    break;
                case OP_INCR:
                    ClearMathFlags();
                    if (IsFlagSet(FLG_SIGNED)) {
                        int32_t stmp = tmp;
                        stmp++;
                        if (stmp == INT_MIN) // overflow
                            SetFlag(FLG_OVER);
                        tmp = stmp;
                    } else { // unsigned math
                        tmp++;
                        if (tmp == 0) // overflow
                            SetFlag(FLG_OVER);
                    }
                    faultval = PutToDest(tmp);
                    IndicateZero(tmp);
                    break;
                case OP_DECR:
                    ClearMathFlags();
                    if (IsFlagSet(FLG_SIGNED)) {
                        int32_t stmp = tmp;
                        stmp--;
                        if (stmp == INT_MAX) // underflow
                            SetFlag(FLG_UNDER);
                        tmp = stmp;
                    } else { // unsigned math
                        tmp--;
                        if (tmp == 0xFFFFFFFF)
                            SetFlag(FLG_UNDER);
                    }
                    faultval = PutToDest(tmp);
                    IndicateZero(tmp);
                    break;
                default:
                    faultval = FAULT_BAD_INSTR;
                    break;
            }
    }
    return faultval;
}


// Subfunction to execute control flow instructions (with destination register only).
// Returns fault status. May change registers and memory.
uint32_t CPU::ExecuteControlFlow()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint32_t tmp;
    uint8_t opcode = CurrentInst->GetOpcode();

    if (CurrentInst->IsDirectValInstr())
        tmp = RetrieveDirectValue(); // cannot fault
    else
        faultval = GetFromReg(CurrentInst->GetDestReg(), tmp);

    if (faultval == FAULT_NO_FAULT) {
        switch (opcode) {
            case OP_JZERO:
                if (IsFlagSet(FLG_ZERO) == true)
                    WriteReg(REG_IP, tmp);
                break;
            case OP_JNZERO:
                if (IsFlagSet(FLG_ZERO) == false)
                    WriteReg(REG_IP, tmp);
                break;
            case OP_JOVER:
                if (IsFlagSet(FLG_OVER) == true)
                    WriteReg(REG_IP, tmp);
                break;
            case OP_JNOVER:
                if (IsFlagSet(FLG_OVER) == false)
                    WriteReg(REG_IP, tmp);
                break;
            case OP_JUNDER:
                if (IsFlagSet(FLG_UNDER) == true)
                    WriteReg(REG_IP, tmp);
                break;
            case OP_JNUNDER:
                if (IsFlagSet(FLG_UNDER) == false)
                    WriteReg(REG_IP, tmp);
                break;
            case OP_JMP:
                WriteReg(REG_IP, tmp);
                break;
            case OP_CALL:
                // push current IP, then jump
                faultval = PushWord(ReadReg(REG_IP));
                if (faultval == FAULT_NO_FAULT)
                    WriteReg(REG_IP, tmp);
                break;
            default:
                faultval = FAULT_BAD_INSTR;
                break;
        }
    }
    return faultval;
}

// Subfunction to execute instructions with two source and one destination register specified.
// Returns fault status. May change registers and memory.
uint32_t CPU::Execute2SrcDest()
{
    uint32_t faultval {FAULT_NO_FAULT};
    uint32_t src1val, src2val;
    uint32_t destval {0};
    uint8_t opcode = CurrentInst->GetOpcode();

    faultval = GetFromReg(CurrentInst->GetSrc1Reg(), src1val);
    if (faultval == FAULT_NO_FAULT)
        faultval = GetFromReg(CurrentInst->GetSrc2Reg(), src2val);
    if (faultval == FAULT_NO_FAULT) {
        ClearMathFlags();
        switch (opcode) {
            case OP_ADD:
                if (IsFlagSet(FLG_SIGNED)) {
                    int32_t s1 = src1val;
                    int32_t s2 = src2val;
                    int32_t d = s1 + s2;
                    if (d < s1 || d < s2)
                        SetFlag(FLG_OVER);
                    destval = d;
                } else { // unsigned
                    destval = src1val + src2val;
                    if (destval < src1val || destval < src2val)
                        SetFlag(FLG_OVER);
                }
                break;
            case OP_SUB:
                if (IsFlagSet(FLG_SIGNED)) {
                    int32_t s1 = src1val;
                    int32_t s2 = src2val;
                    int32_t d = s1 - s2;
                    if (d > s1 || d > s2)
                        SetFlag(FLG_UNDER);
                    destval = d;
                } else { // unsigned
                    destval = src1val - src2val;
                    if (destval > src1val || destval > src2val)
                        SetFlag(FLG_UNDER);
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
    IndicateZero(destval);
    faultval = PutToDest(destval);
    return faultval;
}

