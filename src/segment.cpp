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
// segment.cpp - function declarations for the CodeSegment object
// A segment is a hunk of object code with a specified lenght and
// start address. Segments may not overlap, and each segment contains
// code from only one source file. (The opposite is not true; a single
// source file may specify more than one segment.
// A program must have a minimum of one segment.

#include "segment.hpp"


void CodeSegment::SetBase(uint32_t Base)
{
    BaseAddr = Base;
}

void CodeSegment::AddWord(uint32_t NewWord)
{
    Binary.push_back(NewWord);
}

uint32_t CodeSegment::ReadWord(uint32_t Offset)
{
    if (Offset >= Binary.size())
        return 0;
    else
        return Binary[Offset];
}

void CodeSegment::ModifyWord(uint32_t Offset, uint32_t NewValue)
{
    if (Offset < Binary.size())
        Binary[Offset] = NewValue;

}

uint32_t CodeSegment::GetBase()
{
    return BaseAddr;
}

std::size_t CodeSegment::GetLen()
{
    return Binary.size();
}

std::string CodeSegment::getFilename() const
{
    return Filename;
}

void CodeSegment::setFilename(const std::string &newFilename)
{
    Filename = newFilename;
}
