#include <iostream>
#include <thread>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void receive_messages(int socket) {
    char buffer[1024];
    int valread;
    while ((valread = read(socket, buffer, 1024)) > 0) {
        buffer[valread] = '\0';
        std::cout << "Message: " << buffer << std::endl;
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << "Invalid address/ Address not supported\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection Failed\n";
        return -1;
    }

    std::thread(receive_messages, sock).detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        send(sock, message.c_str(), message.length(), 0);
    }

    close(sock);
    return 0;
}
