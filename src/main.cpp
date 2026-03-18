#include "Server.hpp"
#include <iostream>
#include <string>
#include <unistd.h>


bool isValidPort(const std::string& str) {
    
    for (size_t i = 0; i < str.length(); i++) {
        if (!isdigit(str[i]))
            return false;
    }
    
    try {
        int port = std::stoi(str);
        if (port < 1024 || port > 65535)
            return false;
    
    } 
    catch (const std::exception& e) {
        return false;
    }

    return true;
}


int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    if (!isValidPort(argv[1])) {
        std::cerr << "Error: Invalid port number. Port must be an integer between 1024 and 65535." << std::endl;
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