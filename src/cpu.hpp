// cpu.hpp - declarations for CPU class
#ifndef __CPU_HPP__
#define __CPU_HPP__

#include <cstdint>
#include "memory.hpp"
#include "instruction.hpp"

struct cpu_internal_state {
    uint32_t registers[NUMREGS]; // includes flags
    bool halted;
    uint32_t FHAP_base;
    uint32_t IHAP_base;
};

class CPU {
private:
    Memory *mem;
    uint32_t execute(); // executes current instruction, returns fault value
    uint32_t reg[NUMREGS] {0};
    bool running {true};
    uint32_t FHAPAddr {0}; // Fault Handler Pointer
    uint32_t IHAPAddr {0}; // Interrupt Handler Pointer
    Instruction *CurrentInst;
    uint32_t retrieve_direct_value();
    uint32_t put_to_dest(uint32_t);
    uint32_t get_from_reg(RegisterArg, uint32_t &);
    uint32_t execute_no_args();
    uint32_t execute_src_dest();
    uint32_t execute_src_only();
    uint32_t execute_dest_only();
    uint32_t execute_control_flow();
    uint32_t execute_2src_dest();
    void indicate_zero(uint32_t);
    void incr_IP();

public:
    CPU();
    ~CPU();
    void step();
    cpu_internal_state dump_internal_state();
    uint32_t read_reg(uint8_t);
    void write_reg(uint8_t, uint32_t);
    uint32_t read_mem(uint32_t address);
    void write_mem(uint32_t address, uint32_t value);
    void set_flag(uint32_t);
    void clear_flag(uint32_t);
    void clear_math_flags();
    bool is_flag_set(uint32_t);
    void set_FHAP(uint32_t);
    void set_IHAP(uint32_t);
    void fault(uint32_t);
    int push_state();
    int pop_state();
    int push_word(uint32_t);
    int pop_word(uint32_t &);
    void halt();
    bool is_halted();

};

#endif // __CPU_HPP__
