TEMPLATE = app
TARGET = Literacy

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += .

# use your own path in the following config
unix: {
    INCLUDEPATH += /home/kdr2/programs/tesseract/include
    LIBS += -L/home/kdr2/programs/tesseract/lib -ltesseract
}

win32 {
    INCLUDEPATH += c:/path/to/tesseract/include
    LIBS += -lc:/path/to/opencv/lib/tesseract
}

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += TESSDATA_PREFIX=\\\"/home/kdr2/programs/tesseract/share/tessdata/\\\"

# Input
HEADERS += mainwindow.h
SOURCES += main.cpp mainwindow.cpp
