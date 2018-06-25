CC = g++
FLAGS = -g -Wall -pthread -std=c++14
CFLAGS = $(FLAGS) $(INCLUDE)

UTILS = udp.o \
		util.o \
		client.o \
		server.o \
		serverManager.o \
		task.o \
		filesystem.o \

CLIENT_O = dropboxclient.o

SERVER_O = dropboxserver.o

.PHONY: all clean

all: dropboxserver dropboxclient removeObjects

dropboxserver: $(UTILS) $(SERVER_O)
	$(CC) $(CFLAGS) -o $@ $^

dropboxclient: $(UTILS) $(CLIENT_O)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean: removeObjects
	rm -f dropboxclient dropboxserver

removeObjects:
	rm -f $(UTILS) $(SERVER_O) $(CLIENT_O)

runc:
	./dropboxclient Barbor localhost 8080

runs:
	./dropboxserver 8080

udp:
		$(CC) $(CFLAGS) -c udp.cpp -o udp.o
