all : client server
.PHONY : clean


client:client.cpp
		g++ -pthread client.cpp  -ljson -o client -std=c++11

server:server.cpp MyDB.cpp
		g++ -pthread server.cpp MyDB.cpp -o server -ljson  `mysql_config --cflags --libs` -lpthread  -std=c++11




clean:
		rm server
			rm client
