#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <iterator>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>



ParsedCommand Server::parseMessage(const std::string& msg) {
	ParsedCommand c;
	std::string s = msg;
	std::string temp;
	size_t pos;

	// command
	if ((pos = s.find(' ')) != std::string::npos) {
		c.cmd = s.substr(0, pos);
		s.erase(0, pos + 1);
	}

	// first check if there is a ":" and save that to temp
	if ((pos = s.find(':')) != std::string::npos) {
		temp = s.substr(pos, s.substr(pos, s.length()).length());
		s.erase(pos, temp.length());
		temp.erase(0, 1); // removes the ":"
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
	if (c.cmd == "USER")
		std::cout << "User command found!" << std::endl;
	else if (c.cmd == "NICK")
		std::cout << "NICK command found!" << std::endl;
	else if (c.cmd == "PASS") {
		sendReply(client, ERR_UNKNOWNCOMMAND, c.cmd);
		std::cout << "PASS command found!" << std::endl;
	}
	else
		std::cout << "No known command found. :(" << std::endl;
}
