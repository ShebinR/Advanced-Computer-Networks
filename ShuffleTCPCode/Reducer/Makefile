Reducer: Reducer.o TCPClient.o message_formats.pb-c.o TCPClient.h
	gcc -o Reducer Reducer.o TCPClient.o message_formats.pb-c.o -lprotobuf-c

message_formats.pb-c.o: message_formats.pb-c.c
	gcc -c message_formats.pb-c.c

TCPClient.o: TCPClient.c TCPClient.h message_formats.pb-c.o ThreadInfo.h
	gcc -c TCPClient.c

Reducer.o: Reducer.c TCPClient.h Constants.h ThreadInfo.h
	gcc -c Reducer.c

clean:
	rm -f Reducer *.o

run:
	./Reducer reducer_conf.txt
