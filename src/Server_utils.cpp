#include "Server.hpp"
#include <iostream>

// TODO: check what includes are actually needed

ParsedCommand Server::parseMessage(const std::string& msg) {
	ParsedCommand c;
	std::string s = msg;
	std::string temp;
	size_t pos;

	// first check if there is a ":" and save that to temp
	if ((pos = s.find(':')) != std::string::npos) {
		temp = s.substr(pos, s.substr(pos, s.length()).length());
		s.erase(pos, temp.length());
		temp.erase(0, 1); // removes the ":"
	}

	// command
	if ((pos = s.find(' ')) != std::string::npos) {
		c.cmd = s.substr(0, pos);
		for (size_t i = 0; i < c.cmd.length(); ++i) // change cmd to uppercase
			c.cmd[i] = toupper(c.cmd[i]);
		s.erase(0, pos + 1);
	}
	else
	{ // if there is no spaces, it is cmd
		c.cmd = s;
		for (size_t i = 0; i < c.cmd.length(); ++i) // change cmd to uppercase
			c.cmd[i] = toupper(c.cmd[i]);
		if (!temp.empty())
			c.params.push_back(temp);
		return c;
	}
	
	// then split the rest by spaces
	while ((pos = s.find(' ')) != std::string::npos) {
		c.params.push_back(s.substr(0, pos));
		s.erase(0, pos + 1);
	}

	// if there is anything left add that to vector string as well
	if (!s.empty()) {
		if (c.cmd.empty())
			c.cmd = s;
		else
			c.params.push_back(s);
	}

	// add temp if it is not empthy
	if (!temp.empty())
		c.params.push_back(temp);
	return c;
}

void Server::checkCommand(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.cmd == "USER") handleUser(client, c.params[0], c.params[3], c.params.size());
    else if (c.cmd == "NICK") handleNick(client, c);
    else if (c.cmd == "PASS") handlePass(client, c);
    else if (c.cmd == "QUIT") handleQuit(client, c);

    // Commands that require registration
    else if (requiresRegistration(c.cmd)) {
        if (!client->getRegistered()) {
            sendReply(client, ERR_NOTREGISTERED, ""); 
            return;
        }
        // Route to appropriate handler
        if (c.cmd == "JOIN") handleJoin(client, c);
        //else if (c.cmd == "PART") handlePart(client, c);
        //else if (c.cmd == "PRIVMSG") handlePrivmsg(client, c);
        //else if (c.cmd == "KICK") handleKick(client, c);
        //else if (c.cmd == "INVITE") handleInvite(client, c);
        //else if (c.cmd == "TOPIC") handleTopic(client, c);
        //else if (c.cmd == "MODE") handleMode(client, c);
    } else {
        sendReply(client, ERR_UNKNOWNCOMMAND, c.cmd); 
    }
}

bool Server::requiresRegistration(const std::string& cmd) {
    return (cmd == "JOIN" || cmd == "PART" || cmd == "PRIVMSG" || 
            cmd == "KICK" || cmd == "INVITE" || cmd == "TOPIC" || 
            cmd == "MODE");
}
