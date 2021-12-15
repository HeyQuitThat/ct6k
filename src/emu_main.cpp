// An emulator for the Comp-o-Tron 6000 Computer.
// First successful run in full-screen ncurses mode on November 30, 2021.

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>
#include "unistd.h"
#include <ncurses.h>
#include "arch.h"
#include "cpu.hpp"
#include <boost/format.hpp>
#include "ui.hpp"

#define SLOW_SLEEP 400000 // 400msec
#define QUICK_SLEEP 100000 // 100msec

// Print the instruction based upon the value(s) given.
std::string FormatDisasm(uint32_t Val, uint32_t Val2)
{
    std::string outstr;
    auto i = new Instruction(Val, Val2);
    i->Print(outstr);
    delete i;
    return outstr;
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
// Load a program from a binary file. No checking is done at this point - if you load
// garbage, it will attempt to execute garbage. 
void LoadProgram(char* File, CPU *C)
{
    std::ifstream infile;
    uint32_t loc {0};
    uint32_t word;
    int bytes {0};

    std::cout << "Loading program...\n";
    try {
        infile.open(File, std::ios::in | std::ios::binary);
    } catch (const char* msg) {
        std::cout << "Error opening input file: " << msg << ", continuing.\n";
        return;
    }

    while ((bytes = ReadWord(infile, word)) == sizeof(uint32_t))
        C->WriteMem(loc++, word);

    if (!infile.eof() || bytes != 0) {
        std::cout << "Error reading file, continuing\n";
    }
    infile.close();
    std::cout << "Loading complete, beginning run.\n\n";
}

// Display usage. Caller passes in argv[0] so that we can display the name of the command.
int Usage(char *cmd)
{
    std::cout << "USAGE:\n\t";
    std::cout << cmd << " [binfile]\n";
    std::cout << "No other options are currently supported\n\n";
    return 0;
}

enum RunState {
    RS_Step,
    RS_Slow,
    RS_Quick,
    RS_Full,
    RS_Halted,
};

// compare internal CPU states and only display what has changed
void UpdateScreen(CPUInternalState Current, CPUInternalState Previous, UI *Screen)
{
    for (int i = 0; i < NUMREGS; i++) {
        if (Current.Registers[i] != Previous.Registers[i])
            Screen->DrawReg(i, Current.Registers[i]);
    }
    if (Current.Registers[REG_FLG] != Previous.Registers[REG_FLG])
        Screen->DrawFlags(Current.Registers[REG_FLG]);

    if ((Current.FHAP_Base != Previous.FHAP_Base) ||
        (Current.IHAP_Base != Previous.IHAP_Base))
        Screen->DrawHAPs(Current.FHAP_Base, Current.IHAP_Base);
   
}

// The main loop. Create a CPU, read a binary file into memory, and step through until it halts.
int main(int argc, char *argv[0])
{
    CPU *ct6k = new CPU();
    UI *foil = new UI();  // [n]curses, foiled again!
    CPUInternalState curr_state, prev_state;
    RunState RS {RS_Step};
    int quitting {false};
    bool bp_active {false};
    uint32_t breakpoint {0};

    // Loading a program is optional, users can hand-assemble a bootstrap loader if they want.
    if (argc > 2)
        return Usage(argv[0]);

    if (argc == 2)
        LoadProgram(argv[1], ct6k);
    curr_state = ct6k->DumpInternalState(); // Just to prep

    foil->InitGui();
    foil->DrawStaticElements();
    foil->DrawRunState("STEPPING");

    while (!quitting) {
        int c;
        std::string tmp;

        prev_state = curr_state;
        curr_state = ct6k->DumpInternalState();
        UpdateScreen(curr_state, prev_state, foil);

        if (bp_active && (curr_state.Registers[REG_IP] == breakpoint))
        {
            nodelay(stdscr, false);
            foil->DrawMessage("    Reached breakpoint! Press any key.");
            RS = RS_Step;
            foil->DrawRunState("STEPPING");
        }

        if (RS != RS_Full) {
            tmp = FormatDisasm(ct6k->ReadMem(curr_state.Registers[REG_IP]), ct6k->ReadMem(curr_state.Registers[REG_IP]+1));
            foil->DrawNextInstr(tmp);
        }
        
        if (curr_state.Halted == true) {
            RS = RS_Halted;
            foil->DrawRunState("HALTED");
            nodelay(stdscr, false);
        }

        // TODO check for breakpoint
        switch (RS) {
            case RS_Slow:
                usleep(SLOW_SLEEP);
                // fall through - do both delays to get 2Hz time
            case RS_Quick:
                usleep(QUICK_SLEEP);
                // fall through
            case RS_Full:
                // we are in nodelay mode, just check for any key and keep rolling
                c = getch(); // ignore return value - any key stops run mode
                if (c == ERR) {
                    ct6k->Step();
                    continue; // just keep cranking through
                } else {
                    RS = RS_Step;
                    foil->DrawRunState("STEPPING");
                    nodelay(stdscr, false);
                    continue;
                }
                break;
            default:
                c = getch();
                break;
        }

        c = toupper(c);
        switch (c) {
            case 'S':
            case ' ':
                // Single-step - no effect if halted
                ct6k->Step();
                break;
            case 'F':
                // Full-speed run
                if (RS != RS_Halted) {
                    nodelay(stdscr, true);
                    RS = RS_Full;
                    foil->DrawRunState("RUNNING");
                    foil->DrawNextInstr("FULL-SPEED RUN");
                }
                break;
            case 'W':
                // Slow run - 2Hz
                if (RS != RS_Halted) {
                    nodelay(stdscr, true);
                    RS = RS_Slow;
                    foil->DrawRunState("RUNNING 2Hz");
                }
                break;
            case 'Q':
                // Quick run - 10Hz
                if (RS != RS_Halted) {
                    nodelay(stdscr, true);
                    RS = RS_Quick;
                    foil->DrawRunState("RUNNING 10Hz");
                }
                break;
            case 'B':
                bp_active = foil->InputBreakpoint(breakpoint);
                break;
            // set breakpoint
            case 'T':
                foil->ChangeDisplayState();
                break;
            case 'R':
            // modify register
            case 'M': {
            // change memory
                std::vector<uint32_t> values;
                uint32_t addr;
                if (foil->InputMem(addr, values))
                    for (auto i : values)
                        ct6k->WriteMem(addr++, i);
                break;
            }
            case 'C':
            // show disassembly (code)
                break;
            case 'Y': {
                // view memory
                std::vector<uint32_t> values;
                uint32_t addr;

                if (foil->InputMemAddr(addr)) {
                    for (int i = 0; i < 16; i++)
                        values.push_back(ct6k->ReadMem(addr + i));
                    foil->ShowMemDump(addr, values);
                }
                break;
            }
            case 'K': {
                // view stack
                std::vector<uint32_t> values;
                uint32_t addr = curr_state.Registers[REG_SP];
                for (int i = 0; i < 16; i++)
                    values.push_back(ct6k->ReadMem(addr - i));
                foil->ShowStackDump(addr, values);
                break;
            }
            case KEY_F(12):
            // reset
                if (foil->ConfirmReset()) {
                    ct6k->Reset();
                    bp_active = false;
                    RS = RS_Step;
                    foil->DrawRunState("STEPPING");
                    nodelay(stdscr, false);
                }
                break;
            case KEY_END:
            // exit
                if (foil->ConfirmExit())
                    quitting = true;
                break;
            case KEY_F(1):
                foil->ShowHelpWindow();
                break;
            default:
            // break from run, already handled
                break;
        }
    }
    
    delete(foil); // will call endwin();
    delete(ct6k);
    return 0;
}
