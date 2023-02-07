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

// mainwindow.hpp - declarations for the MainWindow class
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFont>
#include "cpuworker.hpp"
#include "controlpanel.hpp"
#include "printerwindow.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// MainWindow class. Primary control point for the UI of the application.
// Owns all of the display elements and the menu items, as well as the
// CPU worker object.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    CPUWorker *Worker;
    ControlPanel *CP;
    PrinterWindow *PW;

private slots:
    void on_actionExit_triggered();

    void on_actionLoad_Binary_triggered();

    void on_actionReset_triggered();

    void on_actionShow_FHAP_triggered();

    void on_actionShow_IHAP_triggered();

    void on_actionDump_Memory_triggered();

    void on_actionDisassemble_triggered();

    void on_actionModify_Registers_triggered(bool checked);

    void on_actionModify_Memory_triggered();

    void on_actionInstructions_triggered();

    void on_actionAbout_triggered();

    void on_actionCaution_triggered();

    void on_actionPrint_O_Tron_XL_triggered(bool checked);

private:
    Ui::MainWindow *ui;
    QFont *CabinB;
    QPixmap *OffImg;
    QPixmap *OnImg;
    QPixmap *TriImg;
    bool RegistersLocked;
    uint32_t GetValFromUser(QString Title, QString Message, QString Seed, bool *Success);

};
#endif // MAINWINDOW_H
