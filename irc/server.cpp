#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include <algorithm>
#include <string>

class user
{
public:
    int id;
    std::string nickName;
    int idRoom;

    bool operator==(const user &other) const
    {
        return this->id == other.id;
    }
    bool operator!=(const user &other)
    {
        return this->id != other.id;
    }

    user(int id_, std::string nickName_) : id(id_), nickName(nickName_), idRoom(0) {}
};

class room
{
public:
    int id;
    std::string name;
    std::vector<user> broadcastList;

    bool operator==(const room &other) const
    {
        return this->id == other.id;
    }

    room(int id_, std::string name_) : id(id_), name(name_) {}
};

std::vector<user> clients;
std::vector<room> rooms;
std::mutex mtx;

void handleClient(user u);
void handleRooms(user u);
void createNewRoom(user u);

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Listening" << std::endl;

    while (true)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::lock_guard<std::mutex> lock(mtx);
        send(new_socket, "Please type your nickname", strlen("Please type your nickname"), 0);
        char userName[1024] = {0};
        read(new_socket, userName, 1024);
        userName[1023] = '\0';
        user tempUser(new_socket, userName);
        clients.push_back(tempUser);
        std::cout << "New client: ID: " << new_socket << ", " << " NickName: " << userName << std::endl;
        rooms.push_back(room(rooms.size() + 1, tempUser.nickName));
        handleRooms(tempUser);
        std::thread(handleClient, tempUser).detach();
    }

    close(server_fd);
    return 0;
}

void handleClient(user u)
{
    char buffer[1024];
    int valread;

    while ((valread = read(u.id, buffer, 1024)) > 0)
    {
        buffer[valread] = '\0';
        std::cout << u.nickName << " : " << buffer << std::endl;

        std::lock_guard<std::mutex> lock(mtx);
        if (std::string(buffer) == "q")
        {
            handleRooms(u);
            continue;
        }

        for (room &room_ : rooms)
        {
            if (u.idRoom == room_.id)
            {
                for (user &user_ : room_.broadcastList)
                {
                    if (user_ != u)
                        send(user_.id, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        clients.erase(std::remove(clients.begin(), clients.end(), u), clients.end());
    }
    close(u.id);
}

void createNewRoom(user u)
{
    std::string prompt = "Please enter the new room's name: ";
    send(u.id, prompt.c_str(), prompt.size(), 0);

    char buffName[1024] = {0};
    ssize_t bytesRead = read(u.id, buffName, sizeof(buffName) - 1);

    if (bytesRead < 0)
    {
        std::cerr << "Error reading from socket" << std::endl;
        close(u.id);
        return;
    }

    buffName[bytesRead] = '\0';

    std::string roomBuffer = "New room's name is: " + std::string(buffName) + " Do you want to continue? (y/n): ";
    send(u.id, roomBuffer.c_str(), roomBuffer.size(), 0);

    char buffApprove[2] = {0};
    read(u.id, buffApprove, 1);
    buffApprove[1] = '\0';
    if (buffApprove[0] == 'y')
    {
        std::string str(buffName);
        room tempRoom(rooms.size() + 1, str);
        {
            rooms.push_back(tempRoom);
        }
    }
}
void handleRooms(user u)
{
    std::string roomBuffer = "";
    char buffer[3] = {0};

    if (rooms.size() > 1)
    {
        for (int i = 1; i <= rooms.size(); i++)
        {
            roomBuffer.append(std::to_string(rooms[i - 1].id));
            roomBuffer.append(" - ");
            roomBuffer.append(rooms[i - 1].name);
            roomBuffer.append("\n");
        }
        roomBuffer.append("Type the id of the room that you want to enter:\n");
        roomBuffer.append("If you want to create a new room, press 'c' and enter:\n");

        send(u.id, roomBuffer.c_str(), roomBuffer.size(), 0);
        read(u.id, buffer, 2);
    }
    else
    {
        buffer[0] = 'c';
        roomBuffer.append("Currently there's no room available. You must create a new room.");
        send(u.id, roomBuffer.c_str(), roomBuffer.size(), 0);
        createNewRoom(u);
        roomBuffer.clear();
        roomBuffer.append("You've entered room : " + rooms[u.idRoom].name + " successfully. Now loading older messages...");
    }

    buffer[2] = '\0';
    if (buffer[0] == 'c')
    {
        createNewRoom(u);
    }
    else
    {
        int roomId = std::atoi(buffer);
        if (roomId > rooms.size() || roomId <= 0)
        {
            send(u.id, "You've entered incorrectly.", sizeof("You've entered incorrectly."), 0);
            return;
        }
        else
        {
            if (u.idRoom != 0)
            {
                auto &currentRoom = rooms[u.idRoom - 1];
                currentRoom.broadcastList.erase(std::remove(currentRoom.broadcastList.begin(), currentRoom.broadcastList.end(), u), currentRoom.broadcastList.end());
            }

            auto &newRoom = rooms[roomId - 1];
            newRoom.broadcastList.push_back(u);
            u.idRoom = roomId;

            // Update user room in the global clients list
            for (auto &client : clients)
            {
                if (client.id == u.id)
                {
                    client.idRoom = roomId;
                    break;
                }
            }
        }
    }
}
