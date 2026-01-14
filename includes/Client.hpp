#pragma once

#include <string>

class Client {
    public:
        Client(int fd);
        ~Client();

        int getFd() const;
        bool getAuthPassed() const;
        bool getRegistered() const;

    private:
        int fd_;
        std::string nickName_;
        std::string userName_;
        std::string realName_;
        bool authPassed_;
        bool registered_;
        std::string buffer_;
};