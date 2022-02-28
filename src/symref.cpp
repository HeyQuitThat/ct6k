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
bool SymbolTable::AddSymbol(std::string NewName, uint32_t Location, uint32_t LineNum)
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
            it->Addr = Location;
            it->SrcLine = LineNum;
        }
    } else {
        // It's not in the table, add definition as known
        HeadList.push_front({NewName, true, Location, LineNum, });
    }
    return false;
}

// Add a reference to a symbol. If the symbol has not been declared, start a
// new list with this ref, it will (hopefully) be populated later.
// Returns true on error.
bool SymbolTable::AddRef(std::string NewName, uint32_t Location, uint32_t LineNum)
{
    if (NewName.length() < 2)
        return true;
    // See if the symbol has been defined
    auto it = std::find_if(HeadList.begin(), HeadList.end(),
                           [NewName](SymbolHead &r){return (r.Name == NewName);});
    if (it == HeadList.end()) {
        // not found, begin a list but mark it unknown
        HeadList.push_front({NewName, false, 0, 0, });
        it = HeadList.begin();
    }
    it->Refs.push_back({Location, LineNum});
    return false;
}

// Update the fully-assembled executable in memory with all of the symbol
// information in the table. This is where we check for missing declarations.
// Returns true on error.
bool SymbolTable::UpdateExecutable(std::vector<uint32_t> &Blob)
{
    for (auto it : HeadList) {
        // Unknown symbol with refs is fatal
        if (it.Known == false) {
            std::cerr << "Fatal: symbol " << it.Name << " used but not defined at line " <<
                         it.SrcLine << "\n";
            return true;
        }
        // Known symbol with no refs is not an error - we just carry on.
        for (auto vit : it.Refs) {
            if (Blob[vit.RefAddr]) {
                std::cerr << "Fatal internal error: symbol " << it.Name <<
                             " ref is nonzero at line " << vit.SrcLine << "\n";
                return true;
            } else {
                Blob[vit.RefAddr] = it.Addr;
            }
        }
    }
    return false;
}
