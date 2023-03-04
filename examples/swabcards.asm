* swabcards.asm
* Reads a deck of cards from the Card-o-Tron Scanner, then changes
* them from TXTPM to TXTPL or vice-versa. The modified cards are then
* punched with the Card-o-Tron Puncher

    MOVE 0x1000, R14    * set stack
* Start by looking for the punch
    MOVE 0x33435357, R0 * DDN of Comp-o-Tron Punch
    CALL $FINDDEV
    OR R0, R0, R0       * no effect but sets zero flag
    JZERO $END
    MOVE R0, R10        * Puncher register base now in R10
    MOVE I10, R1        * Read status register
    CMP 0x1, R1         * Ready bit should the only one set
    JNZERO $END         * Printer error or not ready

* Now find the card reader
    MOVE 0x33435352, R0
    CALL $FINDDEV
    OR R0, R0, R0
    JZERO $END
    MOVE R0, R11        * Card scanner register base now in R11

$MAINLOOP
    MOVE I11, R1
    MOVE 0x1, R2
    AND R1, R2, R2
    JZERO $END
    MOVE 1, R0
    ADD R0, R11, R12    * R12 now points to scanner command reg
    MOVE 1, I12         * Issue read command
$READLOOP
    CALL $WAIT
    MOVE I11, R1
    MOVE 0x8, R2        * Check for busy bit
    AND R1, R2, R2
    JNZERO $READLOOP
* Reading complete, now transfer data into buffer
    MOVE 0x10, R2       * check for completion
    AND R1, R2, R2
    JZERO $END
    MOVE 2, R0          * read card info register
    ADD R0, R11, R12
    MOVE I12, R5        * Info register, keep this around for later
    MOVE 0xC00, R0      * TXTPM || TXTPL
    AND R5, R0, R0
    JZERO $END

*OK, we have valid data in the right format, move it to the buffer
    MOVE 0x2000, R0
    CALL $READCARD
    OR R0, R0, R0
    JZERO $END
    MOVE 0x2000, R0
    MOVE 0x2100, R1
    MOVE 0xFF, R2
    AND R5, R2, R2      * strip length from info register contents
    CALL $SWAB          * no return code
* switch info reg
    MOVE R5, R0
    CALL $SWITCHTYPE    * no return code to test
    MOVE R0, R1
    MOVE 0x2100, R0     * buffer
    CALL $PUNCHBUF
    OR R0, R0, R0
    JZERO $END
$WRITELOOP
    CALL $WAIT
    MOVE I10, R1
    MOVE 0x1, R2        * Check for ready
    AND R1, R2, R2
    JZERO $WRITELOOP

    JMP $MAINLOOP
$END
    INCR R10            * R10 now points to punch command reg
    MOVE 0x10, I10      * Send flush command to end job
    HALT



* FINDDEV - find a device in the peripheral map.
* DDN is passed in R0, if found, base address returned in R0
$FINDDEV
    SSTATE
    MOVE 0xFFF00000, R8     * Peripheral map table location
    MOVE 0xF, R9            * Table size
    MOVE 0x4, R10           * Entry size
$FDLP
    MOVE I8, R7             * Get DDN
    CMP  R7, R0
    JZERO $FDFOUND
    ADD R10, R8, R8
    DECR R9
    JNZERO $FDLP
* fell out, not found
    MOVE 0x0, R0
    LSTATE
    RETURN

$FDFOUND
    INCR R8             * move to base address in table entry
    MOVE I8, R0         * store memory base address in R0 for return
    LSTATE
    RETURN

* READCARD - copy data from card to buffer
* Address of buffer in R0. Base address of scanner in R11
* Upon return, length read is in R0
$READCARD
    SSTATE
    MOVE 0xFF, R2
    AND R5, R2, R1      * R1 now has size in words
    MOVE R1, R2         * Stash in R2
    INCR R12            * R11 now points to the first data word
$RCLOOP
    MOVE I12, I0
    INCR R12
    INCR R0
    DECR R1
    JNZERO $RCLOOP
$RCDONE
    MOVE R2, R0         * Return buffer length
    LSTATE
    RETURN


* SWAB - swap bytes from LSB to MSB (or vice-versa)
* RO - address of source buffer
* R1 - address of destination buffer
* R2 - length of buffer
$SWAB
    SSTATE
    MOVE 24, R3         * shift distance
    MOVE 8, R4          * shift distance
    MOVE 0xFF00, R5     * Mask for single byte
    MOVE 0x00FF0000, R6
$SWABLOOP
* MSB->LSB, shift right 24 bits, no need to mask
    MOVE I0, R8         * Packed word
    SHIFTR R8, R3, R10  * move high byte into R11, no need to mask

* 2nd MSB -> 3rd MSB, shift right 8, mask 0x0000ff00
    SHIFTR R8, R4, R9   * Second byte, shift right to 3rd byte
    AND R9, R5, R9      * R9 now masked off
    OR R9, R10, R10     * ...and placed in R10

* 3rd MSB -> 2nd MSB, shift left 8, mask 0x00ff0000
    SHIFTL R8, R4, R9   * Now move third word up to second
    AND R9, R6, R9      * R9 now masked off
    OR R9, R10, R10     * ...and placed in R10

* LSB -> MSB, shift left 24, no mask
    SHIFTL R8, R3, R9
    OR R9, R10, R10     * ...and placed in R10

    MOVE R10, I1        * place swabbed word
    INCR R1
    INCR R0
    DECR R2
    JNZERO $SWABLOOP
    LSTATE
    RETURN

* SWITCHTYPE - switches type bits in output info
* R0 - input info reg
* On return R0 contains the data for the output
* info register to be programmed to the punch.
* Note that we don't need to test for other card types,
* that is done before this is called.
$SWITCHTYPE
    SSTATE
    MOVE R0, R1     * Will build new value in R1
    MOVE 0xFF, R2
    AND R1, R2, R1  * Strip off length
    MOVE 0x800, R2
    AND R2, R0, R3  * test TXTPM bit
    JZERO $TXTPL    * if not set, jump to end and set it
    MOVE 0x400, R2
    JMP $SWFIN
$TXTPL
    MOVE 0x800, R2  * otherwise, set the TXTPL

$SWFIN
    OR R2, R1, R1
    MOVE R1, R0
    LSTATE
    RETURN


* PUNCHBUF - send buffer of words to card punch
* R0 - buffer address
* R1 - card info reg, including length
$PUNCHBUF
    SSTATE
    MOVE I10, R2        * Read status register
    MOVE 1, R3
    AND R2, R3, R3
    JZERO $PBFAIL       * Punch not ready
    INCR R10            * R10 now points to command reg
    MOVE R10, R12
    INCR R12            * R12 now points to card info reg

    MOVE R1, I12        * set card info
    MOVE 0xFF, R2
    AND R1, R2, R2      * R2 now contains length

$PUNCHLOOP
    INCR R12
    MOVE I0, I12        * program next word
    INCR R0
    DECR R2
    OR R2, R2, R2
    JZERO $PUNCHIT
    JMP $PUNCHLOOP

$PUNCHIT
    MOVE 1, I10         * Punch the card
    JMP $PBEND

$PBFAIL
    MOVE 0, R0
$PBEND
    LSTATE
    RETURN

* WAIT - delay execution for approximately 10ms
* No register effects, R0 unchanged
$WAIT
    SSTATE
    MOVE 1000, R6
$WAITLOOP
    NOP
    NOP
    NOP
    NOP
    NOP
    DECR R6
    JNZERO $WAITLOOP
    LSTATE
    RETURN
