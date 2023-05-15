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

// segment.hpp - declarations for the CodeSegment class and other dependent classes
// This is the core object that holds the object code produced by the assembler
#ifndef __SEGMENT_HPP__
#define __SEGMENT_HPP__
#include <cstdint>
#include <string>
#include <vector>


class CodeSegment {
public:
    void SetBase(uint32_t Base);
    void AddWord(uint32_t NewWord);
    uint32_t ReadWord(uint32_t Offset);
    void ModifyWord(uint32_t Offset, uint32_t NewValue);
    uint32_t GetBase();
    std::size_t GetLen();
    std::string GetFilename() const;
    void SetFilename(const std::string &newFilename);

private:
    uint32_t BaseAddr;
    std::vector<uint32_t> Binary;
    std::string Filename;
};

#endif // __SEGMENT_HPP__
