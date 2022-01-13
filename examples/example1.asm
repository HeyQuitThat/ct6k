* example1.asm
* very simple test file for ct6k

    MOVE 10000, R0
    MOVE 1, R1
$LOOP
    ADD R1, R2, R2
    INCR R1
    DECR R0
    JNZERO $LOOP
    NOP
    HALT
