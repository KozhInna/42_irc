#include "Channel.hpp"
#include "Server.hpp"
#include <iostream>

void Server::handleTopic(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.size() < 1) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "TOPIC");
        return;
    }
    if (c.params.size() > 2) {
        return;
    }

    std::string channelName = c.params[0];
    //check if channel exist
    std::shared_ptr <Channel> channel = findChannel(channelName);
    if (!channel) {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName);
        return;
    }
    
    //check if user is a member
    if (!channel->isMember(client.get())) {
        sendNumericReply(client, ERR_NOTONCHANNEL, channelName);
        return;
    }

    if (c.params.size() == 2) {
        //check permission to change topic if there is restriction
        if (channel->isRestricted() && !channel->isOperator(client.get())) {
            sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName);
            return;
        }
        // Set topic (whether empty string (clear) or new text)
        channel->setTopic(c.params[1]);
        channel->setTopicAuthor(client.get());
        channel->setTopicTimestamp();
        std::string reply = formatClientAction(client, "TOPIC", channelName + " :" + channel->getTopic());
        channel->broadcastToChannel(reply);

    } else { 
        //just check topic
        if (channel->getTopic().empty()) {
            sendNumericReply(client, RPL_NOTOPIC, channel->getChannelName());
        } else {
            std::string extraTopic = channelName + " :" + channel->getTopic();
            sendNumericReply(client, RPL_TOPIC, extraTopic);

            std::string extra = channelName + " " + 
                                channel->getTopicAuthor() + " " + 
                                std::to_string(channel->getTopicTimestamp());
            sendNumericReply(client, RPL_TOPICWHOTIME, extra);
        }
    }
}