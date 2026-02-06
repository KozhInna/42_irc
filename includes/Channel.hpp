#pragma once

class Channel {
	public:
		Channel(const std::string& channelName);
		~Channel();

		void setChannelName(const std::string& channelName);
		
		const std::string& getChannelName();

		// add member to a list
		// remove member from a list
	
	private:
		std::string channelName_;
		// topic - list of strings
		// invite only flag - bool
		// topic restricted - bool
		// has_key - bool
		// the key - std::string
		// has_limit - bool
		// max_limit_users - int
		// invited users - list
		// list of members - in currently in channel?
		// list of priviledged channel members so moderators
		// list of channel operators
		// list of channel creators
};