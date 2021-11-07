// instruction.hpp - declarations for Instruction class
#ifndef __INSTRUCTION_HPP__
#define __INSTRUCTION_HPP__

#include <cstdint>
#include <string>

enum _reg_type {
    rt_unused,
    rt_indirect,
    rt_value,
    rt_invalid,
    rt_null, // special case for MOV instruction
};

class RegisterArg {
public:
    RegisterArg(uint8_t);
    _reg_type get_type();
    uint8_t get_num();
    bool is_valid();
    void print(std::string &);

private:
    enum _reg_type type;
    uint8_t regnum;
};

enum _op_type {
    op_invalid = 0,
    op_no_args,
    op_src_only,
    op_dest_only,
    op_control_flow, // can use direct data
    op_src_dest, // allows direct data as well
    op_2src_dest,
};

#define MAX_INSTR_TEXT_LEN 9    // UNSIGNED + null. In real life, the compiler's likely to give us
                                // 16 bytes anyway.
struct opmap {
    const char name[MAX_INSTR_TEXT_LEN];
    const uint8_t opcode;
    const enum _op_type type;
};

class Instruction {
public:
    Instruction(uint32_t, uint32_t); // extra prefetch value for immdiate addressing,
    Instruction(uint32_t); // decodes upon init
    ~Instruction();
    void print(std::string&);
    uint32_t output_binary(bool, uint32_t&);
    uint8_t get_opcode();
    _op_type get_type();
    RegisterArg get_src1();
    RegisterArg get_src2();
    RegisterArg get_dest();
    bool is_direct_val_instr();
    bool is_direct_val_present();    // only used when assembling or debugging
    uint32_t get_direct_val();


private:
    // data
    uint32_t raw;
    bool is_raw {false};
    uint8_t opcode;
    struct opmap *map;
    RegisterArg *src1;
    RegisterArg *src2;
    RegisterArg *dest;
    uint32_t direct_val {0}; // only allowed for MOV
    bool direct_val_in_use {false};
    bool direct_val_provided {false};

    // methods
    bool is_valid_instruction();
    void print_opstr(std::string &outstr);

};

// Not part of the class, but can be used to build one
uint32_t build_instruction(std::string instring, uint32_t& extra_word, bool& extra_word_present);

#endif // __INSTRUCTION_HPP__
