EXE = main.exe
CXX = g++
FLAGS = -Wall -I/home/kdr2/programs/opencv/include/opencv4
LIBS = -lGLEW -lglfw -lGL -lm
LIBS += -L/home/kdr2/programs/opencv/lib -lopencv_core -lopencv_highgui
SRC = main.c

all:
	${CXX} ${FLAGS} -o ${EXE} ${SRC} ${LIBS}

clean:
	rm *.o -fr
	rm ${EXE}
