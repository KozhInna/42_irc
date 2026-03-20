#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <sys/socket.h>

void Server::handlePass(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.empty()) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, c.cmd); // 461 - Include command name
        return;
    }
    if (client->getPassAccepted()) {
        sendNumericReply(client, ERR_ALREADYREGISTERED, "");
        return;
    }
    // Only the first parameter is used; any extra words are ignored
    const std::string& firstParam = c.params[0];
    if (firstParam != password_) {
        sendNumericReply(client, ERR_PASSWDMISMATCH, "");
        client->setPassAccepted(false);
        return;
    }
    client->setPassAccepted(true);
}

/* 
    PASS secret123 extraParam
    - most IRC servers are forgiving and only take the first word
 */

 void Server::handleNick(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    // check if Pass is accepted
    if (!client->getPassAccepted()) {
        sendNumericReply(client, ERR_NOTREGISTERED, "");
        disconnectClient(client->getFd());
        return;
    }

    // check if nick is sent
    if (c.params.empty()) {
        sendNumericReply(client, ERR_NONICKNAMEGIVEN, "");
        return;
    }

    const std::string& nickname = c.params[0];
    //check if nick is of valid format
    if (nickname.empty()) {
        sendNumericReply(client, ERR_ERRONEUSNICKNAME, nickname); 
        return;
    }
    if (!isValidNickname(nickname)) {
        sendNumericReply(client, ERR_ERRONEUSNICKNAME, nickname); 
        return;
    }
    
    //check if nickname is already in use by somebody else
    for (const auto& pair : clients_) {
        const std::shared_ptr<Client>& existingClient = pair.second;
        if (existingClient != client && nickname == existingClient->getNickName()) {
            sendNumericReply(client, ERR_NICKNAMEINUSE, nickname); 
            return;
        } 
    }

    if (client->getNickName().empty()) {
        client->setNickName(nickname);
    } else {
        std::string reply = formatClientAction(client, "NICK", nickname);
        send(client->getFd(), reply.c_str(), reply.length(), 0);  // notify the client themselves
        for (auto channel : channels_) {
            if (channel.second->isMember(client.get())) {
                channel.second->broadcastToChannel(reply, client.get());
            }
        }
        client->setNickName(nickname);
    }
 }

 bool Server::isValidNickname(const std::string& nick) {
    // RFC 2812: max 9 characters
    if (nick.length() > 9) {
        return false;
    }
        // First character: letter or special
        char first = nick[0];
        bool firstIsLetter = std::isalpha((unsigned char)first);
        bool firstIsSpecial = (first >= '[' && first <= '`') || 
                              (first >= '{' && first <= '}');
        
        if (!firstIsLetter && !firstIsSpecial) {
            return false;
        }
    // Rest: letter, digit, special, or hyphen
    for (size_t i = 1; i < nick.length(); i++) {
        char c = nick[i];
        bool isLetter = std::isalpha((unsigned char)c);
        bool isDigit = std::isdigit((unsigned char)c);
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
    std::string quitMsg = "";

    if (!c.params.empty()) {
        quitMsg = c.params[0];
    }

    Client *client_ptr = client.get();

    // 1. Remove client from channel, notify everybody
    std::string quiteReason = formatClientAction(client, "QUIT", ":Quit: " + quitMsg);
    broadcastQuitAndCleanup(client_ptr, quiteReason);

    // 2. Send ERROR to quitting client
    std::string errorMsg = "ERROR :Closing Link: " + serverName_ + " (" + quitMsg + ")\r\n";
    send(client->getFd(), errorMsg.c_str(), errorMsg.length(), 0);

    //3. Close connection
    disconnectClient(client->getFd());
 }