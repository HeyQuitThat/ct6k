// memory.hpp - Prototypes and declarations for memory class
#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#include <cstdint>

#define MEM_DEFAULT_SIZE 1024*1024
// That's 1M words or 4MB.

class Memory {
public:
    Memory(uint32_t);
    Memory();
    ~Memory();
    uint32_t MemRead(uint32_t);
    void MemWrite(uint32_t address, uint32_t value);
private:
    uint32_t limit;
    uint32_t *blob;
};

#endif // __MEMORY_HPP__
