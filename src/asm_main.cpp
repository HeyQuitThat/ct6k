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

// First successful assembly November 2, 2021
#include <iostream>
#include <fstream>
#include <cstdint>
#include <forward_list>
#include <string>
#include <vector>
#include "symref.hpp"
#include "arch.h"
#include "instruction.hpp"

// Super simple assembler. Most of the work is done by build_instruction(), so this code just
// needs to handle the symbol table and file I/O.
//
// Input file format:
//  - instructions should be preceded by whitespace, traditionally a tab '\t' character.
//  - comments start with * and MUST begin in the first column if they are alone on a line
//  - comments can follow any instruction or symbol declaration. They must be preceded by
//    whitespace and a * is traditional but not absolutely required.
//  - symbolic address declaration start with $ and MUST begin in the first column.
//  - symbols MUST be alphnumeric ONLY, no punctuation
//  - symbols are case-sensitive
//
// Example source file:
//     * Simple loop to add numbers 1 to 10
//          MOVE 10, R0   * load counter
//          MOVE 0, R2    * clear total
//     $LOOP
//          ADD R0, R2, R2
//          DECR R0
//          JNZERO $LOOP
//          MOVE R2, R0   * move final result to R0
//          CALL $OUTPUT
//
//
// N.B. The assembler for the most part ignores punctuation, except as a separator. So:
//      ADD R0, R2, R2
// could legally be written
//      ADD R0 + R2 = R2
// and it would assemble and work just fine. Keep in mind that symbols must be singular, so
//      MOVE R0 -> R1  * don't do this!
// will NOT work, but
//      MOVE R0 > R1
// is just fine and still indicates direction.
//
// Basic scheme for symbol table handling is a linked list of linked lists.
// First list contains the name of each symbol and (when known) its location, along with the head
// pointer to a list of locations where the symbol is used.
//
// During processing, we add symbols to the lists and track the input line and location in the object
// file. Then we replace the symbol with 0 before handing the input line to build_instruction().
//
// After we have processed the entire input file, we traverse the list of symbols, looking for any
// that do not have a location defined - this is a fatal error.
//
// If all symbols have been defined, then we walk the table and fix up all the spots in the object
// file with the correct value, before finally writing the output to disk.
//


// Extract a symbol from the string, beginning at the specified position.
// Returns a string, possibly empty.
std::string ExtractSymbol(std::string Line, unsigned int Pos)
{
    std::string retval;

    while (isalpha(Line[Pos]))
        retval += Line[Pos++];

    return retval;
}

// Display usage. Caller passes in argv[0] so that we can display the name of the command.
int Usage(char *Cmd)
{
    std::cout << "USAGE:\n\t";
    std::cout << Cmd << " infile outfile\n";
    std::cout << "No other options are currently supported\n\n";
    return 0;
}

// Fill a byte array from the given word in MSB order for binary output.
void FillMSBFirst(uint8_t *Buffer, uint32_t Word)
{
    for (auto i = 24, j = 0; i >= 0; i -= 8, j++) // ugly magic numbers
        Buffer[j] = (uint8_t)((Word >> i) & 0xff);
}

// Write a blob of 32-bit words to the given output file in MSB-first order. Not particularly efficient,
// but we're not dealing with huge amounts of data here.
void WriteBlob(std::vector<uint32_t>& Blob, std::ofstream &File)
{
    uint8_t outbuf[4];

    for (auto i : Blob) {
        FillMSBFirst(outbuf, i);
        File.write((const char *)outbuf, 4);
    }
}

// Main function of the assembler. Most of the work is done in the BuildInstruction() function, and in
// the SymbolTable class.
int main(int argc, char *argv[])
{
    std::ifstream infile;
    std::vector<uint32_t> outbuf;
    std::ofstream outfile;
    std::string in_line;
    unsigned int linenum {0};
    uint32_t addr {0};
    SymbolTable syms;

    if (argc != 3)
        return Usage(argv[0]);

    try {
        infile.open(argv[1], std::ios::in);
    } catch (const char* msg) {
        std::cout << "Error opening input file: " << msg << "\n";
        return -1;
    }

    try {
        outfile.open(argv[2], std::ios::binary | std::ios::out | std::ios::trunc);
    } catch (const char* msg) {
        std::cout << "Error opening output file: " << msg << "\n";
        infile.close();
        return -1;
    }

    // The actual loop where things get done.
    while (!infile.eof()) {
        uint32_t word, extra_word;
        bool extra_present;
        bool ret {false};
        size_t pos;

        getline(infile, in_line);
        linenum++;
        // skip comment and blank lines
        if ((in_line[0] == '*') || (in_line[0] == '#') || (in_line.length() == 0))
            continue;

        // check for symbol on a line
        pos = in_line.find('$');
        if (pos != std::string::npos) {
            std::string tmpsym = ExtractSymbol(in_line, pos + 1);

            if (pos > 0) { // symbol is a reference
                ret = syms.AddRef(tmpsym, addr+1, linenum);
                // for direct value instructions, the value comes after the instruction
                // fix up source line so parser inserts a zero
                if (!ret) {
                    in_line.erase(pos, tmpsym.length() + 1);
                    in_line.insert(pos, "0 ");
                }
            } else { // Symbol starts at position 0, it's a declaration.
                ret = syms.AddSymbol(tmpsym, addr, linenum);
                if (!ret)
                    continue; // no further processing on this line
            }
        }
        if (ret) { // error happened!
            std::cerr << "Fatal: invalid symbol on line " << linenum << ". Stopping.\n";
            infile.close();
            outfile.close();
            remove(argv[2]);
            exit(1);
        }
        // process line as an instruction
        try {
            word = BuildInstruction(in_line, extra_word, extra_present);
        } catch (const char* msg) {
            std::cerr << "Fatal: parse error, line " << linenum << ": " << msg << "\n";
            infile.close();
            outfile.close();
            remove(argv[2]);
            exit(1);
        }
        // Add the resultant word(s) to the output blob.
        outbuf.push_back(word);
        addr++;
        if (extra_present) {
            outbuf.push_back(extra_word);
            addr++;
        }
    }

    // Fix up all of the symbols and make sure each reference actually points somewhere valid.
    syms.UpdateExecutable(outbuf);
    WriteBlob(outbuf, outfile);
    infile.close();
    outfile.close();
    return 0;
}
