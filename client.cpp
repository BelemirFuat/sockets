// C++ program to illustrate the client application in the
// socket programming
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void sendMessage(int);

int main()
{
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // sending connection request
    connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    // sending data
    sendMessage(clientSocket);
    char buffer[1024];
    read(clientSocket, buffer, 1024);
    std::cout<<"Message : " << buffer << std::endl;

    // closing socket
    close(clientSocket);

    return 0;
}
void sendMessage(int clientSocket)
{
    std::string message;
    std::cout << "Enter your message" << std::endl;
    std::getline(std::cin, message);                          // Read the entire line of input
    send(clientSocket, message.c_str(), message.length(), 0); // Convert string to C-style string and send
}
