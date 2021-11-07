#ifndef __SYMREF_HPP__
#define __SYMREF_HPP__

#include <vector>
#include <cstdint>
#include <forward_list>
#include <string>



struct SymbolRef {
    uint32_t RefAddr;
    uint32_t SrcLine;
};

struct SymbolHead {
    std::string Name; // might need fixed-size char array
    bool Known;
    uint32_t Addr;
    uint32_t SrcLine;
    std::vector<SymbolRef> Refs;
};

class SymbolTable {
public:
    bool AddSymbol(std::string NewName, uint32_t Location, uint32_t LineNum);
    bool AddRef(std::string NewName, uint32_t Location, uint32_t LineNum);
    bool UpdateExecutable(std::vector<uint32_t> &Blob);
private:
    std::forward_list<SymbolHead> HeadList;
};

#endif // __SYMREF_HPP__
