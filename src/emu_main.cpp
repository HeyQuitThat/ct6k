#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include "arch.h"
#include "cpu.hpp"
#include <boost/format.hpp>

#define INDICATE_FLAG(_r, _f) {std::cout << " "; if ((_r) & (_f)) std::cout << "+"; else std::cout << "-"; std::cout << #_f;}

void print_next(uint32_t val, uint32_t val2)
{
    std::string outstr;
    auto i = new Instruction(val, val2);
    i->print(outstr);
    std::cout << outstr;
    delete i;
}
void print_cpu_state(CPU *c)
{
    cpu_internal_state state = c->dump_internal_state();

    for (int i = 0; i < NUMREGS; i++) {
        uint32_t tmp = state.registers[i];

        std::cout << "R" << i;
        if (i < 10)
            std::cout << " ";
        
        std::cout << " = " << (boost::format("0x%08X") % tmp).str();
        if (i == REG_FLG) {
            std::cout << " [FLAGS:";
            INDICATE_FLAG(tmp, FLG_OVER);
            INDICATE_FLAG(tmp, FLG_UNDER);
            INDICATE_FLAG(tmp, FLG_ZERO);
            INDICATE_FLAG(tmp, FLG_IN_INT);
            INDICATE_FLAG(tmp, FLG_SIGNED);
            INDICATE_FLAG(tmp, FLG_INTENA);
            INDICATE_FLAG(tmp, FLG_FAULT);
            std::cout << "]";
        }
        if (i == REG_SP) {
            std::cout << " [SP]";
            if (tmp == 0)
                std::cout << " [INVALID STACK]";
            else {
                std::cout << " [ ";
                for (int j = 0; j < 8; j++)
                    std::cout << (boost::format("0x%08X") % c->read_mem(tmp - j)).str() << " ";
                std::cout << "]";
            }
        }
        if (i == REG_IP) {
            std::cout << " [IP]";
            std::cout << " [ ";
            for (int j = 0; j < 8; j++)
                std::cout << (boost::format("0x%08X") % c->read_mem(tmp + j)).str() << " ";
            std::cout << "]";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << "FHAP addr = " << (boost::format("0x%08X") % state.FHAP_base).str() << "\n";
    std::cout << "IHAP addr = " << (boost::format("0x%08X") % state.IHAP_base).str() << "\n";
    std::cout << "Next instruction:\n";
    print_next(c->read_mem(state.registers[REG_IP]), c->read_mem(state.registers[REG_IP]+1));
    std::cout << "\nCPU is ";
    if (state.halted)
        std::cout << "HALTED";
    else
        std::cout << "RUNNING";
    std::cout << "\n\n";
}

uint32_t fill_word_msb_first(uint8_t *buf)
{
    uint32_t retval {0};
    for (auto i = 24, j = 0; i >= 0; i -= 8, j++) // ugly magic numbers
        retval |= (uint32_t)buf[j] << i;
    return retval;
}

int read_word(std::ifstream& file, uint32_t& word)
{
    uint8_t buffer[sizeof(uint32_t)];

    file.read((char *)buffer, sizeof(uint32_t));
    word = fill_word_msb_first(buffer);
    // return bytes read
    return file.gcount();
}

int usage(char *cmd)
{
    std::cout << "USAGE:\n\t";
    std::cout << cmd << " binfile\n";
    std::cout << "No other options are currently supported\n\n";
    return 0;
}

int main(int argc, char *argv[0])
{
    CPU *c = new CPU();
    std::ifstream InFile;
    uint32_t Loc {0};
    uint32_t Word;
    int bytes {0};

    if (argc != 2)
        return usage(argv[0]);

    std::cout << "Loading program...\n";
    try {
        InFile.open(argv[1], std::ios::in | std::ios::binary);
    } catch (const char* msg) {
        std::cout << "Error opening input file: " << msg << "\n";
        return -1;
    }

    while ((bytes = read_word(InFile, Word)) == sizeof(uint32_t)) {
        std::cout << "Read word " << (boost::format("0x%08X") % Word).str() << " for location " << Loc << "\n";
        c->write_mem(Loc++, Word);
    }
    if (!InFile.eof() || bytes != 0) {
        std::cout << "Error reading file, continuing\n";
    }
    InFile.close();
    
    print_cpu_state(c);
    while (!c->is_halted()) {
        c->step();
        print_cpu_state(c);
        std::cout << "Press enter to continue...\n";
        std::cin.ignore();
    }
    std::cout << "HALT - END OF RUN\n";
    delete(c);
    return 0;
}
