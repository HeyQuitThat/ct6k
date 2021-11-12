// memory.hpp - Prototypes and declarations for memory class
#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#include <cstdint>

#define MEM_DEFAULT_SIZE 1024*1024
// That's 1M words or 4MB.


// The simplest class - this really could be done with something out of STL, but that's really too heavyweight here.
// This block of memory is allocated once and released once. It never grows or shrinks, and it doesn't need to be
// accessed with an iterator. So we can use a boring old c-style array and add a few methods to access it.
// When Uncle Bob says "program to an interface, not an implementation" this is what he means.
//
class Memory {
public:
    Memory(uint32_t Size);
    Memory();
    ~Memory();
    uint32_t MemRead(uint32_t Address);
    void MemWrite(uint32_t Address, uint32_t Value);
private:
    uint32_t Limit;
    uint32_t *Blob;
};

#endif // __MEMORY_HPP__
