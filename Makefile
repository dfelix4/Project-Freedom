#
# Makefile for 50 shades of Freedom game
#
# Enter one of the following
#
# make
# make all
# make asteroids
# make clean
#
CFLAGS = -I ./include
#LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm
LFLAGS = -lrt -lX11 -lGL

all: projectFreedom scoreBoard

projectFreedom: projectFreedom.cpp log.cpp timers.cpp dangeloF.cpp
	g++ $(CFLAGS) projectFreedom.cpp log.cpp timers.cpp dangeloF.cpp \
	    joshuaR.cpp juanO.cpp andresZ.cpp \
	libggfonts.a   -Wall $(LFLAGS) -o projectFreedom
scoreBoard: scoreBoard.txt
	cat cleanBoard.txt > scoreBoard.txt
clean:
	rm -f projectFreedom
	rm -f *.o

