
CC=g++ -std=c++11 -g -Wall -pthread

default: server.cpp client.cpp
	$(CC) -Wall -Wextra -g -o server -I. server.cpp
	$(CC) -Wall -Wextra -g -o client -I. client.cpp

dist:
	tar -czvf project2.tar.gz server.cpp client.cpp packet.h Makefile

clean:
	rm project2.tar.gz
