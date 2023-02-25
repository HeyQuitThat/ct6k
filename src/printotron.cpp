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

// printotron.cpp - definitions for the Print-o-Tron XL emulator
#include "printotron.hpp"
#include "hw.h"

// Get the required IO memory size. Used during registration to make sure we have enough space.
// (In real life, this is ignored by the CPU which always gives 64k words.)
uint32_t PrintOTron::GetMemSize()
{
    return POT_MEM_SIZE;
}

// Device class is used the UI to know how to handle the device
DeviceClass PrintOTron::GetDeviceClass()
{
    return DC_PRINTER;
}

// Digital Device Name - used by apps to find the printer device
uint32_t PrintOTron::GetDDN()
{
    return POT_DDN;
}

PrintOTron::PrintOTron()
{
    OutputBuffer.clear();
    Status = POT_STATUS_NO_PAPER; // Will change to ready when UI initializes.
}

// Defines how the device responds to memory writes by applications.
// Behavior described in hw.h
void PrintOTron::WriteIOMem(uint32_t Offset, uint32_t Value)
{
    switch (Offset) {
        case POT_REG_STATUS:
            // Read-only register
            break;
        case POT_REG_OUTPUT:
            OutputBuffer += (char)(Value & 0xff);
            break;
        case POT_REG_CONTROL:
            if (Value & POT_CONTROL_LINE_RELEASE) {
                Status = POT_STATUS_BUSY;
                LineRelease = true;
            }
            if (Value & POT_CONTROL_PAGE_RELEASE) {
                Status = POT_STATUS_BUSY;
                OutputBuffer = "\f";
                LineRelease = true;
            }
            break;
        default:
        // invalid, ignored
            break;
    }
}

// Defines how the device responds to memory reads by applications.
uint32_t PrintOTron::ReadIOMem(uint32_t Offset)
{
    if (Offset != 0)
        return 0xFFFFFFFF;
    return Status;
}

// Called by the main loop to determine when output is ready to be put on-screen.
bool PrintOTron::IsOutputReady()
{
    if (Status == POT_STATUS_NO_PAPER) {
        Status = POT_STATUS_OK;
    }
    return LineRelease;

}

// Called by the main loop to actually get output. Clears the buffer to wait for
// the next line of text.
std::string PrintOTron::GetOutputLine()
{
    std::string retval;

    if (!LineRelease)
        return "";

    retval = OutputBuffer;
    LineRelease = false;
    Status = POT_STATUS_OK;
    OutputBuffer.clear();
    return retval;
}

// Reset the device as though a power cycle had happened.
void PrintOTron::PowerOnReset()
{
    OutputBuffer.clear();
    Status = POT_STATUS_NO_PAPER; // Will change to ready when UI initializes.
}
