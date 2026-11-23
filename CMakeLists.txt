cmake_minimum_required(VERSION 3.16)
project(QtImageEditorV1 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 同時支援 Qt 5.15 與 Qt 6.x；Qt Creator 可直接開啟此專案。
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(QtImageEditorV1
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/imageview.h
    src/imageview.cpp
    src/exportdialog.h
    src/exportdialog.cpp
    resources/icons.qrc
)

target_link_libraries(QtImageEditorV1 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(MSVC)
    target_compile_options(QtImageEditorV1 PRIVATE /utf-8)
endif()

if(WIN32)
    set_target_properties(QtImageEditorV1 PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
