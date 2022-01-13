// periph.cpp
#include "periph.hpp"
#include "hw.h"

// function definitions for the abstract class Periph - these do nothing but make the compiler happy
void Periph::DoBackground()
{
    return;
}

bool Periph::InterruptSupported()
{
    return false;
}

bool Periph::InterruptActive()
{
    return false;
}

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
                OutputBuffer.clear();
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
    return Status;
}

// Called by the main loop to determine when ouput is ready to be put on-screen.
bool PrintOTron::IsOutputReady()
{
    if (Status == POT_STATUS_NO_PAPER)
        Status = POT_STATUS_OK;
    return LineRelease;

}

// Called by the main loop to actually get output. Clears the buffer to wait for
// the next line of text.
std::string PrintOTron::GetOutputLine()
{
    std::string retval;

    if (!LineRelease)
        return nullptr;

    retval = OutputBuffer;
    LineRelease = false;
    Status = POT_STATUS_OK;
    OutputBuffer.clear();
    return retval;
}
