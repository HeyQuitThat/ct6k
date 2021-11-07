// memory.cpp - classes for machine memory.
#include "memory.hpp"
#include <new>

Memory::Memory(uint32_t size)
{
    blob = new uint32_t[size]{0};
    limit = size;
};
Memory::Memory()
{
    blob = new uint32_t[MEM_DEFAULT_SIZE]{0};
    limit = MEM_DEFAULT_SIZE;
}
Memory::~Memory()
{
    delete[] blob;
};

uint32_t Memory::MemRead(uint32_t address)
{
    if (address <= limit)
        return blob[address];
    else
        // No memory present at this address, the data lines
        // float to 1.
        return 0xFFFFFFFF;
};
void Memory::MemWrite(uint32_t address, uint32_t value)
{
    if (address <= limit)
        blob[address] = value;
    // No error if address is out of range, value just disappears
    // like in a a real (vintage) CPU.
};
