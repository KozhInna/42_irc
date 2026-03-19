#pragma once

#include <string>

class Client {
    public:
        Client(int fd, const std::string& hostname);
        ~Client();

        int getFd() const;
        bool getPassAccepted() const;
        bool getRegistered() const;
		void setRegistered(bool v) { registered_ = v; }
        void setPassAccepted(bool v) { passAccepted_ = v; }

		// TODO; move the functions inside the client folders
		const std::string& getNickName() const { return nickName_; }
		const std::string& getUserName() const { return userName_; }
		const std::string& getRealName() const { return realName_; }
		const std::string& getHostName() const { return hostname_; }
        bool getPingSent() const { return pingSent_; }
        time_t getLastActivity() const { return lastActivity_; }
        time_t getConnectedAt() const { return connectedAt_; }
               
		void setNickName(const std::string& nick) { nickName_ = nick; }
		void setUserName(const std::string& user) { userName_ = user; }
		void setRealName(const std::string& real) { realName_ = real; }
		void setHostname(const std::string& host) { hostname_ = host; }
        void setPingSent(bool pingSent) { pingSent_ = pingSent; }
        void setLastActivity_(time_t lastActivityTime) { lastActivity_ = lastActivityTime; }

        std::string& getBuffer();


    private:
        int fd_;
        std::string nickName_;
        std::string userName_;
        std::string realName_;
		std::string hostname_;
        bool passAccepted_; // PASS cmd sent successfully
        bool registered_; // PASS + NICK + USER sent
        std::string buffer_;
        time_t lastActivity_;
        bool pingSent_;
        time_t connectedAt_;
};