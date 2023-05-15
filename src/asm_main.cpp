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
#include <string_view>
#include <boost/format.hpp>
#include <vector>
#include "symref.hpp"
#include "segment.hpp"
#include "instruction.hpp"

#define ASM_VER_MAJOR "2"
#define ASM_VER_MINOR "0"
#define ASM_VER_SUB "1"
#define ASM_VER_STRING "Comp-o-Tron 6000 symbolic assembler, version " ASM_VER_MAJOR "." ASM_VER_MINOR "." ASM_VER_SUB

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

// Version 2 additions:
// - Output to Card-o-Tron card file instead of straight binary
// - Assemble multiple files
// - Multiple code segments (C-O-T output only)
// - .ADDR directive to define new code segment
// - .TXTN .TXTM .TXTL for text (not packed, packed MSB first, packed LSB first)
// - .VALUE to define a fixed value
//


// Remove whitespace from beginning of string
void StripBlank(std::string &Line)
{
    unsigned int pos {0};
    while (isspace(Line[pos]))
        pos++;
    Line.erase(0,pos);

}

// Extract a symbol from the string, beginning at the specified position.
// Returns a string, possibly empty.
std::string ExtractToken(std::string Line, unsigned int Pos)
{
    std::string retval;

    while (isalpha(Line[Pos]))
        retval += Line[Pos++];

    return retval;
}

// Extract a line of text from the string, beginning at the first non-whitespace
// character and terminated with the same character or EOL
std::string ExtractText(std::string Line)
{
    std::string retval;
    unsigned int pos {0};
    char marker;

    while (isspace(Line[pos]))
        pos++;
    marker = Line[pos];
    pos++;
    while ((Line[pos] != marker) && (pos < Line.length())) {
        retval += Line[pos];
        pos++;
    }
    return retval;
}

// Display usage. Caller passes in argv[0] so that we can display the name of the command.
int Usage(char *Cmd)
{
    std::cout << "Usage: " << Cmd << "[options] file ...\n";
    std::cout << "Options:\n";
    std::cout << "\t-o outfile\n";
    std::cout << "\t-B raw binary output format\n";
    std::cout << "\t-C Card-O-Tron card deck output format\n";
    std::cout << "\t-l produce complete listing\n";
    std::cout << "\nRaw binary files can be loaded directly by the emulator but must start at\n";
    std::cout << "address 0. Generally, this is for short programs that can run on the raw\n";
    std::cout << "hardware. For more flexiblity, or to assemble multiple files in multiple\n";
    std::cout << "code segments, assemble to Card-O-Tron binary format and load them using\n";
    std::cout << "the Comp-O-Tron 6000 Self Rising feature.\n";

    return 0;
}

// Fill a byte array from the given word in MSB order for binary output.
void FillMSBFirst(uint8_t *Buffer, uint32_t Word)
{
    for (auto i = 24, j = 0; i >= 0; i -= 8, j++) // ugly magic numbers
        Buffer[j] = (uint8_t)((Word >> i) & 0xff);
}

// Write a blob of 32-bit words to the given output file in MSB-first order. Not particularly efficient,
// but we're not dealing with huge amounts of data here. Multiple segments are allowed but they must
// be contiguous. Warn but don't fail if starting address is not 0x0.
// Returns true if error.
bool WriteBin(std::vector<CodeSegment *>Segs, std::ofstream &File)
{
    uint8_t outbuf[4];
    uint32_t addr {0};

    for (auto s : Segs) {
        if (s->GetBase() != addr) {
            if (addr == 0) {
                std::cerr << "Warning! Binary file begins at nonzero address " <<
                             std::hex << s->GetBase() << "\n";
                // carry on; it's their problem
            } else {
                std::cerr << "Fatal error - Binary file with non-contiguous segments\n";
                return true;
            }
        }
        for (std::size_t i = 0; i < s->GetLen(); i++) {
            FillMSBFirst(outbuf, s->ReadWord(i));
            File.write((const char *)outbuf, 4);
        }
        addr = s->GetBase() + s->GetLen();
    }
    return false;
}

// Print the instruction based upon the value(s) given. If Count is specified, update the count of
// words used for the instruction.
std::string FormatDisasm(uint32_t Val, uint32_t Val2, uint32_t *Count)
{
    std::string outstr;
    auto i = new Instruction(Val, Val2);
    i->Print(outstr);
    if (Count != nullptr)
        *Count = i->SizeInMemory();
    delete i;
    return outstr;
}

// Dump a listing of the completed program to the given file. This is done after we write
// the binary, so no error checking needs to be done.
void DumpListing(std::vector<CodeSegment *>Segs,  std::ofstream &File)
{
    int segnum {0};
    File << ASM_VER_STRING << "\n";
    for (auto s : Segs) {
        File << "\nSEGMENT # " << segnum << " FROM FILE " << s->GetFilename() << "\n\n";
        for (std::size_t i = 0; i < s->GetLen(); i++) {
            uint32_t word1 = s->ReadWord(i);
            uint32_t word2 = s->ReadWord(i + 1); // OK to read off the end, we'll just get 0.
            uint32_t used {0};
            std::string instr = FormatDisasm(word1, word2, &used);
            File << (boost::format("0x%08X : ") % (i + s->GetBase())).str();
            File << (boost::format("0x%08X ") % word1).str();
            if (used == 2) {
                File << (boost::format("0x%08X ") % word2).str();
                i++;
            } else {
                File << "           ";
            }
            File << instr << "\n";
        }
        segnum++;
        File << "\n";
    }
    File << " --- END OF LISTING ---\n\n";
}


#define COT_BIN_CARD_LEN 31
// Write a single Card-o-Tron card in the file format expected by the COT emulator.
void PunchCard(std::ofstream &File, uint32_t Address, uint32_t Count, uint32_t *Data)
{
    File << "<C> " << std::dec << Count << "\n";
    File << std::hex;
    File << Address << '\n';
    for (unsigned int i = 0; i < Count; i++) {
        File << *(Data + i);
        if ((i > 0) && ((i % 8) == 0))
            File << '\n';
        else
            File << ' ';
    }
    File << '\n';

}
// Write an assembled program out to a stack of Card-o-Tron cards in executable format.
// Multiple non-contiguous segments allowed.
void WriteCOT(std::vector<CodeSegment *>Segs, std::ofstream &File)
{
    uint32_t outbuf[COT_BIN_CARD_LEN];

    File << std::showbase;
    for (auto s : Segs) {
        uint32_t base = s->GetBase();
        for (std::size_t i = 0; i < s->GetLen(); i+= COT_BIN_CARD_LEN) {
            uint32_t len = s->GetLen() - i;
            if (len > COT_BIN_CARD_LEN)
                len = COT_BIN_CARD_LEN;
            for (std::size_t j = 0; j < len ; j++)
                outbuf[j] = s->ReadWord(i + j);
            PunchCard(File, base + i, len, outbuf);
        }
    }
}

// Pack a dword from a string, Least Significant Byte first
uint32_t PackLSB(std::string &Text)
{
    uint32_t retval {0};
    // Yes, I could be super clever and make a loop, but it's only four bytes
    if (Text.length()) {
        retval |= Text[0];
        Text.erase(0,1);
    }
    if (Text.length()) {
        retval |= Text[0] << 8;
        Text.erase(0,1);
    }
    if (Text.length()) {
        retval |= Text[0] << 16;
        Text.erase(0,1);
    }
    if (Text.length()) {
        retval |= Text[0] << 24;
        Text.erase(0,1);
    }

    return retval;
}

// Pack a dword from a string, Most Significant Byte first
uint32_t PackMSB(std::string &Text)
{
    uint32_t retval {0};
    // Yes, I could be super clever and make a loop, but it's only four bytes
    // Yes, I cut and pasted this code (and comment)
    if (Text.length()) {
        retval |= Text[0] << 24;
        Text.erase(0,1);
    }
    if (Text.length()) {
        retval |= Text[0] << 16;
        Text.erase(0,1);
    }
    if (Text.length()) {
        retval |= Text[0] << 8;
        Text.erase(0,1);
    }
    if (Text.length()) {
        retval |= Text[0];
        Text.erase(0,1);
    }

    return retval;

}

// Assemble from the provided (open) file, into the given segment. Assembly stops when EOF
// is reached, or when a .ADDR directive is reached.
// Retuns true if an error occurred. The caller will check EOF on the file.
bool AssembleSegment(std::ifstream *InFile, CodeSegment *CurrentSeg, SymbolTable *Symbols, int *CurrentLine)
{
    uint32_t word, extra_word;
    std::string in_line;
    bool extra_present;
    bool ret {false};
    size_t pos;

    // The actual loop where things get done.
    while (!InFile->eof()) {
        auto oldpos = InFile->tellg();
        getline(*InFile, in_line);
        (*CurrentLine)++;
        // skip comment and blank lines
        if ((in_line[0] == '*') || (in_line[0] == '#') || (in_line.length() == 0))
            continue;

        // Check for directive on a line.
        // Possibilities are:
        //      .ADDR to set the address of the next instruction
        //      .VALUE to set a numeric value
        if (in_line[0] == '.') {
            std::string directive = ExtractToken(in_line, 1);
            if (directive == "ADDR") {
                in_line.erase(0, directive.length() + 1);
                uint32_t tmpval;
                try {
                    tmpval = std::stoul(in_line, nullptr, 0);
                } catch(std::invalid_argument const &ex) {
                    std::cerr << "Fatal: invalid .ADDR on line " << *CurrentLine << "\n";
                    std::cerr << "in_line is: " << in_line << "\n";
                    return true;
                    // TODO how to get filename? Pass it in? Or expect caller to print it?
                } catch(std::out_of_range const &ex) {
                    std::cerr << "Fatal: .ADDR out of range on line " << *CurrentLine << "\n";
                    return true;
                }
                if (CurrentSeg->GetLen() == 0) {
                    CurrentSeg->SetBase(tmpval);
                    continue;
                } else {
                    InFile->seekg(oldpos);
                    return false;
                }
            }
            if (directive == "VALUE") {
                in_line.erase(0, directive.length() + 1);
                StripBlank(in_line);
                std::string tmpsym = ExtractToken(in_line, 0);
                in_line.erase(0, tmpsym.length());
                uint32_t tmpval;
                try {
                    tmpval = std::stoul(in_line, nullptr, 0);
                } catch(std::invalid_argument const &ex) {
                    std::cerr << "Fatal: invalid .VALUE on line " << *CurrentLine << "\n";
                    std::cerr << "tmpsym = " << tmpsym << "\n";
                    std::cerr << "in_line = " << in_line << "\n";
                    return true;
                    // TODO how to get filename? Pass it in? Or expect caller to print it?
                } catch(std::out_of_range const &ex) {
                    std::cerr << "Fatal: .VALUE out of range on line " << *CurrentLine << "\n";
                    return true;
                }
                Symbols->AddValue(tmpsym, tmpval, *CurrentLine, CurrentSeg);
                continue;
            }
            if (directive.substr(0,3) == "TXT") {
                in_line.erase(0, directive.length() + 1);
                std::string Data = ExtractText(in_line);
                if (Data.length() == 0) {
                    std::cerr << "Fatal: encoding zero-length text " << *CurrentLine << "\n";
                    return true;
                }
                switch (directive[3]) {
                case 'N': // no encoding, one char per word (wasteful)
                    for (auto c : Data) {
                        uint32_t word = (uint32_t)c;
                        CurrentSeg->AddWord(word);
                    }
                    break;
                case 'L': // 4 chars per word, LSB first
                    while (Data.length())
                        CurrentSeg->AddWord(PackLSB(Data));
                    break;
                case 'M': // 4 chars per word, MSB first
                    while (Data.length())
                        CurrentSeg->AddWord(PackMSB(Data));
                    break;
                default:
                    std::cerr << "Fatal: invalid directive " << directive << " on line " << *CurrentLine << "\n";
                    return true;
                    break;
                }
                continue;

            }
            // invalid directive
            std::cerr << "Fatal: invalid directive " << directive << " on line " << *CurrentLine << "\n";
            return true;
        }
        // check for symbol on a line
        pos = in_line.find('$');
        if (pos != std::string::npos) {
            std::string tmpsym = ExtractToken(in_line, pos + 1);

            if (pos > 0) { // symbol is a reference
                ret = Symbols->AddRef(tmpsym, CurrentSeg->GetLen() + 1,
                                      *CurrentLine, CurrentSeg);
                // for direct value instructions, the value comes after the instruction
                // fix up source line so parser inserts a zero
                if (!ret) {
                    in_line.erase(pos, tmpsym.length() + 1);
                    in_line.insert(pos, "0 ");
                }
            } else { // Symbol starts at position 0, it's a declaration.
                ret = Symbols->AddSymbol(tmpsym, CurrentSeg->GetLen(),
                                         *CurrentLine, CurrentSeg);
                if (!ret)
                    continue; // no further processing on this line
            }
        }
        if (ret) { // error happened!
            std::cerr << "Fatal: invalid symbol on line " << *CurrentLine << ". Stopping.\n";
            return true;
        }
        // process line as an instruction
        try {
            word = BuildInstruction(in_line, extra_word, extra_present);
        } catch (const char* msg) {
            std::cerr << "Fatal: parse error, line " << *CurrentLine << ": " << msg << "\n";
            return true;
        }
        // Add the resultant word(s) to the segment
        CurrentSeg->AddWord(word);
        if (extra_present) {
            CurrentSeg->AddWord(extra_word);
        }
    }
    return false;
}

// Main function of the assembler. Most of the work is done in the BuildInstruction() function, and in
// the SymbolTable class.
int main(int argc, char *argv[])
{
    std::string outname;
    std::ofstream outfile;
    std::string in_line;
    SymbolTable syms;
    std::vector<std::string> innames;
    CodeSegment *CurrentSeg;
    std::vector<CodeSegment *>Segs;
    bool OutputBin {true};
    bool GotOutputType {false};
    bool ProduceListing {false};

    std::cout << ASM_VER_STRING << "\n";
    // Minimum number of args is 4, so don't bother checking them if we don't have
    // that many
    if (argc < 4)
        return Usage(argv[0]);

    for (auto i = 1; i < argc; i++) {
        std::string TmpArg = argv[i];
        if (TmpArg == "-b") {
            OutputBin = true;
            GotOutputType = true;
            continue;
        }
        if (TmpArg == "-c") {
            OutputBin = false;
            GotOutputType = true;
            continue;
        }
        if (TmpArg == "-o") {
            i++;
            if (i >= argc)
               return Usage(argv[0]);
            outname = argv[i];
            continue;
        }
        if (TmpArg == "-l") {
            ProduceListing = true;
            continue;
        }
        innames.push_back(TmpArg);
    }

    if (innames.empty()) {
        std::cerr << "No input files found. Exiting.\n";
        return -1;
    }

    if (outname.empty()) {
        std::cerr << "No output filename specified. Exiting.\n";
        return -1;
    }

    if (!GotOutputType) {
        std::cerr << "No output format specified. Exiting.\n";
        return -1;
    }
    // make sure we can open the output file first
    try {
        auto mode = std::ios::out | std::ios::trunc;
        if (OutputBin)
            mode |= std::ios::binary;
        outfile.open(outname, mode);
    } catch (const char* msg) {
        std::cerr << "Error opening output file: " << msg << "\n";
        return -1;
    }


    uint32_t NextAddr {0};
    for (auto& it : innames) {
        std::string CurrInFileName = it;
        std::ifstream InFile;
        int LineNum = 0;

        try {
            InFile.open(CurrInFileName, std::ios::in);
        } catch (const char* msg) {
            std::cerr << "Error opening input file " << CurrInFileName << ": " << msg << "\n";
            outfile.close();
            remove(outname.c_str());
            for (auto s = Segs.begin(); s != Segs.end(); s++)
               delete *s;
            return -1;
        }
        while (!InFile.eof()) {
            CurrentSeg = new CodeSegment();
            CurrentSeg->SetBase(NextAddr);
            CurrentSeg->SetFilename(CurrInFileName);
            Segs.push_back(CurrentSeg);
            if (AssembleSegment(&InFile, CurrentSeg, &syms, &LineNum)) {
               std::cerr << "Error assembling input file " << CurrInFileName << "\n";
               InFile.close();
               outfile.close();
               remove(outname.c_str());
               for (auto s = Segs.begin(); s != Segs.end(); s++)
                   delete *s;
               return -1;
            }
            NextAddr = CurrentSeg->GetBase() + CurrentSeg->GetLen();
        }
        InFile.close();
    }
    // All input files have been read and successfully assembled. Now we fix up the
    // symbol tables and check to make sure everything's resolved.
    for (auto s : Segs) {
        if (syms.UpdateSegment(s)) {
            std::cerr << "Error resolving symbol table\n";
            outfile.close();
            remove(outname.c_str());
            for (auto s = Segs.begin(); s != Segs.end(); s++)
               delete *s;
            return -1;
        }
    }
    // Check for overlapping code segments
    for (auto s : Segs) {
        for (auto s1: Segs) {
            if (s == s1)
               continue;
            if (((s->GetBase() > s1->GetBase()) && (s->GetBase() < (s1->GetBase() + s1->GetLen()))) ||
                ((s1->GetBase() > s->GetBase()) && (s1->GetBase() < (s->GetBase() + s->GetLen())))) {
               std::cout << "Fatal error: overlapping code segments!\n";
               outfile.close();
               remove(outname.c_str());
               for (auto s = Segs.begin(); s != Segs.end(); s++)
                   delete *s;
               return -1;
            }

        }
    }


    if (OutputBin) {
        if (WriteBin(Segs, outfile)) {
            outfile.close();
            remove(outname.c_str());
            for (auto s = Segs.begin(); s != Segs.end(); s++)
               delete *s;
            return -1;
        }
    } else {
        WriteCOT(Segs, outfile);
    }
    outfile.close();
    if (ProduceListing) {
        std::ofstream listfile;
        outname += ".listing";
        try {
            listfile.open(outname, std::ios::out | std::ios::trunc);
        } catch (const char* msg) {
            std::cerr << "Error opening output file: " << msg << "\n";
            for (auto s = Segs.begin(); s != Segs.end(); s++)
               delete *s;
            return -1;
        }
        DumpListing(Segs, listfile);
        listfile.close();
    }

    for (auto s = Segs.begin(); s != Segs.end(); s++)
        delete *s;
    return 0;
}
