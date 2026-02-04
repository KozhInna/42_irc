#pragma once

#include <string>

class Client {
    public:
        Client(int fd);
        ~Client();

        int getFd() const;
        bool getAuthPassed() const;
        bool getRegistered() const;
		void setRegistered(bool v) { registered_ = v; }

		// TODO; move the functions inside the client folders
		const std::string& getNickName() { return nickName_; }
		const std::string& getUserName() { return userName_; }
		const std::string& getRealName() { return realName_; }

		void setNickName(std::string& nick) { nickName_ = nick; }
		void setUserName(std::string& user) { userName_ = user; }
		void setRealName(std::string& real) { realName_ = real; }

        std::string& getBuffer();


    private:
        int fd_;
        std::string nickName_;
        std::string userName_;
        std::string realName_;
        bool authPassed_;
        bool registered_;
        std::string buffer_;
};