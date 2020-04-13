mapper: mapper.o message_formats.pb-c.o hash_map.o communication.o tcp_server.o
	gcc -o mapper mapper.o message_formats.pb-c.o hash_map.o communication.o tcp_server.o -lprotobuf-c

testmessage.pb-c.o: message_formats.pb-c.c
	gcc -c message_formats.pb-c.c

mapper.o: mapper.c hash_map.o constants.h message_formats.pb-c.h hash_map.h communication.h
	gcc -c mapper.c

hash_map.o: hash_map.c constants.h
	gcc -c hash_map.c

communication.o: communication.c
	gcc -c communication.c

tcp_server.o: tcp_server.c tcp_server.h
	gcc -c tcp_server.c

clean:
	rm mapper *.o
