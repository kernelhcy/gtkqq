CFLAGS=`pkg-config --libs --cflags glib-2.0` -Wall -O2 -ggdb
CC=gcc

jsontest:json.o testjson.o
	${CC} ${CFLAGS} json.o testjson.o -o testjson
json.o:json.c
	${CC} ${CFLAGS} json.c -c -o json.o
testjson.o:testjson.c
	${CC} ${CFLAGS} testjson.c -c -o testjson.o

httptest:http.o testhttp.o
	$(CC) $(CFLAGS) http.o testhttp.o -o testhttp
http.o:http.c
	$(CC) $(CFLAGS) -c http.c -o http.o
testhttp.o:testhttp.c
	$(CC) $(CFLAGS) -c testhttp.c -o testhttp.o

clean:
	rm *.o
	
