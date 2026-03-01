#pragma once

#include "Client.hpp"
#include <string>
#include <list>

class Channel {
	public:
		Channel(const std::string& channelName);
		~Channel();

		void setChannelName(const std::string& channelName) { channelName_ = channelName; };
		void setInvitationFlag(bool inviteOnly) { inviteOnly_ = inviteOnly; };
		void setRestrictionFlag(bool restrictedTopic) { restrictedTopic_ = restrictedTopic; };
		void setHasPassword(bool hasPassword) { hasPassword_ = hasPassword; };
		void setPassword(const std::string& password) { password_ = password; };
		void setLimitMembers(bool limitMembers) { limitMembers_ = limitMembers; };
		void setMaxMembers(size_t maxMembers) { maxMembers_ = maxMembers; };
        void setTopicAuthor(Client* client) { topicAuthor_ = client->getNickName(); };
        void setTopicTimestamp() { topicTimestamp_ = time(0); }
        void setTopic(const std::string& newTopic) { topic_ = newTopic; };


		bool isInvitedOnly() const { return inviteOnly_; };
		bool isRestricted() const { return restrictedTopic_; };
		bool hasPassword() const { return hasPassword_; };
		bool hasLimit() const { return limitMembers_; };

		const std::string& getChannelName() { return channelName_; };
		const std::string& getTopic() { return topic_; };
		const std::string& getPassword() { return password_; };
		size_t getMaxMembers() const { return maxMembers_; };
        std::list<Client*> getMembers() const { return members_; };
        const std::string& getTopicAuthor() const { return topicAuthor_; };
        time_t getTopicTimestamp() const { return topicTimestamp_; };

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

        std::string getMembersList();
        size_t getNumberMembers() { return members_.size(); };
        void broadcastToChannel(const std::string& reply);


	
	private:
		std::string channelName_;
		std::string topic_;
		std::string password_;
		bool inviteOnly_;
		bool restrictedTopic_;
		bool hasPassword_;
		bool limitMembers_;
		size_t maxMembers_; // max amount of users
        std::string topicAuthor_;
        time_t topicTimestamp_;
		
		std::list<Client*> members_;
		std::list<Client*> operators_;
		std::list<Client*> invited_;
		// i dont know if there are all necessary, we can remove or add as we go
};