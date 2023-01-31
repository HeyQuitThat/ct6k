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

// cpuworker.cpp - function definitions for the CPU Worker class

#include "cpuworker.hpp"
#include "mainwindow.hpp"
#include <QPushButton>
#include <QObject>
#include "../src/cpu.hpp"

// Constructor. This instantiates the CPU as well as the Spinner to run it in
// a separate thread.
CPUWorker::CPUWorker(QObject *parent)
    : QObject{parent}
{
    MainWindow *M = (MainWindow *)parent;
    ControlPanel *P = (ControlPanel *)M->centralWidget();
    CT6K = new CPU(); // default mem size
    Spinner = new CPUSpinner(this, CT6K);
    QObject::connect(Spinner, SIGNAL(UpdatePanel(CPUInternalState*)), P, SLOT(UpdateFromCPU(CPUInternalState*)));
    Spinner->start();
}

// Unlike widgets, we do need a destructor here as the CPU class doesn't respect QT parentage,
// and the Spinner needs to be stopped before the application exits.
CPUWorker::~CPUWorker()
{
    delete Spinner;
    delete CT6K;
}

// The next six functions are wired into the UI and send signals to the spinner.
void CPUWorker::StepOnce()
{
    Spinner->ChangeState(CR_STEP);
}

void CPUWorker::Run1Hz()
{
    Spinner->ChangeState(CR_1HZ);
}

void CPUWorker::Run10Hz()
{
    Spinner->ChangeState(CR_10HZ);
}

void CPUWorker::Run60Hz()
{
    Spinner->ChangeState(CR_60HZ);
}

void CPUWorker::RunFull()
{
    Spinner->ChangeState(CR_FULL);
}

void CPUWorker::Pause()
{
    Spinner->ChangeState(CR_STOPPED);
}

// Resetting the CPU is easy, but we need to kill and reinstantiate
// the spinner.
void CPUWorker::ResetCPU()
{
    Quiesce();
    CT6K->Reset();
    MainWindow *M = (MainWindow *)this->parent();
    ControlPanel *P = (ControlPanel *)M->centralWidget();
    CPUInternalState State = CT6K->DumpInternalState();
    // Directly call the Control Panel update slot
    P->UpdateFromCPU(&State);
    Go();
}


// Program a binary image into the CPU's memory. The UI is in charge
// of actually reading the file, we just need the buffer.
void CPUWorker::ProgramBinary(uint32_t *Buffer, int Len)
{
    ResetCPU();
    // Safe to write mem here because the UI is single thread, so nobody
    // will change the spinner's state. The reset call sets it to STOPPED.
    for (int i = 0; i < Len; i++)
        CT6K->WriteMem(i, Buffer[i]);
}

// Debug helper functions below. These are NOT THREAD SAFE. The caller MUST
// call Quiesce90 first, the Go when done.
uint32_t CPUWorker::ReadMem(uint32_t Address)
{
    if (Spinner != nullptr)
        return 0;
    return CT6K->ReadMem(Address);
}

void CPUWorker::WriteMem(uint32_t Address, uint32_t Value)
{
    CT6K->WriteMem(Address, Value);
}

// Note that ReadReg is not needed, the Control Panel knows the current value.
// This used when the user changes register values via the UI.
void CPUWorker::WriteReg(uint8_t Index, uint32_t Value)
{
    if (Index < NUMREGS)
        CT6K->WriteReg(Index, Value);
}

// Stop the spinner while we access memory or registers. Keep it quiet by
// the simple expedient of killing it. "Dead threads tell no tales."
void CPUWorker::Quiesce()
{
    if (Spinner != nullptr) {
        delete Spinner;
        Spinner = nullptr;
    }
}

// Go() is much easier to type than UnQuiesce(), but less Computer Science-y.
// Note that we must reestablish the signal/slot connection each time we recreate
// the spinner.
void CPUWorker::Go()
{
    if (Spinner == nullptr) {
        Spinner = new CPUSpinner(this, CT6K);
        MainWindow *M = (MainWindow *)this->parent();
        ControlPanel *P = (ControlPanel *)M->centralWidget();
        QObject::connect(Spinner, SIGNAL(UpdatePanel(CPUInternalState*)), P, SLOT(UpdateFromCPU(CPUInternalState*)));
        Spinner->start();
    }
}
