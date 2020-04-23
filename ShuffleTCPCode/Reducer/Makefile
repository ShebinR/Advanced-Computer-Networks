CC=gcc
CFLAGS=-std=c99
LINKER_FLAGS=-lprotobuf-c -lpthread

reducer: reducer.o tcp_client.o message_formats.pb-c.o tcp_client.h thread_info.h communication.o queue.o hash_map_group_by_key.o group_by_key_reducer.o
	$(CC) $(CFLAGS) -o reducer reducer.o tcp_client.o message_formats.pb-c.o communication.o queue.o hash_map_group_by_key.o group_by_key_reducer.o $(LINKER_FLAGS)

message_formats.pb-c.o: message_formats.pb-c.c
	$(CC) $(CFLAGS) -c message_formats.pb-c.c

tcp_client.o: tcp_client.c message_formats.pb-c.o thread_info.h communication.o
	$(CC) $(CFLAGS) -c tcp_client.c

reducer.o: reducer.c tcp_client.h constants.h thread_info.h
	$(CC) $(CFLAGS) -c reducer.c

communication.o: communication.c communication.h
	$(CC) $(CFLAGS) -c communication.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

hash_map_group_by_key.o: hash_map_group_by_key.c hash_map_group_by_key.h
	$(CC) $(CFLAGS) -c hash_map_group_by_key.c

group_by_key_reducer.o: group_by_key_reducer.c group_by_key_reducer.h
	$(CC) $(CFLAGS) -c group_by_key_reducer.c

clean:
	rm -f reducer *.o

run:
	./reducer reducer_conf.txt
