EXE = main.exe
CC = gcc
FLAGS = -Wall -pedantic -std=c99
LIBS = -lGLEW -lglfw -lGL -lm
SRC = main.c

all:
	${CC} ${FLAGS} -o ${EXE} ${SRC} ${LIBS}

clean:
	rm *.o -fr
	rm ${EXE}
