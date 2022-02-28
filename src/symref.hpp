/*
    The Comp-o-Tron 6000 software is Copyright (C) 2022 Mitch Williams.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// symref.hpp - declarations for the SymbolTable class and other dependent classes
// This is the core of the assembler's symbol table.

#ifndef __SYMREF_HPP__
#define __SYMREF_HPP__

#include <vector>
#include <cstdint>
#include <forward_list>
#include <string>


// A single symbol reference - includes the source line and the address in the output code
struct SymbolRef {
    uint32_t RefAddr;
    uint32_t SrcLine;
};

// The head of a list of refs for a single symbol. This contains the name of the symbol
// and information about where it's declared.
struct SymbolHead {
    std::string Name;
    bool Known;
    uint32_t Addr;
    uint32_t SrcLine;
    std::vector<SymbolRef> Refs;
};

// The main SymbolTable class
class SymbolTable {
public:
    bool AddSymbol(std::string NewName, uint32_t Location, uint32_t LineNum);
    bool AddRef(std::string NewName, uint32_t Location, uint32_t LineNum);
    bool UpdateExecutable(std::vector<uint32_t> &Blob);
private:
    std::forward_list<SymbolHead> HeadList;
};

#endif // __SYMREF_HPP__
