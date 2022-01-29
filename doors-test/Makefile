CC = clang
CCFLAGS = -Wall -Wextra -fPIC -g
BSD_SRC = ../../freebsd-src
INCLUDEDIR = -I${BSD_SRC}/include  -I${BSD_SRC}/sys
LDFLAGS = -L ../libdoor -ldoor -lpthread

all: runner

runner:
	${MAKE} -C fnv
	${CC} ${LDFLAGS} ${INCLUDEDIR} ${CCFLAGS} door_parallel_test.c door_helper.c ./fnv/libfnv.a -o runner_parallel
	${CC} ${LDFLAGS} ${INCLUDEDIR} ${CCFLAGS} door_basic_test.c door_helper.c ./fnv/libfnv.a -o runner

clean:
	${MAKE} -C fnv clean
	rm -f runner* *.o *~ *.core serv

