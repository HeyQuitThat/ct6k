#include "symref.hpp"
#include <algorithm>
#include <iostream>


bool SymbolTable::AddSymbol(std::string NewName, uint32_t Location, uint32_t LineNum)
{
    // First, see if the symbol has been seen before
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
bool SymbolTable::AddRef(std::string NewName, uint32_t Location, uint32_t LineNum)
{
    // First, see if the symbol has been defined
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
