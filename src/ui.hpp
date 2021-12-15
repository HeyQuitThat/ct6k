// UI class - encapsulation of UI calls for ncurses ct6k emulator


#ifndef __UI_HPP__
#define __UI_HPP__
#include "arch.h"
#include <ncurses.h>
#include <cstdint>
#include <string>
#include <vector>

enum DisplayState {
    DS_Binary, // default, shows 0/1 for each bit
    DS_Blinky, // same as binary but red and green dots instead of 1/0
};

class UI {
public:
    ~UI();
    int InitGui();
    void StopGui();
    void DrawStaticElements();
    void ChangeDisplayState();
    void DrawReg(uint32_t RegNum, uint32_t Value);
    void DrawFlags(uint32_t FlagsReg);
    void DrawHAPs(uint32_t FHAPAddr, uint32_t IHAPAddr);  // No need for current/prev as this is updated
                                                            // so infrequently and it's only two dwords
    void DrawRunState(std::string State);
    void DrawNextInstr(std::string Instruction);
    void DrawMessage(std::string Message);
    // Modal dialogs
    void ShowHelpWindow(); // called function gets keystroke and destroys window
    void ShowDisasmWindow(); // ditto
    bool InputBreakpoint(uint32_t &BP);
    bool InputAssembly(uint32_t &Addr, std::string& Input);  // Does not actually assemble here
    bool InputReg(uint32_t &RegNum, uint32_t &NewVal);
    bool InputMem(uint32_t &Addr, std::vector<uint32_t> &Data);
    bool InputMemAddr(uint32_t &Addr);
    bool ConfirmExit();
    bool ConfirmReset();
    void ShowMemDump(uint32_t Addr, std::vector<uint32_t> Data);
    void ShowStackDump(uint32_t Addr, std::vector<uint32_t> Data); // Similar to mem dump but counts down


private:
    // data
    WINDOW *RegWin;
    WINDOW *FlagWin;
    WINDOW *HAPsWin;
    DisplayState CurrentState;
    // methods
    WINDOW *CreateWindow(int Height, int Width, int Y, int X);
    int RegCharTranslate(int Current);
    void HighlightFlagRow(int Row, bool On);
    bool HexInput(int Row, int Col, uint32_t &Input);
    void RedrawRegWindow();
    void ClearMessageLine();
    void RefreshAll(); // must be called after destroying any modal window
    bool ShowConfirmation(const char *Message);
    void ShowDumpWin(uint32_t Addr, std::vector<uint32_t> Data, bool CountUp);

};

#define CT6K_KEY_STEP 'S'
#define CT6K_KEY_SLOW 'W'
#define CT6K_KEY_QUICK 'Q'
#define CT6K_KEY_FULL 'F'
#define CT6K_KEY_VIEWMEM 'Y'
#define CT6K_KEY_VIEWSTACK 'K'
#define CT6K_KEY_VIEWCODE 'C'
#define CT6K_KEY_MODREG 'R'
#define CT6K_KEY_MODMEM 'M'
#define CT6K_KEY_MODBRK 'B'
#define CT6K_KEY_MODE 'T'
#define CT6K_KEY_EXIT KEY_END
#define CT6K_KEY_HELP KEY_F(1)
#define CT6K_KEY_RESET KEY_F(12)
#endif // __UI_HPP__
