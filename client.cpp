#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "udp.hpp"

#define PORT 4000

int main(int argc, char *argv[])
{
  int n;
	unsigned int length;
  struct sockaddr_in from;

	char buffer[256];
	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);
	}

  UDPSocket* socket = new UDPSocket(PORT, argv[1]);

	printf("Enter the message: ");
	bzero(buffer, 256);
	fgets(buffer, 256, stdin);

	n = sendto(socket->getSocketDesc(), buffer, strlen(buffer), 0, (const struct sockaddr *) socket->getAddr(), sizeof(struct sockaddr_in));
	if (n < 0)
		printf("ERROR sendto");

	length = sizeof(struct sockaddr_in);
	n = recvfrom(socket->getSocketDesc(), buffer, 256, 0, (struct sockaddr *) socket->getAddrFrom(), &length);
	if (n < 0)
		printf("ERROR recvfrom");

	printf("Got an ack: %s\n", buffer);

	close(socket->getSocketDesc());
	return 0;
}
