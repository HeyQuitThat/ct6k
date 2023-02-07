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

// cpuspinner.hpp - declarations for the CPUSpinner class
#ifndef CPUSPINNER_H
#define CPUSPINNER_H
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include "../src/cpu.hpp"
#include "../src/periph.hpp"

// Run state of CPU Spinner thread. Set by buttons on the UI, checked by the spinner thread
// each time through the loop.
enum CPURunState {
    CR_STOPPED,
    CR_STEP,
    CR_1HZ,
    CR_10HZ,
    CR_60HZ,
    CR_FULL,
    CR_HALTED,
    CR_EXITING, // used internally
};


// CPUSpinner class, runs in a separate thread from the rest of the application. Controls
// the (emulated) CPU at various speeds and reports (via QT signal) to the UI.
class CPUSpinner : public QThread
{
    Q_OBJECT
public:
    CPUSpinner(QObject *parent = nullptr, CPU *CT6K = nullptr, PrintOTron *POT = nullptr);
    ~CPUSpinner();
    void ChangeState(CPURunState NewState);

signals:
    void UpdatePanel(CPUInternalState *state);
    void UpdatePrinterWindow(QString OutLine);

protected:
    void run() override;


private:
    QMutex Mutex;
    CPURunState RunState;
    QWaitCondition WaitCondition;
    CPUInternalState CurrentState;
    CPU *MyCPU;
    PrintOTron *MyPOT;
    void RunThenWait(int msec);

};

#endif // CPUSPINNER_H
