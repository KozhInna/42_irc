#include "Server.hpp"
#include "Channel.hpp"
#include <sys/socket.h>

void Server::handlePrivmsg(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.size() < 2 || c.params[0].empty()) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PRIVMSG");
        return;
    }

    //in case user omitted ":", the first param is taken as msg, others ignored

    //If target is a channel
    if (c.params[0][0] == '#') {
        std::string channelName = c.params[0];

        //Find channel
        std::shared_ptr<Channel> channel = findChannel(c.params[0]);
        if (!channel) {
            sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName);
            return;
        }
        //Check if user is a channel member
        if (!channel->isMember(client.get())) {
            sendNumericReply(client, ERR_CANNOTSENDTOCHAN, channelName);
            return;
        }

        //Broadcast to channel
        std::string reply = formatClientAction(client, "PRIVMSG", channelName + " :" + c.params[1]);
        broadcastToChannel(channel.get(), reply, client.get());        
    } else {
     //If target is a user
        std::shared_ptr<Client> targetClient = getClientByNick(c.params[0]);
        if (!targetClient) {
            sendNumericReply(client, ERR_NOSUCHNICK, c.params[0]);
            return;
        } else {
            std::string reply = formatClientAction(client, "PRIVMSG", targetClient->getNickName() + " :" + c.params[1]);
            queueMessage(targetClient, reply);
        }
    }
}
