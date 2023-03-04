* printcards.asm
* Reads a deck of cards from the Card-o-Tron, then prints the text
* on the Print-o-Tron

    MOVE 0x1000, R14    * set stack
* Start by looking for the printer
    MOVE 0x504F5458, R0 * DDN of Print-o-Tron
    CALL $FINDDEV
    OR R0, R0, R0       * no effect but sets zero flag
    JZERO $END
    MOVE R0, R10        * Printer register base now in R10
    MOVE I10, R1        * Read status register
    OR R1, R1, R1
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
    MOVE 0x800, R0     * TXTPM
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
    CALL $UNPACK
    OR R0, R0, R0
    JZERO $END
    MOVE R0, R1
    MOVE 0x2100, R0
    CALL $PRINTBUF
    OR R0, R0, R0
    JNZERO $MAINLOOP
$END
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

* UNPACK - unpack MSB data into single words
* RO - address of source buffer
* R1 - address of destination buffer
* R2 - length of buffer (words) (trailing null added)
* On return, R0 contains number of words in buffer
$UNPACK
    SSTATE
    MOVE 24, R3         * shift distance for top byte
    MOVE 16, R4         * shift distance for 2nd byte
    MOVE 8, R5          * shift distance for 3rd byte, no shift for final byte
    MOVE 0xFF, R6       * Mask for single byte
    MOVE R1, R7         * Save off buffer address
$UPLOOP
    MOVE I0, R8         * Packed word
    SHIFTR R8, R3, R9   * move high byte into R9
    AND R9, R6, R9      * mask off
    MOVE R9, I1         * save
    INCR R1
    SHIFTR R8, R4, R9   * second byte
    AND R9, R6, R9      * mask off
    MOVE R9, I1         * save
    INCR R1
    SHIFTR R8, R5, R9   * third byte
    AND R9, R6, R9      * mask off
    MOVE R9, I1         * save
    INCR R1
    AND R8, R6, R8      * final byte
    MOVE R8, I1
    INCR R1
    INCR R0
    DECR R2
    JNZERO $UPLOOP
    SUB R1, R7, R0       * Calculate count and return
    LSTATE
    RETURN

* PRINTBUF - send buffer of unpacked chars to printer
* R0 - buffer address
* R1 - length (also stops on null buffer)
* On return, R0 contains number of characters printed
$PRINTBUF
    SSTATE
    MOVE I10, R3        * Read status register
    OR R3, R3, R3
    JNZERO $PBFAIL      * Printer error or not ready
    MOVE R10, R11
    INCR R11            * R11 now points to output reg
$OUTLOOP
    MOVE I0, R2
    OR R2, R2, R2
    JZERO $RELEASE
    MOVE R2, I11
    INCR R0
    DECR R1
    OR R1, R1, R1
    JZERO $RELEASE
    JMP $OUTLOOP

$RELEASE
    INCR R11            * R11 now points at control register
    MOVE 1, I11         * Line release
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
