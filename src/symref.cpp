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

// symref.cpp - contains function declarations for the symbol table object

#include "symref.hpp"
#include <algorithm>
#include <iostream>


// Add a new symbol to the table. If it's been seen before, there will be refs
// without address information, so just update the head. Otherwise, add a new list
// to the table.
// Returns true if an error occurred.
bool SymbolTable::AddSymbol(std::string NewName, uint32_t Location, uint32_t LineNum, CodeSegment *Seg)
{
    return AddDef(NewName, Location, LineNum, Seg, false);
}

// Add a defined value that was set using the .VAL directive. The difference with these is that
// we don't need to calculate an offset when we update the binary; we just use the given value.
// Returns true on error
bool SymbolTable::AddValue(std::string NewName, uint32_t Location, uint32_t LineNum, CodeSegment *Seg)
{
    return AddDef(NewName, Location, LineNum, Seg, true);
}

// Add a reference to a symbol. If the symbol has not been declared, start a
// new list with this ref, it will (hopefully) be populated later.
// Returns true on error.
bool SymbolTable::AddRef(std::string NewName, uint32_t Location, uint32_t LineNum, CodeSegment *Seg)
{
    if (NewName.length() < 2)
        return true;
    // See if the symbol has been defined
    auto it = std::find_if(HeadList.begin(), HeadList.end(),
                           [NewName](SymbolHead &r){return (r.Name == NewName);});
    if (it == HeadList.end()) {
        // not found, begin a list but mark it unknown
        HeadList.push_front({NewName, false, false, 0, 0, nullptr, });
        it = HeadList.begin();
    }
    it->Refs.push_back({Location, LineNum, Seg});
    return false;
}


// Check the symbol table for consistency. Returns true if all symbols are accounted
// for and there are no references with undefined symbols.
bool SymbolTable::IsTableCorrect()
{
    for (auto& Sym : HeadList) {
        // Unknown symbol with refs is fatal
        if (Sym.Known == false) {
            std::cerr << "Fatal: symbol " << Sym.Name << " used but not defined at line " <<
                         Sym.SrcLine << "of file" << Sym.Seg->GetFilename() << "\n";
            return false;
        }
    }
    return true;
}
// TODO refactor call IsTableCorrect
// Update the fully-assembled segment in memory with all of the symbol
// information in the table.
// Returns true if error.
bool SymbolTable::UpdateSegment(CodeSegment *Segment)
{
    for (auto& Sym : HeadList) {
        // Unknown symbol with refs is fatal
        if (Sym.Known == false) {
            std::cerr << "Fatal: symbol " << Sym.Name << " used but not defined at line " <<
                         Sym.SrcLine << "of file" << Sym.Seg->GetFilename() << "\n";
            return true;
        }
        // Known symbol with no refs is not an error - we just carry on.
        for (auto Ref : Sym.Refs) {
            if (Ref.Seg == Segment) {
                if (Segment->ReadWord(Ref.SegOffset)) {
                    std::cerr << "Fatal internal error: symbol " << Sym.Name <<
                                 " ref is nonzero at line " << Sym.SrcLine << "of file" <<
                                 Segment->GetFilename() << "\n";
                    return true;
                } else {
                    if (Sym.IsValue)
                        Segment->ModifyWord(Ref.SegOffset, Sym.Offset);
                    else
                        Segment->ModifyWord(Ref.SegOffset, Sym.Seg->GetBase() + Sym.Offset);
                }
            }
        }
    }
    return false;
}

bool SymbolTable::AddDef(std::string NewName, uint32_t Location, uint32_t LineNum, CodeSegment *Seg, bool IsValue)
{
    // This checks for $ alone on a line, or followed by non-alpha character.
    if (NewName.length() == 0)
        return true;
    // See if the symbol has been seen before
    auto it = std::find_if(HeadList.begin(), HeadList.end(),
                           [NewName](SymbolHead &r){return (r.Name == NewName);});
    if (it != HeadList.end()) {
        // If it's in the table, and listed as known, error (duplicate symbol def)
        if (it->Known == true) {
            std::cerr << "Fatal: symbol " << NewName << " defined multiple times\n";
            return true;
        } else {
            // just update the existing definition
            it->Known = true;
            it->Offset = Location;
            it->SrcLine = LineNum;
            it->Seg = Seg;
            it->IsValue = IsValue;
        }
    } else {
        // It's not in the table, add definition as known
        HeadList.push_front({NewName, true, IsValue, Location, LineNum, Seg, });
    }
    return false;

}
