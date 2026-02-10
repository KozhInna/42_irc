#pragma once

#include "Client.hpp"
#include <string>
#include <list>

class Channel {
	public:
		Channel(const std::string& channelName, Client* client);
		~Channel();

		void setChannelName(const std::string& channelName) { channelName_ = channelName; };
		void setInvitationFlag(bool inviteOnly) { inviteOnly_ = inviteOnly; };
		void setRestrictionFlag(bool restrictedTopic) { restrictedTopic_ = restrictedTopic; };
		void setHasKey(bool hasPassword) { hasPassword_ = hasPassword; };
		void setKey(const std::string& password) { password_ = password; };
		void setLimitClients(bool limitClients) { limitClients_ = limitClients; };
		void setMaxClients(size_t maxClients) { maxClients_ = maxClients; };

		bool isInvitedOnly() const { return inviteOnly_; };
		bool isRestricted() const { return restrictedTopic_; };
		bool hasKey() const { return hasPassword_; };
		bool hasLimit() const { return limitClients_; };

		const std::string& getChannelName() { return channelName_; };
		const std::string& getTopic() { return topic_; };
		const std::string& getKey() { return password_; };
		size_t getMaxClients() const { return maxClients_; };

		// so i need to check if somebody is invited, add somebody to be invited,
		// and remove from invited?
		bool isMember(Client* client);
		void addMember(Client* client);
		void removeMember(Client* client);

		bool isOperator(Client* client);
		void addOperator(Client* client);
		void removeOperator(Client* client);

		bool isInvited(Client* client);
		void addInvited(Client* client);
		void removeInvited(Client* client);

	
	private:
		std::string channelName_;
		std::string topic_;
		std::string password_;
		bool inviteOnly_;
		bool restrictedTopic_;
		bool hasPassword_;
		bool limitClients_;
		size_t maxClients_; // max amount of users
		
		std::list<Client*> members_;
		std::list<Client*> operators_;
		std::list<Client*> invited_;
		// i dont know if there are all necessary, we can remove or add as we go
};