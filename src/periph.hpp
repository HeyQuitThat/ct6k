// periph.cpp - class declaration for peripherals.
// Defines a basic interface for Comp-o-Tron 6000 I/O devices. Individual devices can instantiate
// this class and register with the CPU.
// The peripheral class doesn't need to know its own memory base or interrupt ID - the CPU
// class can take care of this. All the class need to know is if registers have been read or
// written. The CPU polls to see if an interrupt has happened.
// The UI instantiates these and calls into them to handle device-specific IO.

#include <string>
#include <cstdint>

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

    // Interface on UI side varies based on device, so the derived classes will add those functions.
private:
};

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
private:
    std::string OutputBuffer;
    bool LineRelease {false};
    uint32_t Status;
};

