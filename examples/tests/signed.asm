* signed.asm - test of signed math and comparison code

    MOVE 5, R1
    MOVE 7, R2
    SUB R1, R2, R3  * should a big number, with underflow set
    CMP R2, R1
    JNOVER $END
    SIGNED
    SUB R1, R2, R4  * should be a negative number, no underflow
    JUNDER $END
    DECR R4
    MOVE 0, R0
    SUB R0, R4, R5
    UNSIGNED
    SUB R1, R2, R6


$END
    HALT
