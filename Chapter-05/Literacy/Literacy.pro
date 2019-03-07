TEMPLATE = app
TARGET = Literacy

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += .

DEFINES += QT_DEPRECATED_WARNINGS

# Input
HEADERS += mainwindow.h
SOURCES += main.cpp mainwindow.cpp
