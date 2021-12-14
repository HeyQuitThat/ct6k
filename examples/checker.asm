* checker.asm - test program to make a checkerboard of the registers
* bonus difficulty - not allowed to just move all 5s or all As into registers

    MOVE 15, R1
    MOVE 2, R2
$INCRLOOP
    INCR R0     * set low bit
    SHIFTL R0, R2, R0
    DECR R1
    JNZERO $INCRLOOP
* R0 is now alternating 1s and 0s.
    INCR R0
    MOVE R0, R1
    NOT R1
    MOVE R0, R2
    MOVE R0, R3
    NOT R3
    MOVE R0, R4
    MOVE R0, R5
    NOT R5
    MOVE R0, R6
    MOVE R0, R7
    NOT R7
    MOVE R0, R8
    MOVE R0, R9
    NOT R9
    MOVE R0, R10
    MOVE R0, R11
    NOT R11
    MOVE R0, R12
    MOVE R1, R13
    MOVE R0, R14
* final move will set IP to an invalid memory location, which will contain all Fs,
* at that point the system will halt
    JMP  0xAAAAAAA9
    HALT   * not really needed
