#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>

//constructor
Server::Server(int port, const std::string& password) 
    :   listenFd_(-1),
        port_(port), 
        password_(password)
{
    //std::cout << "Server constructor\n" << std::endl;
}

//destructor
Server::~Server()
{
    if (listenFd_ != -1) {
        close(listenFd_);
        listenFd_ = -1;
    }
    //std::cout << "Server destructor\n" << std::endl;
}

void Server::start() 
{
    // Create TCP/IPv4 socket (UNBOUND: no IP/port yet)
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ == -1) {
        throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
    }

    // Bind to all interfaces on specified port (state: BOUND)
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;          // IPv4
    addr.sin_port = htons(port_);       // Network byte order
    addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0 (all interfaces)

    if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        throw std::runtime_error(std::string("bind() failed: ") + strerror(errno));
    }

    // Mark as listening socket (state: LISTENING, backlog: 10)
    if (listen(listenFd_, 10) == -1) {
        throw std::runtime_error(std::string("listen() failed: ") + strerror(errno));
    }
}

void Server::run() {
    while (true) {
        std::shared_ptr<Client> client = acceptNewClient();
        if (!client) { continue; }
        readMsgFromClient(client);
    }
}

std::shared_ptr<Client> Server::acceptNewClient() {
    struct sockaddr_in clientSock;
    socklen_t clientSockLen = sizeof(clientSock);

    int clientFd = accept(listenFd_, (struct sockaddr *)&clientSock, &clientSockLen);
    if (clientFd == -1) {
        std::cerr << "Error: accept() failed: " << strerror(errno) << std::endl;
        return nullptr;
    }
    std::shared_ptr<Client> client = std::make_shared<Client>(clientFd);
    clients_.insert({clientFd, client});
    return client;
}

void Server::processMessage(std::shared_ptr<Client>& client, const std::string& msg) {
    std::cout << "Client sent message: " << msg << std::endl;
    (void)client;
}

void Server::readMsgFromClient(std::shared_ptr<Client> client) {
    char tempBuf[512];
    while (true) {
        ssize_t bytesRead = recv(client->getFd(), tempBuf, sizeof(tempBuf), 0);
        if (bytesRead > 0) {
            client->getBuffer().append(tempBuf, bytesRead);
            size_t pos;
            while ((pos = client->getBuffer().find("\n")) != std::string::npos) {
                std::string message = client->getBuffer().substr(0, pos);
                processMessage(client, message);
                client->getBuffer().erase(0, pos + 1);
            }
        } else if (bytesRead == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        } else if (bytesRead == -1) {
            std::cerr << "Error: recv() failed: " << strerror(errno) << std::endl;
            break;
        }
    }
    clients_.erase(client->getFd());
    close(client->getFd());
}