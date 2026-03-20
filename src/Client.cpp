#include "Client.hpp"
#include <ctime>

//constructor
Client::Client(int fd, const std::string& host)
	:	fd_(fd),
		hostname_(host),
		passAccepted_(false),
		registered_(false),
        lastActivity_(time(nullptr)),
        pingSent_(false),
        connectedAt_(time(nullptr)) {}

//destructor
Client::~Client() {}

int Client::getFd() const {
    return fd_;
}

bool Client::getPassAccepted() const {
    return passAccepted_;
}

bool Client::getRegistered() const {
    return registered_;
}

std::string& Client::getBuffer() {
    return buffer_;
}