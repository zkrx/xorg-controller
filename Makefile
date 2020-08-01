
CC=gcc
CFLAGS=-O4
EVDEV_IDIR=/usr/include/libevdev-1.0
INCLUDES=-I${EVDEV_IDIR}

EVDEV_LIB=evdev
LDFLAGS=-l${EVDEV_LIB}

%.o: %.c
	${CC} -c -o $@ $< ${CFLAGS} ${INCLUDES} ${LIBS}


BallMouse: BallMouse.o


.IGNORE: clean

clean:
	rm -f *.o




