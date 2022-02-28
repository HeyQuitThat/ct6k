* fault.asm - test fault handling

    MOVE 0x100, R14     * set stack
    MOVE $TABLE, R0
    MOVE $HND, R1
    SETFHAP R0
    MOVE 7, R2
$LOOP
    MOVE R1, I0
    INCR R0
    DECR R2
    JNZERO $LOOP

$BAD
    0xf3000000
    MOVE 0x55555555, R0  * just to show we recovered
    HALT

$HND    * handler
    POP R0                  * IP is now in R0
    MOVE I0, R1             * display for debugging purposes
    MOVE 0xF0000000, I0     * NOP instruction
    PUSH R0                 * restore the stack
    IRET
    MOVE 0x1, R0            * should never get here!
    HALT

$TABLE
    0
    0
    0
    0
    0
    0
    0
    0
