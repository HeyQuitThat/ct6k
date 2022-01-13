* hello.asm
* Prints "Hello, Edith" on the Print-o-Tron XL, if the printer is found.

* Start by looking for the printer
    CALL $FINDPRT
    OR R0, R0, R0       * no effect but sets zero flag
    JZERO $END

    MOVE I0, R1         * Read status register
    OR R1, R1, R1
    JNZERO $END         * Printer error or not ready

    INCR R0             * R0 now points at output register
    MOVE $TEXT, R1
$OUTLOOP
    MOVE I1, R2
    OR R2, R2, R2
    JZERO $RELEASE
    MOVE R2, I0
    INCR R1
    JMP $OUTLOOP

$RELEASE
    INCR R0             * R0 now points at control register
    MOVE 1, I0          * Line release

$END
    HALT
$FINDPRT
    MOVE 0xFFF00000, R8     * Peripheral map table location
    MOVE 0xF, R9            * Table size
    MOVE 0x4, R10           * Entry size
    MOVE 0x504F5458, R11    * DDN we are looking for
$FPLP
    MOVE I8, R7             * Get DDN
    CMP  R7, R11
    JZERO $FPFOUND
    ADD R10, R8, R8
    DECR R9
    JNZERO $FPLP
* fell out, not found
    MOVE 0x0, R0
    RETURN

$FPFOUND
    INCR R8             * move to base address in table entry
    MOVE I8, R0         * store memory base address in R0 for return
    RETURN

$TEXT
    0x48
    0x65
    0x6C
    0x6C
    0x6F
    0x2C
    0x20
    0x45
    0x64
    0x69
    0x74
    0x68
    0x21
    0
