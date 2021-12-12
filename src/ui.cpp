// ui.cpp - methods for ui class - ncurses interface for ct6k emulator
#include "ui.hpp"
#include "ui-priv.hpp"
#include <boost/format.hpp>


// Initialize the screen. This can fail if the screen/terminal is too small or doesn't support color.
// Since a constructor can never fail, we would have to throw an exception, which seems really rude.
// Instead, we'll just have separate call that returns status.
int UI::InitGui()
{

    int ScrX, ScrY;
    
    // Initialize the screen
    initscr();

    // Check screen size, fail if it's too small
    getmaxyx(stdscr, ScrY, ScrX);
    if ((ScrY < SCREEN_Y_MIN) || (ScrX< SCREEN_X_MIN)) {
        endwin();
        printf("Please run the Comp-o-Tron 6000 on a terminal with at least 80 columns and 25 rows\n");
        return -1;
    }
    if (has_colors() == false) {
        endwin();
        printf("Please run the Comp-o-Tron 6000 on a color-capable terminal.\n");
        return -1;
    }
    start_color();
    init_pair(CP_DEFAULT, COLOR_CYAN, COLOR_BLACK);
    init_pair(CP_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(CP_RED, COLOR_RED, COLOR_BLACK);
    init_pair(CP_BLUE, COLOR_BLUE, COLOR_BLACK);
  
    // Don't echo typed characters to the screen
    noecho();
    // Don't use line buffered input, respond to keypresses as they come in
    raw();
    // Enable keypad so we can do F1 for help
    keypad(stdscr, true);
    // turn off cursor
    curs_set(0);
    refresh();

    // create register window
    RegWin = CreateWindow(REG_WIN_HEIGHT, REG_WIN_WIDTH, REG_WIN_Y, REG_WIN_X);
    FlagWin = CreateWindow(FLG_WIN_HEIGHT, FLG_WIN_WIDTH, FLG_WIN_Y, FLG_WIN_X);
    HAPsWin = CreateWindow(HAP_WIN_HEIGHT, HAP_WIN_WIDTH, HAP_WIN_Y, HAP_WIN_X);

    CurrentState = DS_Blinky;
    return 0;
}

// destructor for UI - frees memory and resets the terminal to a usable state.
UI::~UI()
{
    // Dear ncurses, do I need to destroy windows here? Will I leak memory if I don't?
    endwin();
}


// Draw things on the screen that never (or rarely) change. This includes boxes, menu text,
// flag names, and the IHAP and FHAP address as zeros.
void UI::DrawStaticElements()
{
    // main screen elements
    attron(COLOR_PAIR(CP_DEFAULT));
    mvprintw(1, 32, "COMP-O-TRON 6000");
    mvprintw(22, 3, "Execute: (S)tep slo(W) (Q)uick (F)ull  / (V)iew Memory / (D)isassemble");
    mvprintw(23, 3, "Modify: (R)egister (M)emory (A)ssemble (B)reakpoint");
    mvprintw(24, 3, "F1 Help  END Exit  F12 Reset");
    mvprintw(24, 35, "(T)oggle Mode");
    attroff(COLOR_PAIR(CP_DEFAULT));

    // register window
    wattron(RegWin, COLOR_PAIR(CP_BLUE));
    for (int i = 0; i < 16; i++)
        mvwprintw(RegWin, i + 1, 2, "R%2.2d:", i);

    mvwprintw(RegWin, REG_FLG_Y, REG_NAME_X, "FLG");
    mvwprintw(RegWin, REG_SP_Y, REG_NAME_X, "SP");
    mvwprintw(RegWin, REG_IP_Y, REG_NAME_X, "IP");
    wattroff(RegWin, COLOR_PAIR(CP_BLUE));
    for (int i = 0; i < 16; i++) {
        mvwprintw(RegWin, i + 1, 2, "R%2.2d:", i);
        DrawReg(i, 0);
    }
    wrefresh(RegWin);

    // Flags window
    mvwaddch(FlagWin, 2, 0, ACS_LTEE);
    mvwaddch(FlagWin, 2, 13, ACS_RTEE);
    mvwhline(FlagWin, 2, 1, ACS_HLINE, 12);
    wattroff(FlagWin, COLOR_PAIR(CP_BLUE));
    mvwprintw(FlagWin, 1, 4, "FLAGS");
    wattron(FlagWin, COLOR_PAIR(CP_RED));
    mvwprintw(FlagWin, FLG_OVER_ROW, 5, "OVER");
    mvwprintw(FlagWin, FLG_UNDER_ROW, 4, "UNDER");
    mvwprintw(FlagWin, FLG_ZERO_ROW, 5, "ZERO");
    mvwprintw(FlagWin, FLG_INTERRUPT_ROW, 2, "INTERRUPT");
    mvwprintw(FlagWin, FLG_SIGNED_ROW, 4, "SIGNED");
    mvwprintw(FlagWin, FLG_INT_ENA_ROW, 3, "INT_ENA");
    mvwprintw(FlagWin, FLG_FAULT_ROW, 4, "FAULT");
    wattroff(FlagWin, COLOR_PAIR(CP_RED));
    wrefresh(FlagWin);

    // IHAP/FHAP window
    mvwaddch(HAPsWin, 3, 0, ACS_LTEE);
    mvwaddch(HAPsWin, 3, 13, ACS_RTEE);
    mvwhline(HAPsWin, 3, 1, ACS_HLINE, 12);
    mvwprintw(HAPsWin, 1, 5, "FHAP");
    mvwprintw(HAPsWin, 4, 5, "IHAP");
    wattroff(HAPsWin, COLOR_PAIR(CP_BLUE));
    mvwprintw(HAPsWin, FHAP_ADDR_ROW, FHAP_ADDR_COL, "0x00000000");
    mvwprintw(HAPsWin, IHAP_ADDR_ROW, IHAP_ADDR_COL, "0x00000000");
    wrefresh(HAPsWin);
    refresh();
}

// Change screen view from blinky lights to 1/0 to hex and back.
// Currently only binary displays are supported
void UI::ChangeDisplayState()
{
    if (CurrentState == DS_Binary)
        CurrentState = DS_Blinky;
    else
        CurrentState = DS_Binary;
    // For a switch from binary to blinky, we only need to redraw the register values
    RedrawRegWindow();
    // TODO hex mode 
    // For a switch to or from hex, we need to redraw windows
        
}

// Utility function to create an ncurses window
WINDOW *UI::CreateWindow(int Height, int Width, int Y, int X)
{
    WINDOW *retval;
    retval = newwin(Height, Width, Y, X);
    wattron(retval, COLOR_PAIR(CP_BLUE));
    wborder(retval, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
            ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
    return retval;
}

#define ZERO_BIN ('0' | A_NORMAL | COLOR_PAIR(CP_GREEN))
#define ZERO_BLINK ('*' | A_BOLD | COLOR_PAIR(CP_RED))
#define ONE_BIN ('1' | A_BOLD | COLOR_PAIR(CP_GREEN))
#define ONE_BLINK ('*' | A_BOLD | COLOR_PAIR(CP_GREEN))

// Draw a single register. Currently only supports binary and blinkenlight mode.
void UI::DrawReg(uint32_t RegNum, uint32_t Value)
{
    std::string Tmp;
    int ZeroChar;
    int OneChar;
    
    if (CurrentState == DS_Blinky) {
        ZeroChar = ZERO_BLINK;
        OneChar = ONE_BLINK;
    } else {
        ZeroChar = ZERO_BIN;
        OneChar = ONE_BIN;
    }

    wmove(RegWin, RegNum + 1, REG_VAL_X);
    for (int i = 31; i >= 0; i--) {
        if (((Value >> i) & 1) == 1)
            waddch(RegWin, OneChar);
        else
            waddch(RegWin, ZeroChar);
        // add spaces to group nybbles
        if (i % 4 == 0)
            waddch(RegWin, ' ');
    }
    wrefresh(RegWin);
}

// Utility function to change the state of the flags
void UI::HighlightFlagRow(int Row, bool On)
{
    if (On)
        mvwchgat(FlagWin, Row, 1, FLG_WIN_WIDTH -2, A_BOLD, CP_GREEN, nullptr);
    else
        mvwchgat(FlagWin, Row, 1, FLG_WIN_WIDTH -2, A_NORMAL, CP_RED, nullptr);
}

// Update all flags on screen
void UI::DrawFlags(uint32_t FlagsReg)
{
    // The flag text is already drawn on the screen, so we just need to change 
    // the attributes to make them bold or normal.
    HighlightFlagRow(FLG_OVER_ROW, (FlagsReg & FLG_OVER));
    HighlightFlagRow(FLG_UNDER_ROW, (FlagsReg & FLG_UNDER));
    HighlightFlagRow(FLG_ZERO_ROW, (FlagsReg & FLG_ZERO));
    HighlightFlagRow(FLG_OVER_ROW, (FlagsReg & FLG_OVER));
    HighlightFlagRow(FLG_INTERRUPT_ROW, (FlagsReg & FLG_IN_INT));
    HighlightFlagRow(FLG_SIGNED_ROW, (FlagsReg & FLG_SIGNED));
    HighlightFlagRow(FLG_INT_ENA_ROW, (FlagsReg & FLG_INTENA));
    HighlightFlagRow(FLG_FAULT_ROW, (FlagsReg & FLG_FAULT));
    wrefresh(FlagWin);
}

// Update IHAP and FHAP display on screen
void UI::DrawHAPs(uint32_t FHAPAddr, uint32_t IHAPAddr)
{
    mvwaddstr(HAPsWin, FHAP_ADDR_ROW, FHAP_ADDR_COL, (boost::format("0x%08X") % FHAPAddr).str().c_str());
    mvwaddstr(HAPsWin, IHAP_ADDR_ROW, IHAP_ADDR_COL, (boost::format("0x%08X") % IHAPAddr).str().c_str());
}

// In hex mode, draw a stack dump window
void UI::DrawStack(uint32_t *Stack)
{   // only active in hex mode
    return;
}

// Update the right side of the status line with the run state, formatted right.
void UI::DrawRunState(std::string State)
{
    int col = 79 - State.length();
    if (col < RUN_STATE_COL) // Should never happen, maybe an assert?
        col = RUN_STATE_COL;
    mvprintw(STATUS_ROW, RUN_STATE_COL, RUN_STATE_BLANK);
    mvprintw(STATUS_ROW, col, State.c_str());
    refresh();
}

// Display given string below the register window
void UI::DrawNextInstr(std::string Instruction)
{
    mvprintw(INSTR_ROW, 0, RUN_STATE_BLANK RUN_STATE_BLANK);
    mvprintw(INSTR_ROW, 2, "NEXT: [ %s ]", Instruction.c_str());
    refresh();
}

void UI::DrawMessage(std::string Message)
{
    mvprintw(MESSAGE_ROW, 0, RUN_STATE_BLANK RUN_STATE_BLANK RUN_STATE_BLANK);
    attron(COLOR_PAIR(CP_GREEN));
    mvprintw(MESSAGE_ROW, 0, Message.c_str());
    attroff(COLOR_PAIR(CP_GREEN));
    refresh();
    getch();
    mvprintw(MESSAGE_ROW, 0, RUN_STATE_BLANK RUN_STATE_BLANK RUN_STATE_BLANK);
    refresh();
}

const char *HelpText[] =
{
"COMP-O-TRON 6000 EMULATOR",
"",
"The Comp-o-Tron 6000 is a premium, all-transistor computing device,",
"introduced by the Comp-o-Tron Corporation in 1956. Utilitizing a",
"full 32-bit address width, and a full 32-bit data width, the",
"Comp-o-Tron 6000 is able to hold and process MILLIONS of words of",
"data in its exclusive Stor-o-Tron memory.",
"",
"Like the original Comp-o-Tron 6000, this emulator allows the user",
"to control the flow of execution, and to modify the values",
"contained in memory or in machine registers.",
"",
"Unlike the original Comp-o-Tron 6000, this emulator also allows the",
"user to see the textual representation of instructions (AKA dis-",
"assembly) as well to directly enter instructions in textual mode.",
"",
"Please review the README and HISTORY files for more information.",
"",
"Press any key to continue.",
nullptr,
};

const char *HelpTextB[] =
{
"ACHTUNG! ALLES LOOKENSPEEPERS!",
"",
"Alles touristen und non-technischen looken peepers!",
"Das computenmachine ist nicht fur gefingerpoken und mittengrabben.",
"Ist easy schnappen der springenwerk, blowenfusen und poppencorken",
"mit spitzensparken. Ist nicht fuer gewerken bei das dumpkopfen.",
"Das rubbernecken sichtseeren keepen das cotten-pickenen hans in das",
"pockets muss; relaxen und watchen das blinkenlichten.",
"",
"Drucken Sie any keyboardenbutton.",
nullptr,
};


// Modal dialogs
void UI::ShowHelpWindow()
{
    WINDOW *helpwin;
    const char *textline;
    char const **txt;
    int i = 0;

    helpwin = CreateWindow(HELP_WIN_HEIGHT, HELP_WIN_WIDTH, HELP_WIN_Y, HELP_WIN_X);
    wattron(RegWin, COLOR_PAIR(CP_BLUE));
    if (CurrentState == DS_Blinky)
        txt = HelpTextB;
    else
        txt = HelpText;
    textline = txt[i];
    while (textline != nullptr) {
        mvwaddstr(helpwin, i+1, 2, textline);
        textline = txt[++i];
    }
    wrefresh(helpwin);
    refresh();
    getch();
    wborder(helpwin, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(helpwin);
    refresh();
    delwin(helpwin);
    RefreshAll();
    return;
}

void UI::ShowDisasmWindow()
{
    return;
}

bool UI::InputBreakpoint(uint32_t &BP)
{
    bool retval;

    mvprintw(MESSAGE_ROW, 0, RUN_STATE_BLANK RUN_STATE_BLANK);
    attron(COLOR_PAIR(CP_GREEN));
    mvprintw(MESSAGE_ROW, 4, "Input Breakpoint Address: [        ]");
    refresh();
    retval = HexInput(MESSAGE_ROW, 31, BP);
    attroff(COLOR_PAIR(CP_GREEN));
    mvprintw(MESSAGE_ROW, 0, RUN_STATE_BLANK RUN_STATE_BLANK);
    refresh();
    return retval;
}

bool UI::InputAssembly(uint32_t &Addr, std::string Input)
{
    return false;
}

bool UI::InputReg(uint32_t &RegNum, uint32_t &NewVal)
{
    return false;
}

bool UI::InputMem(uint32_t &Addr, uint32_t &NewVal)
{
    return false;
}

bool UI::ShowConfirmation()
{
    int c;

    attron(COLOR_PAIR(CP_RED));
    mvprintw(MESSAGE_ROW, 27, "Exit? Are you sure? (Y/N)");
    attroff(COLOR_PAIR(CP_RED));
    c = getch();
    mvprintw(MESSAGE_ROW, 0, RUN_STATE_BLANK RUN_STATE_BLANK);
    return ((c == 'y') || (c == 'Y'));
}

// Utility function to swap register display from binary to blinky and back. Does not need to know the values
// of the registers; this just swaps characters on the screen.
int UI::RegCharTranslate(int Current)
{
    int retval = Current;

    switch (Current) {
        case ONE_BIN:
            retval = ONE_BLINK;
            break;
        case ONE_BLINK:
            retval = ONE_BIN;
            break;
        case ZERO_BIN:
            retval = ZERO_BLINK;
            break;
        case ZERO_BLINK:
            retval = ZERO_BIN;
            break;
        default:
            // do nothing
            break;
    }
    return retval;
}

// Redraw the register window when the display mode changes
// TODO hex mode
void UI::RedrawRegWindow()
{
    int current;
    for (int reg = 0; reg <= NUMREGS; reg++)
        for (int col = 0; col < 31 + 8; col++) {
            current = mvwinch(RegWin, reg + 1, col + REG_VAL_X);
            current = RegCharTranslate(current);
            waddch(RegWin, current);
        }
    wrefresh(RegWin);
}

// Input a new hex value from the specified location on the screen. Input is uint32 so we can
// limit to 8 characters and only allow 0-9 and A-F.
// This probably could have been done by the forms library, but that seems awful heavy for this.
// Returns true if input is good, false if failure or cancel.
bool UI::HexInput(int Row, int Col, uint32_t &Input)
{
    int c;
    int count {0};
    std::string instr;
    bool retval {false};
    bool done {false};

    move(Row, Col);
    curs_set(1);
    refresh();
    while (!done) {
        c = getch();
        // adding new input to the string
        if (count < 8) {
            if ((c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F')) {
                count++;
                addch(c);
                refresh();
                instr += c;
            }
        }
        // backspace handling
        if ((c == KEY_BACKSPACE) && (count > 0)) {
            count--;
            instr.pop_back();
            move(Row, Col + count);
            addch(' ');
            move(Row, Col + count);
            refresh();
        }
        if ((c == KEY_ENTER) || (c == KEY_STAB) || (c == '\n'))
            done = true;
    }
    if (count > 0) {
        retval = true;
        Input = std::stoul(instr, nullptr, 16);
    }
    // cursor off
    curs_set(0);
    refresh();
    return retval;
}


// Full refresh after a modal dialog blows away the screen
void UI::RefreshAll()
{
    redrawwin(stdscr);
    refresh();
    redrawwin(RegWin);
    wrefresh(RegWin);
    redrawwin(FlagWin);
    wrefresh(FlagWin);
    redrawwin(HAPsWin);
    wrefresh(HAPsWin);
    // if in hex mode, refresh the stack window as well
}
