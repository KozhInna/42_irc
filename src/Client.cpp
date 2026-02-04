#include "Client.hpp"

//constructor
Client::Client(int fd) : fd_(fd), authPassed_(false), registered_(false) {}

//destructor
Client::~Client() {}

int Client::getFd() const {
    return fd_;
}

bool Client::getAuthPassed() const {
    return authPassed_;
}

bool Client::getRegistered() const {
    return registered_;
}

std::string& Client::getBuffer() {
    return buffer_;
}