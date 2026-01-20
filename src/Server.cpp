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

//constructor
Server::Server(int port, const std::string& password)
	:   listenFd_(-1),
		port_(port),
		password_(password),
		nPollFds(0)
{
	//std::cout << "Server constructor\n" << std::endl;
}

//destructor
Server::~Server()
{
	if (listenFd_ != -1) {
		close(listenFd_);
		listenFd_ = -1;
	}
	//std::cout << "Server destructor\n" << std::endl;
}

void Server::start()
{
	// Create TCP/IPv4 socket (UNBOUND: no IP/port yet)
	listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFd_ == -1) {
		throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
	}

	// Bind to all interfaces on specified port (state: BOUND)
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;          // IPv4
	addr.sin_port = htons(port_);       // Network byte order
	addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0 (all interfaces)

	if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		throw std::runtime_error(std::string("bind() failed: ") + strerror(errno));
	}

	// Mark as listening socket (state: LISTENING, backlog: 10)
	if (listen(listenFd_, 10) == -1) {
		throw std::runtime_error(std::string("listen() failed: ") + strerror(errno));
	}
	pollFds[0].fd = listenFd_;
	pollFds[0].events = POLLIN;
	nPollFds = 1;
}

void Server::run() {
	while (true) {
		int res = poll(pollFds, nPollFds, -1);
		if (res == -1) {
			std::cerr << "Error: poll() failed: " << strerror(errno) << std::endl;
			continue;
		}
		if (pollFds[0].revents & POLLIN) {
			std::shared_ptr<Client> client = acceptNewClient();
			if (client) {
				pollFds[nPollFds].fd = client->getFd();
				pollFds[nPollFds].events = POLLIN;
				nPollFds++;
			}
		}
		for (int i = 1; i < nPollFds; i++) {
			if (pollFds[i].revents & POLLIN) {
				auto it = clients_.find(pollFds[i].fd);
				if (it != clients_.end()){
					readMsgFromClient(it->second);
				}
			}
			if (pollFds[i].revents & (POLLHUP | POLLERR)) {
				//handle client disconnected - handleDisconnect(pollFds[i].fd);
			}
		}
	}
}

std::shared_ptr<Client> Server::acceptNewClient() {
	struct sockaddr_in clientSock;
	socklen_t clientSockLen = sizeof(clientSock);

	int clientFd = accept(listenFd_, (struct sockaddr *)&clientSock, &clientSockLen);
	if (clientFd == -1) {
		std::cerr << "Error: accept() failed: " << strerror(errno) << std::endl;
		return nullptr;
	}
	std::shared_ptr<Client> client = std::make_shared<Client>(clientFd);
	clients_.insert({clientFd, client});
	return client;
}

void Server::processMessage(std::shared_ptr<Client>& client, const std::string& msg) {
	std::cout << "Client sent message: " << msg << std::endl;
	// parse the string (msg)
	// returns the found cmd if any
	ParsedCommand c = parseMessage(msg);
	checkCommand(c.cmd);
	std::cout << "command: " << c.cmd << " " << c.cmd.length() << std::endl;
	for (auto i : c.params)
		std::cout << "vector strings: " << i << " " << i.length() << std::endl;
	(void)client;
}

ParsedCommand Server::parseMessage(const std::string& msg) {
	ParsedCommand c;
	std::string s = msg;
	std::string temp;
	size_t pos;
	
	// this doesnt work that, if there is no space and the first command should be
	// added to commands anyway
	// the first word is always command so find it?
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
	if (!s.empty())
		c.params.push_back(s);
	
	// add temp if it is not empthy
	if (!temp.empty())
		c.params.push_back(temp);
	return c;
}

void Server::checkCommand(const std::string& cmd) {
	if (cmd == "USER")
		std::cout << "User command found!" << std::endl;
	else if (cmd == "NICK")
		std::cout << "NICK command found!" << std::endl;
	else if (cmd == "PASS")
		std::cout << "PASS command found!" << std::endl;
	else
		std::cout << "No known command found. :(" << std::endl; 
}

void Server::readMsgFromClient(std::shared_ptr<Client> client) {
	char tempBuf[512];
	ssize_t bytesRead = recv(client->getFd(), tempBuf, sizeof(tempBuf), 0);
	if (bytesRead > 0) {
		client->getBuffer().append(tempBuf, bytesRead);
		size_t pos;
		while ((pos = client->getBuffer().find("\n")) != std::string::npos) {
			std::string message = client->getBuffer().substr(0, pos);
			processMessage(client, message);
			client->getBuffer().erase(0, pos + 1);
		}
	} else if (bytesRead == 0) {
		std::cout << "Client disconnected" << std::endl;
		//handle client disconnected
	} else if (bytesRead == -1) {
		std::cerr << "Error: recv() failed: " << strerror(errno) << std::endl;
		//handle client disconnected
	}

	// clients_.erase(client->getFd());
	// close(client->getFd());
}