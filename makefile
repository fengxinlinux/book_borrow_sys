all : client server
.PHONY : clean


client:client.cpp
	g++ -pthread client.cpp  -ljson -o client 

server:server.cpp MyDB.cpp
	g++ -pthread server.cpp MyDB.cpp -o server -ljson  `mysql_config --cflags --libs`




clean:
	rm server
	rm client

