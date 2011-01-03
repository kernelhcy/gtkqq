CFLAGS=`pkg-config --libs --cflags glib-2.0` -Wall -O2 -ggdb
CC=gcc

all:unidecode.c
	${CC} ${CFLAGS} unidecode.c -ounidecode

clean:
	rm *.o
	
