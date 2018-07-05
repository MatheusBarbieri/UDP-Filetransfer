#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

#include "client.hpp"

int main(int argc, char **argv){
    if (argc < 4) {
        std::cout << "Usage:\n\t./udpClient <username> <host> <port>" << std::endl;
        return 0;
    }
    std::string username = argv[1], host = argv[2];
    int port = atoi(argv[3]);

    UDPClient udpclient(port, host);
    Client client(username, udpclient);
    client.udpClient.connect();
    int response;
    response = client.udpClient.waitResponse();
    std::cout << "Response: " << response << std::endl;
    if (response == ACCEPT){
        std::cout << "Conectado com sucesso!" << std::endl;
    } else {
        std::cout << "Não foi possível conectar." << std::endl;
        return 0;
    }

    Datagram dg;
    std::cout << "Teste\n";
    int test = client.udpClient.sendDatagramMaxTries(dg, 5);

    std::cout << "test: " << test << std::endl;


    client.startThreads();

    close(client.udpClient.getSocketDesc());
    return 0;

    // FILE * file = fopen("doggo.jpeg", "r");
    // client.udpClient.sendFile(file);
    // std::cout << "Mandou a file!\n" << std::endl;
    // fclose(file);
    //
    // close(client.udpClient.getSocketDesc());
    // return 0;
}
