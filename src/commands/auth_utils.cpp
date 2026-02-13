#include "Server.hpp"
#include <sys/socket.h>


// TODO: check what includes are actually needed

void Server::handleUser(std::shared_ptr<Client>& client, const std::string& username, const std::string& realname, const size_t param_size) {
    if (client->getRegistered()) {
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
    sendWelcome(client);
    return;
}

void Server::sendWelcome(std::shared_ptr<Client>& client) {
    std::string nick = client->getNickName();
    std::string user = client->getUserName();
    std::string host = client->getHostName();

    sendReply(client, RPL_WELCOME, ":Welcome to the 42Network, " + nick);
    sendReply(client, RPL_YOURHOST, ":Your host is " + serverName_ + ", running version " + SERVER_VERSION);
    sendReply(client, RPL_CREATED, ":This server was created on " + creationDate_);
    sendReply(client, RPL_MYINFO, serverName_ + " " + SERVER_VERSION + " " + "o itkl");
    sendReply(client, RPL_ISUPPORT, "NETWORK=42Network NICKLEN=9 CHANMODES=itkl PREFIX=(o)@ :are supported by this server");
    sendReply(client, ERR_NOMOTD, "");
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