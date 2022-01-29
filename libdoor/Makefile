CC = clang
CCFLAGS = -Wall -Wextra -fPIC -g
BSD_SRC= ../../freebsd-src
INCLUDEDIR = ${BSD_SRC}/sys -I${BSD_SRC}/include
LDFLAGS = -shared -lpthread
OUTPUT = libdoor.so
SOURCES = door.c
OBJECTS=${SOURCES:.c=.o}

all : ${OUTPUT}

clean:
	rm -f ${OUTPUT} *.o *~

${OUTPUT}: ${OBJECTS}
	${CC} ${LDFLAGS} ${OBJECTS} -o ${OUTPUT}

.c.o:
	${CC} -I${INCLUDEDIR} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}
