/*
    The Comp-o-Tron 6000 software is Copyright (C) 2022-2023 Mitch Williams.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// ct6k-info.h - contains information and text for the UI to display.
#ifndef CT6KINFO_H
#define CT6KINFO_H

#define WINDOW_TITLE "Comp-o-Tron 6000"

#define VERSION_MAJOR "0"
#define VERSION_MINOR "5"
#define VERSION_SUBMINOR "0"
#define VERSION_EXTRA

// Text displayed when the Help/About menu option is triggered.
// The version text from above is appended.
#define INFO_TEXT "The Comp-o-Tron 6000 software is Copyright (C) 2022-2023 Mitch Williams.<br>" \
"<br>" \
"Built using QT libraries and QT Creator.<br><br>" \
"Version "

// Text displayed when the Help/Instructions menu option is triggered.
#define INSTRUCTION_TEXT "<center><b>COMP-O-TRON 6000 EMULATOR</b></center><br>" \
"<br>" \
"The Comp-o-Tron 6000 is a premium, all-transistor computing device, " \
"introduced by the Comp-o-Tron Corporation in 1956. Utilitizing a " \
"full 32-bit address width, and a full 32-bit data width, the " \
"Comp-o-Tron 6000 is able to hold and process MILLIONS of words of " \
"data in its exclusive Stor-o-Tron memory.<br>" \
"<br>" \
"Like the original Comp-o-Tron 6000, this emulator allows the user " \
"to control the flow of execution, and to modify the values " \
"contained in memory or in machine registers.<br>" \
"<br>" \
"Unlike the original Comp-o-Tron 6000, this emulator also allows the " \
"user to see the textual representation of instructions (AKA disassembly) " \
"as well to directly enter instructions in textual mode.<br>" \
"<br>" \
"Please review the README and HISTORY files for more information." \

// Text displayed when the Help/Caution! menu item is triggered.
// If you recognize this, you are officially old.
#define WARNING_TEXT "<center><b>ACHTUNG! ALLES LOOKENSPEEPERS!</b></center><br>" \
"<br>" \
"Alles touristen und non-technischen looken peepers!<br>" \
"Das computenmachine ist nicht fur gefingerpoken und mittengrabben. " \
"Ist easy schnappen der springenwerk, blowenfusen und poppencorken " \
"mit spitzensparken. Ist nicht fuer gewerken bei das dumpkopfen. " \
"Das rubbernecken sichtseeren keepen das cotten-pickenen hans in das " \
"pockets muss; relaxen und watchen das blinkenlichten.<br>" \
"<br>" \
"Verwarnen! Readenbooken der README!!<br>"


#endif // CT6KINFO_H
