// ui-priv.hpp - private defines for ncurses ui module
#ifndef __UI_PRIV_HPP
#define __UI_PRIV_HPP
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
#define MESSAGE_ROW 21
#define HELP_WIN_X 4
#define HELP_WIN_Y 1
#define HELP_WIN_WIDTH 72
#define HELP_WIN_HEIGHT 21

#endif //__UI_PRIV_HPP
