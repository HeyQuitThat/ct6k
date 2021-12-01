// UI class - encapsulation of UI calls for ncurses ct6k emulator


#ifndef __UI_HPP__
#define __UI_HPP__
#include "arch.h"
#include <ncurses.h>
#include <cstdint>
#include <string>

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
    // Modal dialogs
    void ShowHelpWindow(); // called function gets keystroke and destroys window
    void ShowDisasmWindow(); // ditto
    bool ShowConfirmation();
    bool InputAssembly(uint32_t &Addr, std::string Input);  // Does not actually assemble here
    bool InputReg(uint32_t &RegNum, uint32_t &NewVal);
    bool InputMem(uint32_t &Addr, uint32_t &NewVal);


private:
    // data
    WINDOW *RegWin;
    WINDOW *FlagWin;
    WINDOW *StackWin;
    WINDOW *HAPsWin;
    WINDOW *HelpWin;
    WINDOW *DisasmWin;
    DisplayState CurrentState;
    // methods
    WINDOW *CreateWindow(int Height, int Width, int Y, int X);
    int RegCharTranslate(int Current);
    void HighlightFlagRow(int Row, bool On);
    void RedrawRegWindow();
    void RefreshAll(); // must be called after destroying any modal window

};

// defines specific to a text UI - mostly related to what goes where on the screen
#define SCREEN_Y_MIN 25
#define SCREEN_X_MIN 80
#define CP_DEFAULT 1
#define CP_GREEN 2
#define CP_RED 3
#define CP_BLUE 4

#define REG_WIN_Y 2
#define REG_WIN_X 2
#define REG_WIN_WIDTH 54
#define REG_WIN_HEIGHT 18
#define REG_NAME_X 49
#define REG_FLG_Y 14
#define REG_SP_Y 15
#define REG_IP_Y 16
#define REG_VAL_X 7
#define FLG_WIN_Y 2
#define FLG_WIN_X 60
#define FLG_WIN_WIDTH 14
#define FLG_WIN_HEIGHT 11
#define FLG_OVER_ROW 3
#define FLG_UNDER_ROW 4
#define FLG_ZERO_ROW 5
#define FLG_INTERRUPT_ROW 6
#define FLG_SIGNED_ROW 7
#define FLG_INT_ENA_ROW 8
#define FLG_FAULT_ROW 9
#define HAP_WIN_Y 14
#define HAP_WIN_X 60
#define HAP_WIN_WIDTH 14
#define HAP_WIN_HEIGHT 7
#define FHAP_ADDR_ROW 2
#define IHAP_ADDR_ROW 5
#define FHAP_ADDR_COL 2
#define IHAP_ADDR_COL 2
#define STATUS_ROW 24
#define RUN_STATE_COL 50
#define RUN_STATE_BLANK "                              "
#define INSTR_ROW 20
#endif // __UI_HPP__
