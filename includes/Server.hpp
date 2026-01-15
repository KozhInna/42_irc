#pragma once

#include "Client.hpp"
#include "Channel.hpp"
#include <string>
#include <unordered_map>
#include <memory> //for smart pointers

class Server 
{
    public:
        Server(int port, const std::string& password);
        ~Server();

        /**
         * @brief Starts the IRC server by creating, binding, and listening on a socket
         * 
         * Creates a TCP/IPv4 socket, binds it to port_ on all network interfaces (0.0.0.0),
         * and sets it to listen for incoming client connections.
         * 
         * @throws std::runtime_error on socket(), bind(), or listen() failure
         * 
         * @note - socket(): Creates TCP/IPv4 endpoint (AF_INET, SOCK_STREAM)
         * @note - bind():   Associates socket with 0.0.0.0:port_ (all interfaces)
         * @note - listen(): Marks socket as passive with backlog queue of 10
        */
        void start();
        void run();
    
    private:
        //data
        int listenFd_;
        int port_;
        std::string password_;
        std::unordered_map<int, std::shared_ptr<Client>> clients_;
        std::unordered_map<std::string, std::shared_ptr<Channel>> channels_;

        //methods
        std::shared_ptr<Client> acceptNewClient();
        void readMsgFromClient(std::shared_ptr<Client> client);
        void processMessage(std::shared_ptr<Client>& client, const std::string& msg);
};