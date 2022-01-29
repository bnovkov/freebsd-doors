CC = clang
CCFLAGS = -Wall -Wextra -fPIC -g
INCLUDEDIR = ../../../src-bak/include  -I../../../src-bak/sys
LDFLAGS = -L ../../libdoor -ldoor
CLIENT = client
SERVER = server 
CLIENT_SOURCES = client.c
SERVER_SOURCES = server.c
CLIENT_OBJECTS=${CLIENT_SOURCES:.c=.o}
SERVER_OBJECTS=${SERVER_SOURCES:.c=.o}

all : ${SERVER} ${CLIENT}


clean:
	rm -f ${SERVER} ${CLIENT} *.o *~ *.core

${SERVER}: ${SERVER_OBJECTS}
	${CC} ${LDFLAGS} ${SERVER_OBJECTS} -o ${SERVER}

${CLIENT}: ${CLIENT_OBJECTS}
	${CC} ${LDFLAGS} ${CLIENT_OBJECTS} -o ${CLIENT}


.c.o:
	${CC} -I${INCLUDEDIR} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}
