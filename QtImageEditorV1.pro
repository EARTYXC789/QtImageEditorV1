# 如果學校使用 qmake 專案，也可以在 Qt Creator 開啟此 .pro。
QT += core gui widgets
CONFIG += c++17
TEMPLATE = app
TARGET = QtImageEditorV1

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/imageview.cpp \
    src/exportdialog.cpp

HEADERS += \
    src/mainwindow.h \
    src/imageview.h \
    src/exportdialog.h

RESOURCES += resources/icons.qrc

win32-msvc:QMAKE_CXXFLAGS += /utf-8
