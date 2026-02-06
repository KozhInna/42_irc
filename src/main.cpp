#include "Server.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    std::string password = argv[2];
    int port = std::stoi(argv[1]);
    std::cout << "pass - " << password << std::endl;
    std::cout << "port - " << port << std::endl;

    Server server(port, password);

    try {
        server.start();
        std::cout << "Server listening on port " << port << std::endl;
        
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}