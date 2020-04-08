Reducer: Reducer.o TCPClient.o TCPClient.h Constants.h ThreadInput.h
	gcc -o Reducer Reducer.o TCPClient.o

TCPClient.o: TCPClient.c TCPClient.h ThreadInput.h Protocol.h
	gcc -c TCPClient.c

Reducer.o: Reducer.c TCPClient.h Constants.h ThreadInput.h
	gcc -c Reducer.c

clean:
	rm -f Reducer *.o

run:
	./Reducer reducer_conf.txt
