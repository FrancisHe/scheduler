CC = gcc
CFLAGS = -Wall
round-robin: round-robin.c common.o
	${CC} ${CFLAGS} round-robin.c common.o -o $@
common.o: common.h common.c
	${CC} ${CFLAGS} -c common.c
process: ../sigtrap.c
	${CC} ${CFLAGS} ../sigtrap.c -o $@
clean:
	rm -f round-robin *.o
