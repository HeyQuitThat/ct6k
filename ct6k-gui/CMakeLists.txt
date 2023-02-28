cmake_minimum_required(VERSION 3.5)

project(ct6k-gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.hpp
        mainwindow.ui
        indicator.cpp indicator.hpp
        dword.cpp dword.hpp
        controlpanel.cpp controlpanel.hpp
        registerset.cpp registerset.hpp
        ../src/instruction.cpp ../src/instruction.hpp
        ../src/cpu.cpp ../src/cpu.hpp ../src/hw.h ../src/arch.h
        ../src/memory.cpp ../src/memory.hpp
        ../src/periph.cpp ../src/periph.hpp
        ../src/printotron.cpp ../src/printotron.hpp
        ../src/cardotron.cpp ../src/cardotron.hpp
        flagdisplay.cpp flagdisplay.hpp
        cpuworker.cpp cpuworker.hpp
        cpuspinner.cpp cpuspinner.hpp
        printerwindow.hpp printerwindow.cpp
        cotwindow.hpp cotwindow.cpp
        ct6k-info.h
        ct6k.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ct6k-gui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ct6k-gui APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ct6k-gui SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ct6k-gui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ct6k-gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(ct6k-gui PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ct6k-gui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ct6k-gui)
endif()
