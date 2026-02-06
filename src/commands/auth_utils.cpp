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

void Server::cmdUser(std::shared_ptr<Client>& client, const std::string& username, const std::string& realname, const size_t param_size) {
    if (client->getRegistered() == true) {
        sendReply(client, ERR_ALREADYREGISTERED, "USER");
        return;
    }

    if (client->getNickName().empty() || !client->getPassAccepted()) {
        sendReply(client, ERR_NOTREGISTERED, "USER");
        return;
    }

    if (param_size != 4) {
        sendReply(client, ERR_NEEDMOREPARAMS, "USER");
        return;
    }

    client->setUserName(username);
    client->setRealName(realname);
    client->setRegistered(true);
    return;
}