#pragma once

#include "Client.hpp"
#include <string>
#include <map>

// because one member of channel can hold more than one role
struct Roles {
	bool owner_;
	bool operator_;
	bool moderator_;
	//? do we need more?
};

class Channel {
	public:
		Channel(const std::string& channelName, Client* client);
		~Channel();

		void setChannelName(const std::string& channelName);
		void setInvitationFlag(bool invFlag) { invFlag_ = invFlag; };
		void setRestrictionFlag(bool restrictFlag) { restrictFlag_ = restrictFlag; };
		void setHasKey(bool hasKey) { hasKey_ = hasKey; };
		void setKey(const std::string& key) { key_ = key; };
		void setLimitClients(bool limitClients) { limitClients_ = limitClients; };
		void setMaxClients(size_t maxClients) { maxClients_ = maxClients; };


		const std::string& getChannelName();
		

	
	private:
		std::string channelName_;
		std::string topic_;
		std::string key_;
		bool invFlag_;
		bool restrictFlag_;
		bool hasKey_;
		bool limitClients_;
		size_t maxClients_; // max amount of users
		
		std::map<Client*, Roles> members_;
		std::map<Client*, Roles> invited_;
		// i dont know if there are all necessary, we can remove or add as we go
};