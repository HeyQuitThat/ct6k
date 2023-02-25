/*
    The Comp-o-Tron 6000 software is Copyright (C) 2022 Mitch Williams.

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

/* hw.h - definitions of built-in hardware */

#ifndef __HW_H__
#define __HW_H__

#include <cstdint>

/* The DDN is the Digital Device Name - a unique identifier for each device */
#define BASE_IO_MEM 0xFFF00000  // Reserve 20 bits or 1M words for IO space

/* The peripheral map is a sixteen-entry array of the PeriphMapEntry structure,
 * located at BASE_IO_MEM. It's hard-wired via plugboard and must be updated by a
 * Comp-o-Tron engineer any time a peripheral is added or removed.
 * Although the table is guaranteed to be 16 entries long, if the program finds
 * an all-zero DDN, it may abort processing; this indicates the last valid entry.
 *
 * The contents of each entry indicate to the program what memory areas it should
 * use to control a given peripheral device.
 *
 * NOTE: although it is certainly possible for a clever programmer to dump this
 * table and hard-code the entries into his code, Comp-o-Tron engineers recommend
 * against this practice as it will cause programs to fail unexpectedly if hardware
 * is added or removed. Additionally, it is possible that programs written using
 * this technique will be able to run unmodified on other Comp-o-Tron 6000 computing
 * devices!
 *
 * NOTE 2: For simplicity in address handling, the Comp-o-Tron 6000 currently supports
 * a maximum of 15 attached devices. The 16th entry in the table is guaranteed to be
 * all zeros.
 */

struct PeriphMapEntry {
    uint32_t DDN;
    uint32_t Base_Addr;
    uint32_t IOMemLen;
    uint32_t Interrupt;
};
#define PERIPH_NO_INTERRUPT 0xFFFFFFFF

#define PERIPH_MAP_BASE (BASE_IO_MEM)
#define PERIPH_MAP_SIZE 4   // words
#define PERIPH_MAP_ENTRIES 16


/* Print-o-Tron XL full-width matrix imager */
#define POT_DDN                     0x504F5458
#define POT_MEM_SIZE                4   // number of words used for control
#define POT_REG_STATUS              0x0 // offset in given memory region
// values in status register
#define POT_STATUS_OK               0x0
#define POT_STATUS_BUSY             0x7FFFFFFF      // not an error but busy printing. Retry later.
#define POT_STATUS_ERR              0x80000000      // High bit indicates error
#define POT_STATUS_NO_PAPER         0x1             // individual error bits ORed with error bit for true status
#define POT_STATUS_JAM              0x2

#define POT_REG_OUTPUT              0x1             // Write a single character to the low octet of this word
                                                    // Characters are encoded with Comp-o-Tron proprietary innovative
                                                    // Print-o-Tron character set. See 'man ascii' for more information.
#define POT_REG_CONTROL             0x2
#define POT_CONTROL_LINE_RELEASE    0x1             // Write this value to the control register to end the current
                                                    // line and release it to the print head.

#define POT_CONTROL_PAGE_RELEASE    0x2             // Write this value to the control register to eject the current
                                                    // page and load a new one. Erases any text in the current line
                                                    // buffer.
/* (From the user guide)
 *
 * To use the Print-o-Tron, first read the status register. If it is zero, then the printer
 * is ready to accept a line of text. Write individual characters to the output register,
 * with each character encoded in the high octet of the word. Non-printing characters other
 * than space are ignored. When the line is complete, write a 1 to the control register to
 * release the line to the printer. The line buffer is then cleared, ready for new input.
 *
 * Characters written past the width of the paper (80 or 132) are ignored. The Print-O-Tron XL
 * is unable to report the type and size of paper loaded; it is assumed that the operator has
 * loaded the correct paper before beginning the print job.
 *
 * To end the current page and load a new one, write 2 to the control register. This does not
 * print any characters, but clears the line buffer nonetheless. The Print-o-Tron XL assumes
 * standard 60-line pages. If nonstandard page sizes are used, then the page release function
 * should not be used. The program will have to keep track of printed lines and simulate a
 * page release with multiple line releases.
 */

/* Card-o-Tron 3CS (scanner half) */
#define COTS_DDN                0x33435352
#define COTS_MEM_SIZE           43
#define COTS_REG_STATUS         0x0
#define COTS_STATUS_READY       0x00000001
#define COTS_STATUS_HOLL        0x00000002  // If set, using Hollerith cards, otherwise COT cards
                                            // Hollerith cards read 24 bits into 40 words, data is
                                            // not otherwise translated. Significant software work
                                            // may be needed to interpret these cards.
                                            // Comp-o-Tron cards are slightly higher density and
                                            // have no restrictions on adjacent punches, unlike
                                            // Hollerith cards. They decode into 32 words of 32
                                            // bits. If card were written on a Comp-o-Tron Model 3,
                                            // some software work may be required to interpret them
                                            // from their original format of 64 words of 16 bits.
#define COTS_STATUS_EMPTY       0x00000004  // Card reader is empty.
#define COTS_STATUS_READING     0x00000008  // Read in progress
#define COTS_STATUS_COMPLETE    0x00000010  // Read complete, data is valid
#define COTS_STATUS_ERR_CSUM    0x80000000  // Checksum error on Comp-o-Tron card
#define COTS_STATUS_ERR_MECH    0x40000000  // Device has jammed or card has failed to feed
#define COTS_REG_COMMAND        0x1
#define COTS_CMD_READ           0x00000001  // Read next card
#define COTS_CMD_ABORT          0x80000000  // Abort job, unload all cards
#define COTS_REG_CARD_INFO      0x2         // Card type, decoded from the card itself
#define COTS_INFO_LEN_MASK      0x0000003f  // Number of words read from card
#define COTS_INFO_BIN           0x00000100  // General data, no encoding
#define COTS_INFO_TXTU          0x00000200  // Unpacked text, one char per word, low octet
#define COTS_INFO_TXTPL         0x00000800  // Packed text, least significant octet first
#define COTS_INFO_TXTPM         0x00001000  // Packed text, most significant octet first
#define COTS_INFO_CODE          0x00002000  // Binary program data. First word is destination address
#define COTS_REG_READ_BUF       0x3
#define COTS_READ_BUF_LEN       40          // 40 words, 32 used for COT cards

/* Card-o-Tron 3CS (punch half) */
#define COTP_DDN                0x33435357
#define COTP_MEM_SIZE           36
#define COTP_REG_STATUS         0x0
#define COTP_STATUS_READY       0x00000001  // Ready to punch, cards loaded
#define COTP_STATUS_BUSY        0x00000002  // Device is currently punching a card
#define COTP_STATUS_EMPTY       0x00000004  // no cards in hopper to punch
#define COTP_STATUS_ERR_DATA    0x80000000  // card failed readback verification
#define COTP_STATUS_ERR_MECH    0x40000000  // Device has jammed or card has failed to feed
#define COTP_REG_COMMAND        0x1
#define COTP_CMD_WRITE          0x00000001  // Write contents of write buffer to card
#define COTP_CMD_FLUSH          0x00000010  // Job complete, drop written cards to output hopper
#define COTP_REG_CARD_INFO      0x2         // Card information, must be set before issuing write command
#define COTP_INFO_LEN_MASK      0x0000003f  // Number of words to punch
#define COTP_INFO_BIN           0x00000100  // General data, no encoding
#define COTP_INFO_TXTU          0x00000200  // Unpacked text, one char per word, low octet
#define COTP_INFO_TXTPL         0x00000400  // Packed text, least significant octet first
#define COTP_INFO_TXTPM         0x00000800  // Packed text, most significant octet first
#define COTP_INFO_CODE          0x00001000  // Binary program data. First word is destination address
#define COTP_INFO_TYPE_MASK     0x00001f00
#define COTP_REG_WRITE_BUF      0x3
#define COTP_WRITE_BUF_LEN      32          // 32 words

/* The Card-o-Tron 3CS combination card scanner/puncher is actually two devices in one!
 * The scanner half is a high-speed device (600CPM) that can read both Hollerith and Comp-o
 * Tron cards, and will automatically detect the type of card read.
 *
 * The punch side of the device can punch Comp-o-Tron cards at a speedy 300CPM under software
 * control. (Unfortunately, the device cannot punch Hollerith cards due to some pesky patents
 * held by our competition. Sorry. We tried.)
 *
 * Each side of the device operates independently, though both are powered by the same drive-
 * train. We've provided an ample 6 Horsepower 3-phase motor to handle the machine, and it
 * provides plenty of power to both scan and punch at the same time. (N.B. there is currently
 * no software support for this type of operation. But we didn't want hold our customers back.)
 *
 * To scan cards with the Card-o-Tron, first read the scanner's status register. If the returned
 * value is 1 or 3, the device has been loaded with cards and is ready to scan.
 * Write a 1 to the command register, then poll the status register until the "complete" bit is
 * set. This will take approximately 100 millseconds, during which time other processing may take
 * place. Once the status register indicates that data is ready, you may read and decode the info
 * register and read the card data from the data buffer. The data buffer will remain valid until
 * another read command has been issued.
 *
 * Once you have read the data, repeat the process, starting with a read of the status register.
 * When the status register reads "empty", the job is complete; no more cards remain to be read.
 *
 * To punch cards with the Card-o-Tron, first read the punch's status register. If the returned
 * value is 1, the device has been loaded with blank cards and is ready to punch.
 * Begin by writing data to the write buffer, then program the info register with the appropriate
 * size and data type. Finally, write a 1 to the command register. Poll the status register until
 * the busy bit is no longer set (approximately 200ms), the repeat the process for the next card
 * to be punched.
 *
 * When complete, write the flush bit in the command register to end the job and drop the punched
 * cards into the output hopper.
 *
 * N.B. For the emulator, obviously we don't use actual cards. Instead, we use text files.
 * The format is:
 * <type> length: words
 * Type is:
 *  B - Binary
 *  U - Unpacked text, one ASCII character per word
 *  L - Packed text, LSB first
 *  M - Packed text, MSB first
 *  C - Code, first word is the address for the remaining data.
 *
 * Length is a decimal number from 1-32 (zero length would be a blank card, not supported)
 * Words may be decimal or hexadecimal if preceded by 0x. Words are written or read as-is
 * and no formatting or adjustment is done.
 * Whitespace is ignored except as a separator. Data for a single card may span multiple
 * lines.
 * Formatting errors on read will set the ERR_MECH bit in the status register.
 */



/* Tape-o-Tron 1200 */
#define TOT_DDN                 0x544F5412
/* Disc-o-Tron random-access storage */
#define DOT_DDN                 0x444F5400

/* Tick-o-Tron MS */
/* Millisecond-precision timer and real-time clock device with optional wet-cell battery backup. */
#define TKOT_DDN                0x5549434B

#endif /* !__HW_H__ */
