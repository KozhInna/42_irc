#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>

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