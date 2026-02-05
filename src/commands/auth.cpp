#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void Server::handlePass(std::shared_ptr<Client>& client, const ParsedCommand& c) {
        if (c.params.empty()) {
            sendReply(client, ERR_NEEDMOREPARAMS, c.cmd); // 461 - Include command name
            return;
        }
        if (client->getPassAccepted()) {
            sendReply(client, ERR_ALREADYREGISTERED, "");
            return;
        }
        // Only the first parameter is used; any extra words are ignored
        const std::string& firstParam = c.params[0];
        if (firstParam != password_) {
            sendReply(client, ERR_PASSWDMISMATCH, "");
            disconnectClient(client->getFd());
            return;
        }
        client->setPassAccepted(true);
}

/* 
    PASS secret123 extraParam
    - most IRC servers are forgiving and only take the first word
 */