#include "Server.hpp"
#include <sys/socket.h>
#include <string>

void Server::handleUser(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.size() != 4) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, c.cmd);
        return;
    }

    if (client->getRegistered()) {
        sendNumericReply(client, ERR_ALREADYREGISTERED, "USER");
        return;
    }

    if (client->getNickName().empty() || !client->getPassAccepted()) {
        sendNumericReply(client, ERR_NOTREGISTERED, "USER");
        return;
    }

    std::string username = c.params[0];
    const std::string& realname = c.params[3];

    if (username.empty()) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "USER");
        return;
    }
    if (username.size() > 12) {
        username.erase(12);
    }
    client->setUserName(username);
    client->setRealName(realname);
    client->setRegistered(true);
    sendWelcome(client);
}

void Server::sendWelcome(std::shared_ptr<Client>& client) {
    std::string nick = client->getNickName();
    std::string user = client->getUserName();
    std::string host = client->getHostName();

    sendNumericReply(client, RPL_WELCOME, ":Welcome to the 42Network, " + nick);
    sendNumericReply(client, RPL_YOURHOST, ":Your host is " + serverName_ + ", running version " + SERVER_VERSION);
    sendNumericReply(client, RPL_CREATED, ":This server was created on " + creationDate_);
    sendNumericReply(client, RPL_MYINFO, serverName_ + " " + SERVER_VERSION + " " + "o itkl");
    sendNumericReply(client, RPL_ISUPPORT, "NETWORK=42Network NICKLEN=9 USERLEN=12 CHANMODES=itkl PREFIX=(o)@ :are supported by this server");
    sendNumericReply(client, ERR_NOMOTD, "");
}

//CAP cmd used for capability negotiation between a server and a client
//to find what extra features that server supports beyond basic IRC
void Server::handleCap(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.empty())
        return;
    if (c.params[0] == "LS") {
        // Empty list = "I support nothing"
        // This tells irssi "ok, no caps, continue registration"
        std::string msg = ":" + serverName_ + " CAP * LS :\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
    }
}

void Server::handlePing(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    std::string token = c.params.empty() ? serverName_ : c.params[0];
    std::string pong = ":" + serverName_ + " PONG " + serverName_ + " :" + token + "\r\n";
    send(client->getFd(), pong.c_str(), pong.length(), 0);
}