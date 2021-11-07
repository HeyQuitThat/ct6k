# makefile for simple CPU emulator

CXX = g++
CPPFLAGS = -Wall -g
COMMON_HEADERS = arch.h hw.h instruction.hpp

all: emu asm

# The only common code between the assembler and the emulator is the Instruction class
instruction.o: instruction.cpp $(COMMON_HEADERS)

memory.o: memory.cpp memory.hpp $(COMMON_HEADERS)

cpu.o: cpu.cpp cpu.hpp memory.hpp $(COMMON_HEADERS)

symref.o: symref.cpp symref.hpp

emu_main.o: emu_main.cpp cpu.hpp $(COMMON_HEADERS)

asm_main.o: asm_main.cpp $(COMMON_HEADERS)

#emulator - likely to be more frontends later
EMU_OBJS = emu_main.o instruction.o memory.o cpu.o
ASM_OBJS = asm_main.o instruction.o symref.o

emu: $(EMU_OBJS)
	g++ -o emu $(EMU_OBJS)

asm: $(ASM_OBJS)
	g++ -o asm $(ASM_OBJS)

clean:
	@rm -f *.o emu asm

