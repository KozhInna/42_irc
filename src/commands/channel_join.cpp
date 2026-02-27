#include "Channel.hpp"
#include "Server.hpp"
#include <string>
#include <iostream>
#include <sys/socket.h>


// so first we handle just joining a one channel, optionally with password
// also we handle join 0
//TODO: fix parsing to allow only "JOIN #chan1 pass" 
//TODO: (now i can enter smth like "JOIN JOIN JOIN JOIN" or "JOIN #cnan1 #cnan1")
//TODO: validate key if provided
//TODO: handle error - no channel name (JOIN)
ParsedJoin Server::parseJoin(std::shared_ptr<Client>& client, const ParsedCommand& c) {
	ParsedJoin p {.channel = "", .password = ""};

	// if no parameters then cancel
	if (c.params.empty()) {
		sendNumericReply(client, ERR_NEEDMOREPARAMS, c.cmd);
		return p;
	}

	p.channel = c.params[0];

	// we got 0, and no other params, return
	if (p.channel == "0") {
		// leave all channels
		return p;
	}
	
	// we return channel and password, the password maybe empty
	if (c.params.size() >= 2)
		p.password = c.params[1];
	return p;
}

void Server::handleJoin(std::shared_ptr<Client>& client, const ParsedCommand& c) {
	ParsedJoin p;
	p = parseJoin(client, c);
    std::cout << p.channel << " " << p.password << std::endl;

    createChannel(p, client);
    //if (!createChannel(p, client))
     //search channeal among existed
}

bool Server::createChannel(const ParsedJoin& joinCmd, std::shared_ptr<Client>& client) {
    const std::string channelName = joinCmd.channel;
    const std::string chanPassword = joinCmd.password;

    if (channels_.find(channelName) == channels_.end()) {
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

    // 1. JOIN echo
    std::string reply = formatEchoReply(channelName, client);
    sendEchoReply(reply, client);

    // 2. Topic (if exists)
    if (!channel->getTopic().empty()) {
        std::string topicExtra = channelName + " :" + channel->getTopic();
        sendNumericReply(client, 332, topicExtra);
    }

    // 3. Nicknames of users list (353)
    std::string users = channel->getAllChannelUsers();
    std::string extra = "= " + channelName + " :" + users;
    sendNumericReply(client, 353, extra);

    // 4. End of names
    extra = channelName + " :End of /NAMES list";
    sendNumericReply(client, 366, extra);
}

std::string Server::formatEchoReply(const std::string& channelName, std::shared_ptr<Client>& client) {
    std::string nick = client->getNickName();
    std::string name = client->getUserName();
    std::string host = client->getHostName();

    std::string userPrefix = ":" + nick + "!" + name + "@" + host;
    std::string reply = userPrefix + " JOIN :" + channelName + "\r\n";
    return reply;
}

void Server::sendEchoReply(const std::string& reply, std::shared_ptr<Client>& client) {
    send(client->getFd(), reply.c_str(), reply.length(), 0);
}