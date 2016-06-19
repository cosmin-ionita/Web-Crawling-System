build: Client_.o Link.o Message.o
	g++ server.cpp Client_.o Link.o Message.o -o server
	g++ client.cpp Message.o -o client

Client_.o:
	g++ -c Client_.cpp -o Client_.o
	
Link.o:
	g++ -c Link.cpp -o Link.o

Message.o:
	g++ -c Message.cpp -o Message.o

clean:
	rm *.o server client
