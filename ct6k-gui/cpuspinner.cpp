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

// cpuspinner.cpp - function definitions for the CPUSpinner class.

#include "cpuspinner.hpp"
#define MSEC1HZ 1000
#define MSEC10HZ 100
#define MSEC60HZ 17     // Yes, it's a hair slow. I know. So was the real Comp-o-Tron 6000.
#define FAST_RUN_CYCLES 10001 // Number of cycles to run before we check for events.
                              // On my system this takes ~1.3msec to execute, giving
                              // a clock speed of about 770kHz. Not bad for 1956!
                              // Raising the value did not materially affect speeds,
                              // so we will leave it at this value for good responsiveness.
                              // Some tweaking in the CPU and/or Instruction classes would
                              // likely help speed this up, if it becomes necessary.
                              // Use an odd number, an even number may make it appear that
                              // bit 2 of the PC is stuck.

// Constructor. This does not start the thread; this is done by the owner
// of this object when it calls inherited function start().
CPUSpinner::CPUSpinner(QObject *parent, CPU *CT6K, PrintOTron *POT)
{
    setParent(parent);
    MyCPU = CT6K;
    MyPOT = POT;
    RunState = CR_STOPPED;

}

// Destructor. Kills the thread and waits for it to terminate.
CPUSpinner::~CPUSpinner()
{
    RunState = CR_EXITING;
    Mutex.lock();
    WaitCondition.wakeOne();
    Mutex.unlock();

    wait();

}

// Pass the new run state through the mutex to the thread executing
// run(). If the thread is stopped, kick it.
void CPUSpinner::ChangeState(CPURunState NewState)
{
    Mutex.lock();
    CPURunState OldState = RunState;
    if ((RunState == CR_HALTED) || (RunState == CR_EXITING)) {
        // Don't execute, we are done.
        Mutex.unlock();
        return;
    }
    RunState = NewState;
    // Kick the run thread if it's stopped. If it's running,
    // then the new state will kick in at the top of the cycle.
    if (OldState == CR_STOPPED)
        WaitCondition.wakeOne(); // Should be only one, right?
    Mutex.unlock();
}

// The meat of the class - this spins in a separate thread, controlling
// the CPU and sending signals to the UI when and update is required.
void CPUSpinner::run()
{
    forever {
        // Cache the run state locally, under mutex. The "real" RunState can
        // be updated at any time, which could cause Problems.
        Mutex.lock();
        CPURunState LocalState = RunState;
        Mutex.unlock();

        // Actually execute using the Step() method.
        switch (LocalState) {
            case CR_STEP:
                MyCPU->Step();
                break;
            case CR_1HZ:
                RunThenWait(MSEC1HZ);
                break;
            case CR_10HZ:
                RunThenWait(MSEC10HZ);
                break;
            case CR_60HZ:
                RunThenWait(MSEC60HZ);
                break;
            case CR_FULL:
                for (int i = 0; i < FAST_RUN_CYCLES; i++)
                    MyCPU->Step();
                break;
            case CR_HALTED:
            case CR_STOPPED:
            case CR_EXITING:
                // don't do anything
                break;
        }
        // Do I need a mutex here? Only one producer and one consumer,
        // but it's certianly not an atomic update. No problems seen
        // without one so far...
        if (LocalState != CR_EXITING) {
            CurrentState = MyCPU->DumpInternalState();
            emit UpdatePanel(&CurrentState);
        }
        // Check to see if the printer has any output
        if ((MyPOT != nullptr) && MyPOT->IsOutputReady())
            emit UpdatePrinterWindow(MyPOT->GetOutputLine().c_str());

        // Update run state if needed, check if we are done.
        Mutex.lock();
        if (RunState == CR_EXITING) {
            Mutex.unlock();
            return;
        }
        if (MyCPU->IsHalted())
            RunState = CR_HALTED; // This overrides all other running states
        // Logically, this should be in the switch above but we need this
        // protected by the mutex. So here it is.
        if (RunState == CR_STEP)
            RunState = CR_STOPPED;
        // Wait on HALTED as well as STOPPED becaue we still need to wake
        // the thread in order to exit.
        if ((RunState == CR_STOPPED) || (RunState == CR_HALTED))
            WaitCondition.wait(&Mutex);
        Mutex.unlock();
    } // forever
}

// Helper function for the slow speeds. Execute one step of the CPU then
// sleep for the specified number of milliseconds.
void CPUSpinner::RunThenWait(int msec)
{
    MyCPU->Step();
    if (!MyCPU->IsHalted())
        msleep(msec);
}

