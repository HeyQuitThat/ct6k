
# Define common headers
set(COMMON_HEADERS arch.h hw.h instruction.hpp)

set(ASM_OBJS
    asm_main.cpp
    instruction.cpp
    symref.cpp
    segment.cpp
)

set(PUNCH_OBJS
    cardpunch.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# Add executable targets

add_executable(asm6k ${ASM_OBJS})

add_executable(punch ${PUNCH_OBJS})

# Add a custom command for loadprog.h generation
add_custom_command(
    OUTPUT loadprog.h
    COMMAND ./asm6k -b -o loadprog.bin -r ${CMAKE_CURRENT_SOURCE_DIR}/loadprog.cta ${CMAKE_CURRENT_SOURCE_DIR}/../examples/devs.cta ${CMAKE_CURRENT_SOURCE_DIR}/../examples/finddev.cta ${CMAKE_CURRENT_SOURCE_DIR}/../examples/readcard.cta ${CMAKE_CURRENT_SOURCE_DIR}/../examples/wait.cta ${CMAKE_CURRENT_SOURCE_DIR}/../examples/printmsb.cta
    COMMAND ${CMAKE_COMMAND} -E rename loadprog.bin.h loadprog.h
    DEPENDS asm6k loadprog.cta
)
add_custom_target(loadprog_header ALL DEPENDS loadprog.h)
set(LOADPROG_HEADER_DIR ${CMAKE_CURRENT_BINARY_DIR})
#set(TOOLSDIR ${CMAKE_CURRENT_SOURCE_DIR})

# Add machine library for GUI to use
add_library(Machine)

target_sources(Machine
	PRIVATE
	instruction.cpp
	cpu.cpp
	memory.cpp
	periph.cpp
	printotron.cpp
	cardotron.cpp

	PUBLIC
	FILE_SET HEADERS
	FILES
		instruction.hpp
        cpu.hpp
        hw.h
        arch.h
        memory.hpp
        periph.hpp
        printotron.hpp
        cardotron.hpp
)


# Clean rule
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "*.o *.obj emu6k asm6k punch loadprog.bin loadprog.h")
