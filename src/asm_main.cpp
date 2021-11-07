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
//  - symbolic address references may only be used by control-flow instructions (JMPs or CALL)
//    and must begin with $.
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

// TODO handle string data
// TODO handle .ORG statement for relocatable files.

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
#if 0
void test_hexval(uint32_t val)
{
    std::string outstr;
    auto i = new Instruction(val);
    i->print(outstr);
    std::cout << outstr;
    delete i;
}

void test_2hexval(uint32_t val, uint32_t val2)
{
    std::string outstr;
    auto i = new Instruction(val, val2);
    i->print(outstr);
    std::cout << outstr;
    delete i;
}
#endif



std::string ExtractSymbol(std::string Line, unsigned int Pos)
{
    std::string RetVal;
    
    while (isalpha(Line[Pos]))
        RetVal += Line[Pos];

    return RetVal;
}

int usage(char *cmd)
{
    std::cout << "USAGE:\n\t";
    std::cout << cmd << " infile outfile\n";
    std::cout << "No other options are currently supported\n\n";
    return 0;
}

void fill_buf_msb_first(uint8_t *buf, uint32_t dword)
{
    for (auto i = 24, j = 0; i >= 0; i -= 8, j++) // ugly magic numbers
        buf[j] = (uint8_t)((dword >> i) & 0xff);
}

void WriteBlob(std::vector<uint32_t>& Blob, std::ofstream &File)
{
    uint8_t outbuf[4];

    for (auto i : Blob) {
        fill_buf_msb_first(outbuf, i);
        File.write((const char *)outbuf, 4);
    }
}

int main(int argc, char *argv[0])
{
    int retval {0};
    std::ifstream InFile;
    std::vector<uint32_t> OutBuf;
    std::ofstream OutFile;
    std::string InLine;
    unsigned int LineNum {0};
    uint32_t addr {0};
    SymbolTable syms;

    if (argc != 3)
        return usage(argv[0]);

    try {
        InFile.open(argv[1], std::ios::in);
    } catch (const char* msg) {
        std::cout << "Error opening input file: " << msg << "\n";
        return -1;
    }

    try {
        OutFile.open(argv[2], std::ios::binary | std::ios::out | std::ios::trunc);
    } catch (const char* msg) {
        std::cout << "Error opening output file: " << msg << "\n";
        InFile.close();
        return -1;
    }

    while (!InFile.eof()) {
        uint32_t word, extra_word;
        bool extra_present;
        bool ret;
        size_t Pos;
        
        getline(InFile, InLine);
        LineNum++;
        std::cout << "Line " << LineNum << "\n";
        // skip comment and blank lines
        if (InLine[0] == '*' || (InLine.length() == 0)) {
            std::cout << "Skipping comment\n";
            continue;
        }
        // check for symbol on a line
        Pos = InLine.find('$');
        if (Pos != std::string::npos) {
            std::string TmpSymbol = ExtractSymbol(InLine, Pos);
            std::cout << "Extracted symbol " << TmpSymbol << " at position " << Pos << "\n";
            
            if (Pos > 0) {
                ret = syms.AddRef(TmpSymbol, addr+1, LineNum);
                // for direct value instructions, the value comes after the instruction
                // fix up source line so parser inserts a zero
                if (!ret) {
                    InLine.erase(Pos);
                    InLine += "0\n";
                    std::cout << "Fixed up line to read \"" << InLine << "\"\n"; 
                }
            } else {
                ret = syms.AddSymbol(TmpSymbol, addr, LineNum); // TODO handle ret or exception
                if (!ret)
                    continue; // no further processing on this line
            }
        }
        if (ret) { // error happened!
            std::cout << "Error found, stopping\n";
            InFile.close();
            OutFile.close();
            remove(argv[2]);
            exit(1);
        }
        // process line as an instruction
        try {
            word = build_instruction(InLine, extra_word, extra_present);
        } catch (const char* msg) {
            std::cerr << "Fatal: parse error, line " << InLine << ": " << msg << "\n";
            InFile.close();
            OutFile.close();
            remove(argv[2]);
            exit(1);
        }
        std::cout << "Encoded instruction as " << std::hex << word << "\n";
        OutBuf.push_back(word);
        addr++;
        if (extra_present) {
            std::cout << "Adding extra word " << std::hex << extra_word << "\n";
            OutBuf.push_back(extra_word);
            addr++;
        }
        
    }

    syms.UpdateExecutable(OutBuf);
    WriteBlob(OutBuf, OutFile);
    InFile.close();
    OutFile.close();
    return retval;
}
