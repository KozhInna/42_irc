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
#include <sstream>
#include <ctime>

//constructor
Server::Server(int port, const std::string& password)
	:   listenFd_(-1),
		port_(port),
		password_(password),
		serverName_("irc.42.com"),
        creationDate_(getCreationDate()),
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

    // Allow restart without waiting for port to be released
    int opt = 1;
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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
                if (nPollFds >= MAX_CLIENTS) {
                    std::string msg = "ERROR :Server is full\r\n";
                    send(client->getFd(), msg.c_str(), msg.length(), 0);
                    close(client->getFd());
                    clients_.erase(client->getFd());
                } else {
                    pollFds[nPollFds].fd = client->getFd();
                    pollFds[nPollFds].events = POLLIN;
                    nPollFds++;
                }
			}
		}
		for (int i = 1; i < nPollFds; i++) {
			if (pollFds[i].revents & POLLIN) {
				auto it = clients_.find(pollFds[i].fd);
				if (it != clients_.end()){
					bool disconnected = readMsgFromClient(it->second);
                    if (disconnected) i--;
				}
			} else if (pollFds[i].revents & (POLLHUP | POLLERR)) {
                auto it = clients_.find(pollFds[i].fd);
                if (it != clients_.end()) {
                    std::string reply = formatClientAction(it->second, "QUIT", " :Connection lost");
                    broadcastQuitAndCleanup(it->second.get(), reply);
                    disconnectClient(pollFds[i].fd);
                    i--;
                }
            }
		}
	}
}

// Called after poll() signals activity on listenFd_.
// accept() fills clientSock with the client's IP/port and returns a new fd.
// inet_ntop converts the binary IP to a string hostname.
// We store both in a new Client object.
std::shared_ptr<Client> Server::acceptNewClient() {
	struct sockaddr_in clientSock;
	socklen_t clientSockLen = sizeof(clientSock);

	int clientFd = accept(listenFd_, (struct sockaddr *)&clientSock, &clientSockLen);
	if (clientFd == -1) {
		std::cerr << "Error: accept() failed: " << strerror(errno) << std::endl;
		return nullptr;
	}
    
	char hostname[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(clientSock.sin_addr), hostname, INET_ADDRSTRLEN);

	std::shared_ptr<Client> client = std::make_shared<Client>(clientFd, hostname);
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
}

// Reads data from client socket into per-client buffer, then processes complete messages.
// Returns true if the client was disconnected (caller should adjust poll loop index).
bool Server::readMsgFromClient(std::shared_ptr<Client> client) {
	char tempBuf[512];
	ssize_t bytesRead = recv(client->getFd(), tempBuf, sizeof(tempBuf), 0);
	if (bytesRead > 0) {
        client->getBuffer().append(tempBuf, bytesRead);
        // Protection against clients flooding data without newlines (no valid IRC message)
        if (client->getBuffer().size() > 4096) {
            disconnectClient(client->getFd());
            return true;
        }
		size_t pos;
        // Process all complete messages (delimited by \n) from the buffer
		while ((pos = client->getBuffer().find("\n")) != std::string::npos) {
			std::string message = client->getBuffer().substr(0, pos);
			client->getBuffer().erase(0, pos + 1);
			processMessage(client, message);
            // processMessage may trigger QUIT which calls disconnectClient internally
            if (clients_.find(client->getFd()) == clients_.end())
                return true;
        }
	} else if (bytesRead == 0) {
        // TCP FIN received — client closed connection gracefully (Ctrl+D (EOF))
        std::string reply = formatClientAction(client, "QUIT", " :Connection lost");
        broadcastQuitAndCleanup(client.get(), reply);
		disconnectClient(client->getFd());
        return true;
	} else if (bytesRead == -1) {
        // Socket error — treat as disconnect
		std::cerr << "Error: recv() failed: " << strerror(errno) << std::endl;
        std::string reply = formatClientAction(client, "QUIT", " :Connection error");
        broadcastQuitAndCleanup(client.get(), reply);
        disconnectClient(client->getFd());
        return true;
	}
    return false;
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
	replyMessages_[ERR_UNKNOWNCOMMAND] = ":Unknown command";
	replyMessages_[ERR_NONICKNAMEGIVEN] = ":No nickname given";
	replyMessages_[ERR_ERRONEUSNICKNAME] = ":Erroneous nickname";
	replyMessages_[ERR_NICKNAMEINUSE] = ":Nickname is already in use";
	replyMessages_[ERR_NEEDMOREPARAMS] = ":Not enough parameters";
	replyMessages_[ERR_ALREADYREGISTERED] = ":You may not reregister";
	replyMessages_[ERR_PASSWDMISMATCH] = ":Password incorrect";
	replyMessages_[ERR_NOTREGISTERED] = ":You have not registered";
    replyMessages_[ERR_NOMOTD] = ":MOTD File is missing";

        // Channel errors
    replyMessages_[ERR_NOSUCHNICK] = ":No such nick/channel"; // 401
    replyMessages_[ERR_NOSUCHCHANNEL] = ":No such channel"; // 403
    replyMessages_[ERR_CANNOTSENDTOCHAN] = ":Cannot send to channel"; // 404
    replyMessages_[ERR_USERNOTINCHANNEL] = ":They aren't on that channel"; // 441
    replyMessages_[ERR_NOTONCHANNEL] = ":You're not on that channel";  // 442
    replyMessages_[ERR_INVITEONLYCHAN] = ":Cannot join channel (+i)";  // 473
    replyMessages_[ERR_BADCHANNELKEY] = ":Cannot join channel (+k)";   // 475
    replyMessages_[ERR_CHANNELISFULL] = ":Cannot join channel (+l)";   // 471
    replyMessages_[ERR_CHANOPRIVSNEEDED] = ":You're not channel operator"; // 482

    // Channel replies
    replyMessages_[RPL_NOTOPIC] = ":No topic is set";
    
}

std::string Server::formatNumericReply(int code, const std::string& target, const std::string& extra) {

	std::stringstream ss;

	// :<server> <code> <target> [param] <message>\r\n
	ss << ":" << serverName_ << " ";
	ss << std::setw(3) << std::setfill('0') << code << " ";
	ss << (target.empty() ? "*" : target) << " ";

    bool hasStaticMsg = replyMessages_.find(code) != replyMessages_.end();
    if (!extra.empty()) {
        ss << extra;
        if (hasStaticMsg)
            ss << " ";  // ← only add space if static message follows!
    }

	ss << (hasStaticMsg ? replyMessages_[code] : "") << "\r\n";
	return ss.str();
}

void Server::sendNumericReply(std::shared_ptr<Client>& client, int code, const std::string& extra) {
	std::string target = client->getNickName();
	if (target.empty()) {
		target = "*";
	}

	std::string reply = formatNumericReply(code, target, extra);
	send(client->getFd(), reply.c_str(), reply.length(), 0);
}

const std::string Server::getCreationDate() const {
    time_t timestamp;
    time(&timestamp);

    char buf[70];
    strftime(buf, 70, "%a %d %b %Y at %H:%M:%S", localtime(&timestamp));
    return (buf);
}