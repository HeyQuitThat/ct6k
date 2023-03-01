# makefile for simple CPU emulator

CXX = g++
CPPFLAGS = -Wall -g -O2 -std=c++14
COMMON_HEADERS = arch.h hw.h instruction.hpp

all: emu6k asm6k

# The only common code between the assembler and the emulator is the Instruction class
instruction.o: instruction.cpp $(COMMON_HEADERS)

memory.o: memory.cpp memory.hpp $(COMMON_HEADERS)

cpu.o: cpu.cpp cpu.hpp memory.hpp $(COMMON_HEADERS)

symref.o: symref.cpp symref.hpp

ui.o: ui.cpp ui.hpp ui-priv.hpp arch.h

periph.o: periph.cpp periph.hpp hw.h

printotron.o: printotron.cpp printotron.hpp periph.hpp hw.h

emu_main.o: emu_main.cpp cpu.hpp ui.hpp periph.hpp $(COMMON_HEADERS)

asm_main.o: asm_main.cpp $(COMMON_HEADERS)

# nucurses emulator
EMU_OBJS = emu_main.o instruction.o memory.o cpu.o ui.o periph.o printotron.o
ASM_OBJS = asm_main.o instruction.o symref.o
PUNCH_OBJS = cardpunch.o

emu6k: $(EMU_OBJS)
	g++ -o emu6k $(EMU_OBJS) -lncurses

asm6k: $(ASM_OBJS)
	g++ -o asm6k $(ASM_OBJS)

punch: $(PUNCH_OBJS)

clean:
	@rm -f *.o emu6k asm6k punch

