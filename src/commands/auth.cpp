#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
 #include <sys/socket.h>

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

 void Server::handleNick(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    // check is Pass is accepted
    if (!client->getPassAccepted()) {
        sendReply(client, ERR_NOTREGISTERED, ""); //There's NO standard IRC error, so send generic
        disconnectClient(client->getFd());
        return;
    }

    // check if nick is sent
    if (c.params.empty()) {
        sendReply(client, ERR_NONICKNAMEGIVEN, "");
        return;
    }

    const std::string& nickname = c.params[0];
    //check if nick is of valid format
    if (!isValidNickname(nickname)) {
        sendReply(client, ERR_ERRONEUSNICKNAME, nickname); 
        return;
    }
    
    //check if nickname is already in use by somebody else
    for (const auto& pair : clients_) {
        const std::shared_ptr<Client>& existingClient = pair.second;
        if (existingClient != client && nickname == existingClient->getNickName()) {
            sendReply(client, ERR_NICKNAMEINUSE, nickname); 
            return;
        } 
    }

    client->setNickName(nickname);
    //TODO: handle nickname change
    /*     User is CHANGING nickname (already had one)
            Notify the client and everyone in their channels */
 }

 bool Server::isValidNickname(const std::string& nick) {
    // RFC 2812: max 9 characters
    if (nick.length() > 9) {
        return false;
    }
        // First character: letter or special
        char first = nick[0];
        bool firstIsLetter = std::isalpha(first);
        bool firstIsSpecial = (first >= '[' && first <= '`') || 
                              (first >= '{' && first <= '}');
        
        if (!firstIsLetter && !firstIsSpecial) {
            return false;
        }
    // Rest: letter, digit, special, or hyphen
    for (size_t i = 1; i < nick.length(); i++) {
        char c = nick[i];
        bool isLetter = std::isalpha(c);
        bool isDigit = std::isdigit(c);
        bool isSpecial = (c >= '[' && c <= '`') || 
                            (c >= '{' && c <= '}');
        bool isHyphen = (c == '-');
        
        if (!isLetter && !isDigit && !isSpecial && !isHyphen) {
            return false;
        }
    }
    return true;
 }

 void Server::handleQuit(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    std::string quitMsg = "Client quit";

    if (!c.params.empty()) {
        quitMsg = c.params[0];
    }

    // 2. REQUIRED: Notify other users in channels
    // (I'll add this when you have channels)
    // notifyQuit(client, quitMsg);
    //e.g. :alice!user@host QUIT :Client exited

    // 3. Send ERROR to quitting client
    std::string errorMsg = "ERROR :Closing Link: " + serverName_ + " (" + quitMsg + ")\r\n";
    send(client->getFd(), errorMsg.c_str(), errorMsg.length(), 0);

    //4. Close connection
    disconnectClient(client->getFd());
 }