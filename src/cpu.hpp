// cpu.hpp - declarations for CPU class - the core of the Comp-o-Tron 6000
#ifndef __CPU_HPP__
#define __CPU_HPP__

#include <cstdint>
#include "memory.hpp"
#include "instruction.hpp"

// Passed to emulator for printing - allows a single function call to get info instead of 19.
struct CPUInternalState {
    uint32_t Registers[NUMREGS];
    bool Halted;
    uint32_t FHAP_Base; // coding style violation but FHAPBase looks bad
    uint32_t IHAP_Base;
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

private:
    Memory *Mem;
    uint32_t Reg[NUMREGS] {0};
    bool Running {true};
    uint32_t FHAP_Addr {0}; // Fault Handler Pointer
    uint32_t IHAP_Addr {0}; // Interrupt Handler Pointer
    Instruction *CurrentInst;

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
};

#endif // __CPU_HPP__
