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
    std::cout << "Server constructor\n" << std::endl;
}

//destructor
Server::~Server()
{
    if (listenFd_ != -1) {
        close(listenFd_);
        listenFd_ = -1;
    }
    std::cout << "Server destructor\n" << std::endl;
}

void Server::start() 
{
    // create unnamed socket (no address yet)
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ == -1) {
        throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
    }

    //"name it" - bind to IP address and port - "named" as 0.0.0.0:6667
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        throw std::runtime_error(std::string("bind() failed: ") + strerror(errno));
    }

    if (listen(listenFd_, 10) == -1) {
        throw std::runtime_error(std::string("listen() failed: ") + strerror(errno));
    }


    std::cout << "Server listening on port " << port_ << std::endl;

}