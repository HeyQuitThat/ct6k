* stacktest.asm - ct6k stack validation, with some fun math thrown in
    MOVE 0xF0F0F0F0, R0
    MOVE 0x1000, R14  * stack pointer
    MOVE 0x2000, R10
    SETIHAP R10
    MOVE I0, R3
    INTENA
    PUSH R0
    POP R1
    DECR R1
    AND R0, R1, R2
    XOR R0, R0, R0 * should be 0
    MOVE 0x3, R3
    MOVE 0x4, R4
    MOVE 0x5, R5
    MOVE 0x6, R6
    MOVE 0x7, R7
    MOVE 0x8, R8
    SSTATE
    DECR R0
    NOT R1
    NOT R2
    NOT R3
    NOT R4
    NOT R5
    NOT R6
    NOT R7
    NOT R8
    LSTATE * R0 and R_IP should not affected here
    MOVE 0x3, R3
    SHIFTR R1, R3, R4
    INTDIS
    MOVE 0x10, R0
    MOVE 0x11, R1
    CMP R1, R0
    JOVER $BEYOND * should jump
$END
    HALT

$BEYOND
    CMP R0, R0
    JNUNDER $END  * should jump
    NOP
    HALT
