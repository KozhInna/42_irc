#include "Channel.hpp"
#include "Server.hpp"

void Server::handleKick(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.size() < 2) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "KICK");
        return;
    }

    std::string channelName = c.params[0];

    //Find channel
    std::shared_ptr<Channel> channel = findChannel(c.params[0]);
    if (!channel) {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName);
        return;
    }
    //Check if user is a channel member
    if (!channel->isMember(client.get())) {
        sendNumericReply(client, ERR_NOTONCHANNEL, channelName);
        return;
    }
    //Check if user is an operator
    if (!channel->isOperator(client.get())) {
        sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName);
        return;
    }

    //If target is a user
    std::shared_ptr<Client> targetClient = getClientByNick(c.params[1]);
    if (!targetClient) {
        sendNumericReply(client, ERR_NOSUCHNICK, c.params[1]);
        return;
    }
    //Check if target-user is a channel member
    if (!channel->isMember(targetClient.get())) {
        sendNumericReply(client, ERR_USERNOTINCHANNEL, c.params[1] + " " + channelName);
        return;
    }

    //Broadcast to channel
    std::string msgText = channelName + " " + c.params[1];
    if (c.params.size() > 2 && !c.params[2].empty()) {
        msgText = msgText + " :" + c.params[2];
    }
    std::string reply = formatClientAction(client, "KICK", msgText);
    channel->broadcastToChannel(reply);

    //remove target-user from channel
    channel->removeMember(targetClient.get());
    channel->removeOperator(targetClient.get());

    if (channel->getNumberMembers() == 0) {
        channels_.erase(channelName);
    }
}