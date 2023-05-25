# makefile for simple CPU emulator

CXX = g++
CPPFLAGS = -Wall -g -O2 -std=c++14
COMMON_HEADERS = arch.h hw.h instruction.hpp

all: asm6k punch loadprog.h

# The only common code between the assembler and the emulator is the Instruction class
instruction.o: instruction.cpp $(COMMON_HEADERS)

memory.o: memory.cpp memory.hpp $(COMMON_HEADERS)

cpu.o: cpu.cpp cpu.hpp memory.hpp $(COMMON_HEADERS)

symref.o: symref.cpp symref.hpp

segment.o: segment.cpp segment.hpp

ui.o: ui.cpp ui.hpp ui-priv.hpp arch.h

periph.o: periph.cpp periph.hpp hw.h

printotron.o: printotron.cpp printotron.hpp periph.hpp hw.h

emu_main.o: emu_main.cpp cpu.hpp ui.hpp periph.hpp $(COMMON_HEADERS)

asm_main.o: asm_main.cpp $(COMMON_HEADERS)

cardpunch.o: cardpunch.cpp

# nucurses emulator
EMU_OBJS = emu_main.o instruction.o memory.o cpu.o ui.o periph.o printotron.o
ASM_OBJS = asm_main.o instruction.o symref.o segment.o
PUNCH_OBJS = cardpunch.o

emu6k: $(EMU_OBJS)
	g++ -o emu6k $(EMU_OBJS) -lncurses

asm6k: $(ASM_OBJS)
	g++ -o asm6k $(ASM_OBJS)

punch: $(PUNCH_OBJS)
	g++ -o punch $(PUNCH_OBJS)

loadprog.h: asm6k ../examples/loadprog.cta
	./asm6k -b -o loadprog.bin -r ../examples/loadprog.cta ../examples/devs.cta ../examples/finddev.cta ../examples/readcard.cta ../examples/wait.cta ../examples/printmsb.cta
	mv loadprog.bin.h loadprog.h

clean:
	@rm -f *.o *.obj emu6k asm6k punch loadprog.bin loadprog.h

