#include "Channel.hpp"

//constructor
Channel::Channel(const std::string& channelName) 
    :   channelName_(channelName),
        inviteOnly_(false),
        restrictedTopic_(false),
        hasPassword_(false),
        limitMembers_(false),
        maxMembers_(0)
{

}

//destructor
Channel::~Channel() {}


bool Channel::isMember(Client* client) {
	for (Client* c : members_) {
		if (client == c)
			return true;
	}
	return false;
}

void Channel::addMember(Client* client) {
	members_.push_back(client);
}

void Channel::removeMember(Client* client) {
	members_.remove(client);
}

bool Channel::isOperator(Client* client) {
	for (Client* o : operators_) {
		if (client == o)
			return true;
	}
	return false;
}

void Channel::addOperator(Client* client) {
	operators_.push_back(client);
}

void Channel::removeOperator(Client* client) {
	operators_.remove(client);
}

bool Channel::isInvited(Client* client) {
	for (Client* i : invited_) {
		if (client == i)
			return true;
	}
	return false;
}

void Channel::addInvited(Client* client) {
	invited_.push_back(client);
}

void Channel::removeInvited(Client* client) {
	invited_.remove(client);
}

std::string Channel::getAllChannelMembers() {
    std::string users;

    for (auto op : operators_) {
        users += "@" + op->getNickName() + " ";
    }

    for (auto user : members_) {
        if (!isOperator(user)) {
            users += user->getNickName() + " ";
        }
    }

    if (!users.empty()) {
        users.pop_back();
    }
    return users;
}