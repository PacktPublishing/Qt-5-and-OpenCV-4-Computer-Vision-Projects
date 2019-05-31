TEMPLATE = app
TARGET = GLFilter

QT += core gui widgets

INCLUDEPATH += .


# use your own path in the following config
unix: !mac {
    INCLUDEPATH += /home/kdr2/programs/opencv/include/opencv4
    LIBS += -L/home/kdr2/programs/opencv/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
}

unix: mac {
    INCLUDEPATH += /path/to/opencv/include/opencv4
    LIBS += -L/path/to/opencv/lib -lopencv_world
}

win32 {
    INCLUDEPATH += c:/path/to/opencv/include/opencv4
    LIBS += -lc:/path/to/opencv/lib/opencv_world
}


DEFINES += QT_DEPRECATED_WARNINGS

# Input
HEADERS += glpanel.h
SOURCES += main.cpp glpanel.cpp

RESOURCES = res.qrc
