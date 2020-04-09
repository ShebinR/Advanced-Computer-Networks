Mapper: Mapper.o message_formats.pb-c.o hash_map.o hash_map.h
	gcc -o Mapper Mapper.o message_formats.pb-c.o -lprotobuf-c hash_map.o

testmessage.pb-c.o: message_formats.pb-c.c
	gcc -c message_formats.pb-c.c

Mapper.o: Mapper.c Constants.h message_formats.pb-c.h hash_map.h
	gcc -c Mapper.c

hash_map.o: hash_map.c Constants.h
	gcc -c hash_map.c

clean:
	rm Mapper *.o
