#include "Channel.hpp"
#include <string>

//constructor
Channel::Channel(const std::string& channelName) : channelName_(channelName) {}

//destructor
Channel::~Channel() {}


void Channel::setChannelName(const std::string& channelName) {
	channelName_ = channelName;
}


const std::string& Channel::getChannelName() {
	return channelName_;
}