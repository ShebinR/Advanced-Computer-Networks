reducer: reducer.o tcp_client.o message_formats.pb-c.o tcp_client.h thread_info.h communication.o queue.o hash_map_group_by_key.o group_by_key_reducer.o
	gcc -o reducer reducer.o tcp_client.o message_formats.pb-c.o communication.o queue.o hash_map_group_by_key.o group_by_key_reducer.o -lprotobuf-c

message_formats.pb-c.o: message_formats.pb-c.c
	gcc -c message_formats.pb-c.c

tcp_client.o: tcp_client.c message_formats.pb-c.o thread_info.h communication.o
	gcc -c tcp_client.c

reducer.o: reducer.c tcp_client.h constants.h thread_info.h
	gcc -c reducer.c

communication.o: communication.c communication.h
	gcc -c communication.c

queue.o: queue.c queue.h
	gcc -c queue.c

hash_map_group_by_key.o: hash_map_group_by_key.c hash_map_group_by_key.h
	gcc -c hash_map_group_by_key.c

group_by_key_reducer.o: group_by_key_reducer.c group_by_key_reducer.h
	gcc -c group_by_key_reducer.c

clean:
	rm -f reducer *.o

run:
	./reducer reducer_conf.txt
