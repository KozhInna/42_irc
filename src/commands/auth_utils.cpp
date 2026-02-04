#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <iterator>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>


// TODO: check what includes are actually needed

int Server::cmdUser(std::shared_ptr<Client>& client, std::string &username, std::string &realname) {
    if (client->getRegistered() == true)
        return 462; // 461 ERR_ALREADYREGISTRED

    if (client->getNickName().empty() || !client->getAuthPassed()) {
        return 451; // 451 ERR_NOTREGISTERED
    }

    client->setUsername(username);
    client->setRealName(realname);
    client->setRegistered(true);
    return 0; // success
}