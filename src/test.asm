* TEST.ASM
* Assembler test file for simulated CPU

    MOVE 10, R0
    MOVE 1, R1
$LOOP
    ADD R1, R2, R2
    INCR R1
    DECR R0
    JNZERO $LOOP
    NOP
    HALT
