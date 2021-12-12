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
    DS_Hex, // Hex + Comp-o-Code (which looks just like ASCII), room for stack window in text mode
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
    void DrawStack(uint32_t *Stack);                      // 8 words, begins at SP - 7, only in hex mode
    void DrawRunState(std::string State);
    void DrawNextInstr(std::string Instruction);
    void DrawMessage(std::string Message);
    // Modal dialogs
    void ShowHelpWindow(); // called function gets keystroke and destroys window
    void ShowDisasmWindow(); // ditto
    bool ShowConfirmation();
    bool InputBreakpoint(uint32_t &BP);
    bool InputAssembly(uint32_t &Addr, std::string Input);  // Does not actually assemble here
    bool InputReg(uint32_t &RegNum, uint32_t &NewVal);
    bool InputMem(uint32_t &Addr, std::vector<uint32_t> &Data);
    bool InputMemAddr(uint32_t &Addr);
    void ShowMemDump(uint32_t Addr, std::vector<uint32_t> Data); // also for display of memory


private:
    // data
    WINDOW *RegWin;
    WINDOW *FlagWin;
    WINDOW *StackWin;
    WINDOW *HAPsWin;
    WINDOW *DisasmWin;
    DisplayState CurrentState;
    // methods
    WINDOW *CreateWindow(int Height, int Width, int Y, int X);
    int RegCharTranslate(int Current);
    void HighlightFlagRow(int Row, bool On);
    bool HexInput(int Row, int Col, uint32_t &Input);
    void RedrawRegWindow();
    void ClearMessageLine();
    void RefreshAll(); // must be called after destroying any modal window

};

#endif // __UI_HPP__
