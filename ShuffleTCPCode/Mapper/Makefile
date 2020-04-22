CC=gcc
CFLAGS= -std=c99
LINKER_FLAGS=-lprotobuf-c -lpthread

mapper: mapper.o message_formats.pb-c.o hash_map.o communication.o tcp_server.o
	$(CC) -o mapper mapper.o message_formats.pb-c.o hash_map.o communication.o tcp_server.o $(LINKER_FLAGS)

testmessage.pb-c.o: message_formats.pb-c.c
	$(CC) $(CFLAGS) -c message_formats.pb-c.c

mapper.o: mapper.c hash_map.o constants.h message_formats.pb-c.h hash_map.h communication.h
	$(CC) $(CFLAGS) -c mapper.c

hash_map.o: hash_map.c constants.h
	$(CC) $(CFLAGS) -c hash_map.c -std=c99

communication.o: communication.c
	$(CC) $(CFLAGS) -c communication.c

tcp_server.o: tcp_server.c tcp_server.h
	$(CC) $(CFLAGS) -c tcp_server.c

clean:
	rm mapper *.o
