#include "Server.hpp"
#include "Channel.hpp"

void Server::handlePart(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.empty()) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PART");
        return;
    }

    std::string channelName = c.params[0];
    std::shared_ptr<Channel> channel = findChannel(channelName);
    
    if (!channel) {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName);
        return;
    }

    if (!channel->isMember(client.get())) {
        sendNumericReply(client, ERR_NOTONCHANNEL, channelName);
        return;
    }

    std::string partMsg = channelName;
    if (c.params.size() > 1) {
        partMsg += " :" + c.params[1];
    }

    std::string reply = formatClientAction(client, "PART", partMsg);
    channel->broadcastToChannel(reply);

    if (channel->isOperator(client.get())) {
        channel->removeOperator(client.get());
    }
    if (channel->isInvited(client.get())) {
        channel->removeInvited(client.get());
    }
    channel->removeMember(client.get());

    if (channel->getNumberMembers() == 0) {
        channels_.erase(channelName);
    }
}
