#include "Channel.hpp"
#include "Server.hpp"
#include <iostream>

// so first we handle just joining a one channel, optionally with password
// also we handle join 0
ParsedJoin Server::handleJoin(std::shared_ptr<Client>& client, const ParsedCommand& c) {
	ParsedJoin p {.channel = "", .password = ""};

	// if no parameters then cancel
	if (c.params.empty()) {
		sendReply(client, ERR_NEEDMOREPARAMS, c.cmd);
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