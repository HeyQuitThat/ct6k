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

// periph.cpp - class declaration for peripherals.
// Defines a basic interface for Comp-o-Tron 6000 I/O devices. Individual devices can instantiate
// this class and register with the CPU.
// The peripheral class doesn't need to know its own memory base or interrupt ID - the CPU
// class can take care of this. All the class need to know is if registers have been read or
// written. The CPU polls to see if an interrupt has happened.
// The UI instantiates these and calls into them to handle device-specific IO.

#include <string>
#include <cstdint>
#ifndef __PERIPH_HPP__
#define __PERIPH_HPP__
enum DeviceClass {
    DC_PRINTER,     // Print-o-Tron XL full-width matrix imager
    DC_TAPE,        // Tape-o-Tron 1200
    DC_CARD_READER, // Card-o-Tron 3CS (reader half)
    DC_CARD_PUNCH,  // Card-o-Tron 3CS (writer half)
    DC_RAS,         // Disc-o-Tron random-access storage
};

// Abstract class, to be instantiated and extended by individual peripherals.
class Periph {
public:
    // Constructor to set mem range? Or default but then getter to find needed size?
    // Interface on CPU side
    virtual ~Periph() {};
    virtual uint32_t GetMemSize() = 0; // should be called before setting base
    virtual void WriteIOMem(uint32_t Offset, uint32_t Value) = 0;
    virtual uint32_t ReadIOMem(uint32_t Offset) = 0;
    virtual DeviceClass GetDeviceClass() = 0;
    virtual uint32_t GetDDN() = 0;
    virtual bool InterruptSupported();
    virtual bool InterruptActive(); // Level triggered, will drop once interrupt has been serviced.
    virtual void DoBackground();
    virtual void PowerOnReset();

    // Interface on UI side varies based on device, so the derived classes will add those functions.
private:
};


#endif  // __PERIPH_HPP__
