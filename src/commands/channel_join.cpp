#include "Channel.hpp"
#include "Server.hpp"
#include <string>
#include <iostream>
#include <sys/socket.h>


// so first we handle just joining a one channel, optionally with password
// also we handle join 0
//TODO: fix parsing to allow only "JOIN #chan1 pass" 
//TODO: (now i can enter smth like "JOIN JOIN JOIN JOIN" or "JOIN #cnan1 #cnan1")
//TODO: validate key if provided (If the value is invalid, they SHOULD return ERR_INVALIDMODEPARAM)
//TODO: handle error - no channel name (JOIN)
ParsedJoin Server::parseJoin(std::shared_ptr<Client>& client, const ParsedCommand& c) {
	ParsedJoin p {.channel = "", .password = ""};
	size_t pos = 0;

	// if no parameters then cancel
	if (c.params.empty()) {
		sendNumericReply(client, ERR_NEEDMOREPARAMS, c.cmd);
		return p;
	}

	p.channel = c.params[0];

	// trump everything after ','
	if ((pos = p.channel.find(',')) != std::string::npos) {
		p.channel = p.channel.substr(0, pos);
	}

	// channe must start with '#' and must have something after '#'
	if (p.channel.empty() || p.channel[0] != '#' || p.channel.length() == 1) {
		sendNumericReply(client, ERR_NOSUCHCHANNEL, c.cmd);
		p.channel = "";
		return p;
	}
	
	// we return channel and password, the password maybe empty
	if (c.params.size() >= 2)
		p.password = c.params[1];

	// trump everything after ','
	if ((pos = p.password.find(',')) != std::string::npos) {
		p.password = p.password.substr(0, pos);
	}

	return p;
}

void Server::handleJoin(std::shared_ptr<Client>& client, const ParsedCommand& c) {
	ParsedJoin p = parseJoin(client, c);

	if (p.channel.empty())
		return ;

    if (!createChannel(p, client)) {
        std::shared_ptr<Channel> channel = findChannel(p.channel);
        Client *client_ptr = client.get();

        if (channel->isMember(client_ptr)) {
            return ;
        }

        if (channel->isInvitedOnly() && !channel->isInvited(client_ptr)) {
                sendNumericReply(client, ERR_INVITEONLYCHAN, channel->getChannelName());
                return ;
            }
        if (channel->hasPassword() && channel->getPassword() != p.password) {
                sendNumericReply(client, ERR_BADCHANNELKEY, channel->getChannelName());
                return ;
        }
        if (channel->hasLimit() && channel->getNumberMembers() >= channel->getMaxMembers()) {
            sendNumericReply(client, ERR_CHANNELISFULL, channel->getChannelName());
            return ;
        }
        channel->addMember(client_ptr);
        if (channel->isInvited(client_ptr)) {
            channel->removeInvited(client_ptr);
        }
        joinReplyMessages(channel, client);
    }
}

std::shared_ptr<Channel> Server::findChannel(const std::string& channelName) {
    auto it = channels_.find(channelName);
    return (it != channels_.end()) ? it->second : nullptr;
}

bool Server::createChannel(const ParsedJoin& joinCmd, std::shared_ptr<Client>& client) {
    const std::string channelName = joinCmd.channel;
    const std::string chanPassword = joinCmd.password;

    if (!findChannel(channelName)) {
        //create channel
        std::shared_ptr<Channel> channel = std::make_shared<Channel>(channelName);

        channels_.insert({channelName, channel});

        //add client (.get() return raw pointer, out of shared ptr)
        channel->addMember(client.get());
        channel->addOperator(client.get());

        if (!chanPassword.empty()) {
            channel->setPassword(chanPassword);
            channel->setHasPassword(true);
        }
        joinReplyMessages(channel, client);
        return true;
    } else {
        return false;
    }
}

void Server::joinReplyMessages(std::shared_ptr<Channel>& channel, std::shared_ptr<Client>& client) {
    const std::string channelName = channel->getChannelName();

    // 1. JOIN echo to new user and all members of channel
    std::string reply = formatClientAction(client, "JOIN", ":" + channelName);
    channel->broadcastToChannel(reply);

    // 2. Topic (if exists)
    if (!channel->getTopic().empty()) {
        std::string topicExtra = channelName + " :" + channel->getTopic();
        sendNumericReply(client, 332, topicExtra);
    }

    // 3. Nicknames of users list
    std::string users = channel->getMembersList();
    std::string extra = "= " + channelName + " :" + users;
    sendNumericReply(client, RPL_NAMREPLY, extra);

    // 4. End of names
    extra = channelName + " :End of /NAMES list";
    sendNumericReply(client, RPL_ENDOFNAMES, extra);
}

std::string Server::formatClientAction(std::shared_ptr<Client>& client, const std::string& command,
    const std::string& params) {
    std::string prefix = ":" + client->getNickName() + "!" + 
                        client->getUserName() + "@" + 
                        client->getHostName();

    return prefix + " " + command + " " + params + "\r\n";
}