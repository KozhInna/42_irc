#pragma once

#include "Client.hpp"
#include "Channel.hpp"
#include <string>
#include <unordered_map>
#include <memory> //for smart pointers

class Server 
{
    private:
        int listenFd;
        int port;
        std::string password;
        std::unordered_map<int, std::shared_ptr<Client>> clients;
        std::unordered_map<std::string, std::shared_ptr<Channel>> channels;

    public:
        Server(int port, const std::string& password);
        ~Server();
        void start();
};