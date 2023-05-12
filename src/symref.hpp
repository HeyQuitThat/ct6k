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
#include "segment.hpp"


// A single symbol reference - includes the source line and the address in the output code
struct SymbolRef {
    uint32_t SegOffset;
    uint32_t SrcLine;
    CodeSegment *Seg;
};

// The head of a list of refs for a single symbol. This contains the name of the symbol
// and information about where it's declared.
struct SymbolHead {
    std::string Name;
    bool Known;
    bool IsValue;   // not an address but a value set in the program
    uint32_t Offset;
    uint32_t SrcLine;
    CodeSegment *Seg;
    std::vector<SymbolRef> Refs;
};

// The main SymbolTable class
class SymbolTable {
public:
    bool AddSymbol(std::string NewName, uint32_t Location, uint32_t LineNum, CodeSegment *Seg);
    bool AddValue(std::string NewName, uint32_t Location, uint32_t LineNum, CodeSegment *Seg);
    bool AddRef(std::string NewName, uint32_t Location, uint32_t LineNum, CodeSegment *Seg);
    bool IsTableCorrect();
    bool UpdateSegment(CodeSegment *Segment);
private:
    std::forward_list<SymbolHead> HeadList;
    bool AddDef(std::string NewName, uint32_t Location, uint32_t LineNum, CodeSegment *Seg, bool IsValue);
};

#endif // __SYMREF_HPP__
