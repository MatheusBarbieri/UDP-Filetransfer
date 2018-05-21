CC = g++
FLAGS = -g -Wall -lpthread -std=gnu++11
CFLAGS = $(FLAGS) $(INCLUDE)

UTILS = udp.o

CLIENT_O = client.o

SERVER_O = server.o

.PHONY: all clean

all: udpServer udpClient removeObjects

udpServer: $(UTILS) $(SERVER_O)
	$(CC) $(CFLAGS) -o $@ $^

udpClient: $(UTILS) $(CLIENT_O)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean: removeObjects
	rm -f udpClient udpServer

removeObjects:
	rm -f $(UTILS) $(SERVER_O) $(CLIENT_O)

runc:
	./udpClient localhost

runs:
	./udpServer

udp:
		$(CC) $(CFLAGS) -c udp.cpp -o udp.o
		rm -f udp.o
