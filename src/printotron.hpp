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

// printotron.cpp - class declaration for Print-o-Tron XL.

#ifndef __PRINTOTRON_HPP__
#define __PRINTOTRON_HPP__
#include <string>
#include <cstdint>
#include "periph.hpp"

class PrintOTron: public Periph {
public:
    PrintOTron();
    ~PrintOTron() {};
    uint32_t GetMemSize(); // should be called before setting base
    void WriteIOMem(uint32_t Offset, uint32_t Value);
    uint32_t ReadIOMem(uint32_t Offset);
    DeviceClass GetDeviceClass();
    uint32_t GetDDN();
    bool IsOutputReady();
    std::string GetOutputLine();
    void PowerOnReset();
private:
    std::string OutputBuffer;
    bool LineRelease {false};
    uint32_t Status;
};


#endif  // __PRINTOTRON_HPP__
