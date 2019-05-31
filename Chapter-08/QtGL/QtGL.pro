TEMPLATE = app
TARGET = QtGL

QT += core gui widgets

INCLUDEPATH += .

DEFINES += QT_DEPRECATED_WARNINGS

# Input
HEADERS += glpanel.h
SOURCES += main.cpp glpanel.cpp

RESOURCES = shaders.qrc
