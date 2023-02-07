/*
    The Comp-o-Tron 6000 software is Copyright (C) 2022-2023 Mitch Williams.

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

// cpuworker.hpp - Declaration for the CPUWorker class
#ifndef CPUWORKER_H
#define CPUWORKER_H
#include <QObject>
#include <QThread>
#include <cstdint>
#include "../src/cpu.hpp"
// cpu.hpp includes periph.hpp
#include "cpuspinner.hpp"

// CPUWorker class - control interface to CPU Spinner and the CPU from the UI.
// This is the class that actually instantiates the CPU and owns it, along
// with the peripherals.
class CPUWorker : public QObject
{
    Q_OBJECT
public:
    explicit CPUWorker(QObject *parent = nullptr);
    ~CPUWorker();
    void ProgramBinary(uint32_t *Buffer, int Len);
    uint32_t ReadMem(uint32_t Address);
    void WriteMem(uint32_t Address, uint32_t Value);
    void WriteReg(uint8_t Index, uint32_t Value);
    void Quiesce();
    void Go();

public slots:
    void StepOnce();
    void Run1Hz();
    void Run10Hz();
    void Run60Hz();
    void RunFull();
    void Pause();
    void ResetCPU();

private:
    CPU *CT6K;
    PrintOTron *POT;
    CPUSpinner *Spinner;
};

#endif // CPUWORKER_H
