#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include "arch.h"
#include "cpu.hpp"
#include <boost/format.hpp>

// An emulator for the Comp-o-Tron 6000 Computer. Actually, the code in this file is
// really only a basic test-only framework, with a better interface to be added later.
// Currently, this file creates a CPU object, loads the specified binary file into the
// CPU's memory, and then steps through, pausing for input (ignored) after each
// instruction. Pretty, it ain't. But it shows that the CPU and assembler software works
// as expected.


// Helper macro to display status of a flag
#define INDICATE_FLAG(_r, _f) {std::cout << " "; if ((_r) & (_f)) std::cout << "+"; else std::cout << "-"; std::cout << #_f;}

// Print the instruction based upon the value(s) given.
void PrintDisasm(uint32_t Val, uint32_t Val2)
{
    std::string outstr;
    auto i = new Instruction(Val, Val2);
    i->Print(outstr);
    std::cout << outstr;
    delete i;
}

// Print the entire CPU state, including flags and the next instruction.
void PrintCpuState(CPU *C)
{
    CPUInternalState state = C->DumpInternalState();

    for (int i = 0; i < NUMREGS; i++) {
        uint32_t tmp = state.Registers[i];

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
                    std::cout << (boost::format("0x%08X") % C->ReadMem(tmp - j)).str() << " ";
                std::cout << "]";
            }
        }
        if (i == REG_IP) {
            std::cout << " [IP]";
            std::cout << " [ ";
            for (int j = 0; j < 8; j++)
                std::cout << (boost::format("0x%08X") % C->ReadMem(tmp + j)).str() << " ";
            std::cout << "]";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << "FHAP addr = " << (boost::format("0x%08X") % state.FHAP_Base).str() << "\n";
    std::cout << "IHAP addr = " << (boost::format("0x%08X") % state.IHAP_Base).str() << "\n";
    std::cout << "Next instruction:\n";
    PrintDisasm(C->ReadMem(state.Registers[REG_IP]), C->ReadMem(state.Registers[REG_IP]+1));
    std::cout << "\nCPU is ";
    if (state.Halted)
        std::cout << "HALTED";
    else
        std::cout << "RUNNING";
    std::cout << "\n\n";
}

// Read and return a 32-bit word from an array of bytes, MSB first.
uint32_t FillWordFromMSB(uint8_t *Buf)
{
    uint32_t retval {0};
    for (auto i = 24, j = 0; i >= 0; i -= 8, j++) // ugly magic numbers
        retval |= (uint32_t)Buf[j] << i;
    return retval;
}

// Read a single word from the given file, MSB first.
// Returns the number of bytes read from the file, if nonzero and other than 4, it's a problem.
int ReadWord(std::ifstream& File, uint32_t& Word)
{
    uint8_t buffer[sizeof(uint32_t)];

    File.read((char *)buffer, sizeof(uint32_t));
    Word = FillWordFromMSB(buffer);
    // return bytes read
    return File.gcount();
}

// Display usage. Caller passes in argv[0] so that we can display the name of the command.
int Usage(char *cmd)
{
    std::cout << "USAGE:\n\t";
    std::cout << cmd << " binfile\n";
    std::cout << "No other options are currently supported\n\n";
    return 0;
}

// The main loop. Create a CPU, read a binary file into memory, and step through until it halts.
int main(int argc, char *argv[0])
{
    CPU *c = new CPU();
    std::ifstream infile;
    uint32_t loc {0};
    uint32_t word;
    int bytes {0};

    if (argc != 2)
        return Usage(argv[0]);

    std::cout << "Loading program...\n";
    try {
        infile.open(argv[1], std::ios::in | std::ios::binary);
    } catch (const char* msg) {
        std::cout << "Error opening input file: " << msg << "\n";
        return -1;
    }

    while ((bytes = ReadWord(infile, word)) == sizeof(uint32_t))
        c->WriteMem(loc++, word);

    if (!infile.eof() || bytes != 0) {
        std::cout << "Error reading file, continuing\n";
    }
    infile.close();
    
    PrintCpuState(c);
    while (!c->IsHalted()) {
        c->Step();
        PrintCpuState(c);
        std::cout << "Press enter to continue...\n";
        std::cin.ignore();
    }

    std::cout << "HALT - END OF RUN\n";
    delete(c);
    return 0;
}
