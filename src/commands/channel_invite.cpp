#include "Server.hpp"
#include "Channel.hpp"
#include <sys/socket.h>

void Server::handleInvite(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.size() < 2) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "INVITE");
        return;
    }

    std::string targetNick = c.params[0];
    std::string channelName = c.params[1];

    std::shared_ptr<Channel> channel = findChannel(channelName);
    if (!channel) {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName);
        return;
    }

    if (!channel->isMember(client.get())) {
        sendNumericReply(client, ERR_NOTONCHANNEL, channelName);
        return;
    }

    if (channel->isInvitedOnly() && !channel->isOperator(client.get())) {
        sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName);
        return;
    }

    std::shared_ptr<Client> targetClient = getClientByNick(targetNick);
    if (!targetClient) {
        sendNumericReply(client, ERR_NOSUCHNICK, targetNick);
        return;
    }

    if (channel->isMember(targetClient.get())) {
        sendNumericReply(client, ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
        return;
    }

    channel->addInvited(targetClient.get());

    std::string inviteMsg = formatClientAction(client, "INVITE", targetNick + " :" + channelName);
    queueMessage(targetClient, inviteMsg);
    sendNumericReply(client, RPL_INVITING, targetNick + " " + channelName);
}
