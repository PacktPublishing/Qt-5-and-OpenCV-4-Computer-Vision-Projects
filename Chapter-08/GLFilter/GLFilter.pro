TEMPLATE = app
TARGET = GLFilter

QT += core gui widgets

INCLUDEPATH += .

DEFINES += QT_DEPRECATED_WARNINGS

# Input
HEADERS += glpanel.h
SOURCES += main.cpp glpanel.cpp

RESOURCES = res.qrc
