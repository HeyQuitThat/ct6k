/* hw.h - definitions of built-in hardware */

#ifndef __HW_H__
#define __HW_H__

/* The DDN is the Digital Device Name - a unique identifier for each device */

#define BASE_IO_MEM 0xFFF00000  // Reserve 20 bits or 1MB for IO space

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

/* Card-o-Tron 3CS (reader half) */
#define COTR_DDN                0x33435352
#define COTR_MEM_SIZE           43
#define COTR_REG_STATUS         0x0
#define COTR_STATUS_READY       0x00000001
#define COTR_STATUS_HOLL        0x00000002  // If set, using Hollerith cards, otherwise COT cards
                                            // Hollerith cards read 24 bits into 40 words, data is
                                            // not otherwise translated. Significant software work
                                            // may be needed to interpret these cards.
                                            // Comp-o-Tron cards are slightly higher density and
                                            // have no restrictions on adjacent punches, unlike
                                            // Hollerith cards. They decode into 32 words of 32
                                            // bits. If card were written on a Comp-o-Tron Model 3,
                                            // some software work may be required to interpret them
                                            // from their original format of 64 words of 16 bits.
#define COTR_STATUS_EMPTY       0x00000004  // Card reader is empty.
#define CORT_STATUS_READING     0x00000008  // Read in progress
#define CORT_STATUS_COMPLETE    0x00000010  // Read complete, data is valid
#define COTR_STATUS_ERR_CSUM    0x80000000  // Checksum error on Comp-o-Tron card
#define COTR_STATUS_ERR_MECH    0x40000000  // Device has jammed or card has failed to feed
#define COTR_REG_COMMAND        0x1
#define COTR_CMD_READ           0x00000001  // Read next card
#define COTR_CMD_ABORT          0x80000000  // Abort job, unload all cards
#define COTR_REG_READ_BUF       0x2
#define COTR_REG_READ_LEN       40          // 40 words, 32 used for COT cards

/* Card-o-Tron 3CS (writer half) */
#define COTW_DDN                0x33435357
#define COTW_MEM_SIZE           35
#define COTW_REG_STATUS         0x0
#define COTW_STATUS_READY       0x00000001  // Ready to punch, cards loaded
#define COTW_STATUS_EMPTY       0x00000004  // no cards in hopper to punch
#define COTW_STATUS_ERR_DATA    0x80000000  // card failed readback verification
#define COTW_STATUS_ERR_MECH    0x40000000  // Device has jammed or card has failed to feed
#define COTW_REG_COMMAND        0x1
#define COTW_CMD_WRITE          0x00000001  // Write contents of write buffer to card
#define COTW_REG_WRITE_BUF      0x2
#define COTW_REG_WRITE_LEN      32          // 32 words

/* Tape-o-Tron 1200 */
#define TOT_DDN                 0x544F5412
/* Disc-o-Tron random-access storage */
#define DOT_DDN                 0x444F5400

/* Tick-o-Tron MS */
/* Millisecond-precision timer and real-time clock device with optional wet-cell battery backup. */
#define TKOT_DDN                0x5549434B

#endif /* !__HW_H__ */
