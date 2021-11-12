// memory.cpp - classes for Comp-o-Tron 6000 memory.
#include "memory.hpp"
#include <new>

// Constructor with size specified by caller
Memory::Memory(uint32_t Size)
{
    Blob = new uint32_t[Size]{0};
    Limit = Size;
};

// Constructor with default size
Memory::Memory()
{
    Blob = new uint32_t[MEM_DEFAULT_SIZE]{0};
    Limit = MEM_DEFAULT_SIZE;
}

// Destructor
Memory::~Memory()
{
    delete[] Blob;
};

// Read a word from the specified memory location. No errors returned from this function.
uint32_t Memory::MemRead(uint32_t Address)
{
    if (Address <= Limit)
        return Blob[Address];
    else
        // No memory present at this address, the data lines
        // float to 1.
        return 0xFFFFFFFF;
};

// Write a word to the specified memory location. As above, no errors are reported by this function.
void Memory::MemWrite(uint32_t Address, uint32_t Value)
{
    if (Address <= Limit)
        Blob[Address] = Value;
    // No error if address is out of range, value just disappears
    // like in a a real (vintage) CPU.
};
