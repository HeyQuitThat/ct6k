/* arch.h - decribes the CPU and its operation */
#ifndef __ARCH_H__
#define __ARCH_H__

#include <stdint.h>

/*
Registers 32 bit wide, endianness in SW (i.e. no multi-word operations)
R[0-15] (4 bit select) 13 are GP.
Any register may be used as an indirect register. In the assembler these are
indicated as I[0-15].
Flags (R13) (R/0)
	Bit 0: Carry/Overflow
	Bit 1: Borrow/Underflow
	Bit 2: Sign
	Bit 3: Zero
	Bit 29: Signed arithemetic active
	Bit 30: Interrupt Enable (global)
	Bit 31: Fault

SP (R14)
  The stack counts UP from SP as things are pushed. Overflow or underflow of
  address space is a fault.
IP (R15)

Data structures in memory:
FHAP (Fault Handler Array Pointer)
   - Array of 16 dwords, addresses pointing to handler routines for faults
IHAP (Interrupt Handler Array Pointer)
   - Array of 4 dwords, addresses pointing to ISRs

Instructions:
[ALU]
MOVE - can use direct value (i.e. inline with code - MOVE 0x1, R0
ADD
SUB
NOT - logical not of all bits in value indicated by dest
AND
OR
XOR
SHIFTR - src1 is value, src2 is number of bits to shift, fault if > 31
SHIFTL - src1 is value, src2 is number of bits to shift, fault if > 31
PUSH
POP
INCR - increments value indicated by dest
DECR - decrements value indicated by dest
SSTATE - saves all registers on stack
LSTATE - restores all registers except IP from stack

[Flow, if address is used, it may be in dest or direct value]
JZERO
JNZERO
JOVER
JNOVER
JUNDER
JNUNDER
JMP
CALL
RETURN
IRET
SIGNED
UNSIGNED
INTENA
INTDIS
HALT
NOP

[Handlers, address in src11]
SETFHAP
SETIHAP

All instructions 32 bits.
High byte - opcode
2nd byte - src1 reg
3rd byte - src2 reg
Low byte - dest reg
Reg byte bits
0-3: register index
5: Param Unused
6: Indirect addr in reg
7: Value in reg

For MOVE, if src1 and src2 bytes are 0xFFFF then the next 32bits is a direct value to be placed in destination.
SHIFTL and SHIFTR shift src1 by number of bits in src2, result in dest. Shift values > 31 generate overflow/underflow.
Both SHIFTL and SHIFTR fill with zeros.

For JMP and related control-flow instructions, setting src1, src2 and dest register bytes to 0xFFFFFF
indicates a direct value for the jump destination. Otherwise, the address to which the CPU should jump
is specified in the destination register. This is a little weird architecturally because the CPU is actually
reading from the destination register, but makes sense semantically to humans: "Jump to this destination".

Memory:
All accesses 32-bits. Memory granularity is 32 bits. No endianness.

Stack starts at SP, builds up.

Faults:
0: Invalid Instruction
1: Invalid Memory
2: Stack Fault
3: Double fault

Interrupts:
0: Clock

Flags:
For CMP instruction, the CPU compares src1 and destination, and updates flags.
  - if src1 == dest, the ZERO flag is set
  - if src1 < dest, the UNDER flag is set
  - if src1 > dest, the OVER flag is set

For logical instructions, the ZERO flag is updated if the result is zero. Other flags are
unmodified.

For math instructions, including INCR and DECR, the following flags are set:
 - ZERO flag is set if the result is zero
 - OVER flag is set if the value overflows
 - UNDER flag is set if the value underflows
Both the OVER and UNDER flags behave appropriately based on whether or not signed arithmetic is
being used.

Interrupt Control:
INTENA and INTDIS - Globally control interrupts. Individual interrupts can be enabled or disabled
by toggling bits in the FLAGS register (R14).
The interrupt enable flag in R14 should not be used to control global interrupts. Use the INTENA
and INTDIS instructions instead.


ISR/Fault handlers:
Initialize tables with SFHAP and SIHAP, addr specified by src1.
Note that FHAP and IHAP are initialized to 0, same as IP, so an early fault may cause issues.
Machine state pushed to stack. Fault bit set if faulted. Interrupt/Fault indicated by bits
in R0. Recursive faults and interrupts are NOT allowed. Interrupts are disabled by faults.
A fault in an ISR, if handled, will be returned to the ISR.
Double fault indicated by high bit set in R0 and HALT state.
After processing, use IRET to return to site of fault/interrupt.

CALL and RETURN do not save any state except IP.
SSTATE and LSTATE save and restore machine state to stack. IP and R0 are NOT restored.
(R0 is expected to be used for return value.)

Initialization:
Execution begins at 0.
All registers set to 0.
SP must be initialized before call/return or fault.
Fault/Interrupt handlers initialized next if used.
If a fault happens with FHAP not set, the CPU halts with R0 set to 3 (Double Fault).
Top of populated memory is at 0xFFFFFFFF. Access out of bounds will give Invalid Memory Fault.
TODO: preload a register with mem size?
*/

/* One source, one dest, can be direct */
#define OP_MOVE		0x01

/* One source, one dest, not direct */
#define OP_CMP		0x08

/* Two sources, one dest, never direct */
#define OP_ADD		0x11
#define OP_SUB		0x12
#define OP_AND		0x13
#define OP_OR		0x14
#define OP_XOR		0x15
#define OP_SHIFTR	0x16
#define OP_SHIFTL	0x17

/* Take single src value */
#define OP_PUSH		0x30
#define OP_SETFHAP	0x3A
#define OP_SETIHAP	0x3B

/* Take single dest value */
#define OP_POP		0x31
#define OP_NOT		0x20
#define OP_INCR		0x21
#define OP_DECR		0x22
#define OP_JZERO	0x32
#define OP_JNZERO	0x33
#define OP_JOVER	0x34
#define OP_JNOVER	0x35
#define OP_JUNDER	0x36
#define OP_JNUNDER	0x37
#define OP_JMP		0x38
#define OP_CALL		0x39

/* Take no register arguments */
#define OP_SSTATE	0x50
#define OP_LSTATE	0x51
#define OP_RETURN	0x52
#define OP_IRET		0x53
#define OP_SIGNED	0x54
#define OP_UNSIGNED	0x55
#define OP_INTENA	0x56
#define OP_INTDIS	0x57
#define OP_NOP		0xF0
#define OP_HALT		0xFF

/* Finally... */
#define OP_INVALID	0	/* no bits set, used to generate a fault */


/* Macros to help build and tear down instructions */
#define __OPSHIFT 24
#define OP_LOAD(_op) (uint32_t)((uint8_t)(_op) << __OPSHIFT)
#define __S1SHIFT 16
#define S1_LOAD(_reg)  (uint32_t)((uint8_t)(_reg) << __S1SHIFT)
#define __S2SHIFT 8
#define S2_LOAD(_reg)  (uint32_t)((uint8_t)(_reg) << __S2SHIFT)
#define __DSHIFT 0 	/* for consistency */
#define DEST_LOAD(_reg) (_reg)

#define GET_OP(_dword) (uint8_t)(((_dword) >> __OPSHIFT) & 0xFF)
#define GET_SRC1(_dword) (uint8_t)(((_dword) >> __S1SHIFT) & 0xFF)
#define GET_SRC2(_dword) (uint8_t)(((_dword) >> __S2SHIFT) & 0xFF)
#define GET_DEST(_dword) (uint8_t)((_dword) & 0xFF)

#define REGNUM_MASK (0xF)
#define REGTYPE_MASK (0xF0)
#define REG_ERR		0x10 /* if this bit is set, CPU will fault */
#define REG_UNUSED	0x20
#define REG_IND		0x40
#define REG_VAL		0x80
#define REG_USED	0xC0 /* if one of these bits is set, reg is in use */
#define REG_VALID	0xE0 /* if none of these bits are set, it's invalid */
#define REG_NULL	0xFF

#define REG_IS_UNUSED(_reg) ((_reg) & REG_UNUSED)
#define REG_IS_INDIRECT(_reg) ((_reg) & REG_IND)
#define REG_IS_VALUE(_reg) ((_reg) & REG_VAL)
#define REG_IS_NULL(_reg) (((_reg) & REG_NULL) == REG_NULL)

#define NUMREGS 	16
#define REG_R0		0x00
#define REG_R1		0x01
#define REG_R2		0x02
#define REG_R3		0x03
#define REG_R4		0x04
#define REG_R5		0x05
#define REG_R6		0x06
#define REG_R7		0x07
#define REG_R8		0x08
#define REG_R9		0x09
#define REG_R10		0x0A
#define REG_R11		0x0B
#define REG_R12		0x0C
#define REG_R13		0x0D
#define REG_FLG		0x0D
#define REG_R14		0x0E
#define REG_SP		0x0E
#define REG_R15		0x0F
#define REG_IP		0x0F

/* Flag bits */
#define FLG_OVER	0x00000001
#define FLG_UNDER	0x00000002
#define FLG_ZERO	0x00000008
#define FLG_IN_INT	0x00000010
#define FLG_INTEN0  0x00010000
#define FLG_INTEN1  0x00020000
#define FLG_INTEN2  0x00040000
#define FLG_INTEN3  0x00080000
#define FLG_SIGNED	0x20000000
#define FLG_INTENA	0x40000000      // Global, controlled by INTENA and INTDIS
#define	FLG_FAULT	0x80000000

/* Fault codes */
#define FAULT_NO_FAULT	0		// used internally
#define FAULT_BAD_INSTR	0x00000001
#define FAULT_BAD_ADDR	0x00000002
#define FAULT_STACK		0x00000003
#define	FAULT_DOUBLE	0x80000000

/* Convenience */
#define MAX_ADDR		0xFFFFFFFF
#define STATE_SIZE		16 //words
#define MAX_STATE_PUSH	(MAX_ADDR - STATE_SIZE)
#define MIN_STATE_POP	(STATE_SIZE)
#define FHAP_SIZE		16
#define MAX_FHAP		(MAX_ADDR - FHAP_SIZE)
#define IHAP_SIZE		32
#define MAX_IHAP		(MAX_ADDR - IHAP_SIZE)

#endif /* !__ARCH_H__ */
