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
#include <iomanip>

//constructor
Server::Server(int port, const std::string& password)
	:   listenFd_(-1),
		port_(port),
		password_(password),
		serverName_("irc.42.com"),
		nPollFds(0)
{
	//std::cout << "Server constructor\n" << std::endl;
	initReplyMessages();
}

//destructor
Server::~Server()
{
	//std::cout << "Server destructor\n" << std::endl;

	//close listenning socket
	if (listenFd_ != -1) {
		close(listenFd_);
		listenFd_ = -1;
	}

	//close all open sockets
	for (auto& it : clients_) {
		close(it.first);
	}
	clients_.clear();
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
			} else if (pollFds[i].revents & (POLLHUP | POLLERR)) {
				std::cerr << "Client socket error (POLLHUP/POLLERR)" << std::endl;
				disconnectClient(pollFds[i].fd);
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
	checkCommand(client, c);
//	std::cout << "command: " << c.cmd << " " << c.cmd.length() << std::endl;
//	for (auto i : c.params)
//		std::cout << "vector strings: " << i << " " << i.length() << std::endl;
	(void)client;
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
		std::cout << "Client disconnected gracefully" << std::endl;
		disconnectClient(client->getFd());
		//handle client disconnected
	} else if (bytesRead == -1) {
		std::cerr << "Error: recv() failed: " << strerror(errno) << std::endl;
		disconnectClient(client->getFd());
		//handle client disconnected
	}
}

void Server::disconnectClient(int fd) {
	close(fd);
	clients_.erase(fd);

	for (int i = 1; i < nPollFds; i++) {
		if (pollFds[i].fd == fd) {
			if (i != nPollFds - 1) {
				pollFds[i] = pollFds[nPollFds - 1];
			}
			nPollFds--;
		}
	}
}

void Server::initReplyMessages() {
	replyMessages_[RPL_WELCOME] = ":Welcome to the Internet Relay Network";

	replyMessages_[ERR_UNKNOWNCOMMAND] = ":Unknown command";
	replyMessages_[ERR_NONICKNAMEGIVEN] = ":No nickname given";
	replyMessages_[ERR_ERRONEUSNICKNAME] = ":Erroneous nickname";
	replyMessages_[ERR_NICKNAMEINUSE] = ":Nickname is already in use";
	replyMessages_[ERR_NEEDMOREPARAMS] = ":Not enough parameters";
	replyMessages_[ERR_ALREADYREGISTRED] = ":You may not reregister";
	replyMessages_[ERR_PASSWDMISMATCH] = ":Password incorrect";
	replyMessages_[ERR_NOTREGISTERED] = ":You have not registered";
}

std::string Server::formatReply(int code, const std::string& target, const std::string& param) {

	std::stringstream ss;

	// :<server> <code> <target> [param] <message>\r\n
	ss << ":" << serverName_ << " ";
	ss << std::setw(3) << std::setfill('0') << code << " ";
	ss << (target.empty() ? "*" : target) << " ";

	if (!param.empty()) {
		ss << param << " ";
	}

	ss << replyMessages_[code] << "\r\n";
	return ss.str();
}

void Server::sendReply(std::shared_ptr<Client>& client, int code, const std::string& param) {
	std::string target = client->getNickName();
	if (target.empty()) {
		target = "*";
	}

	std::string reply = formatReply(code, target, param);
	send(client->getFd(), reply.c_str(), reply.length(), 0);
}