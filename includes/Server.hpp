#pragma once

#include "Client.hpp"
#include "Channel.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory> //for smart pointers
#include <poll.h>

#define MAX_CLIENTS 128
#define SERVER_VERSION "1.0"

// numeric reply
enum NumericReply {
	// Success replies
    RPL_WELCOME = 001,
    RPL_YOURHOST = 002,
    RPL_CREATED = 003,
    RPL_MYINFO = 004,
    RPL_ISUPPORT = 005,

    // Error replies
    ERR_NOSUCHNICK = 401,
    ERR_NOSUCHCHANNEL = 403,
    ERR_CANNOTSENDTOCHAN = 404,
    ERR_NOMOTD = 422,
    ERR_UNKNOWNCOMMAND = 421,
    ERR_NONICKNAMEGIVEN = 431,
    ERR_ERRONEUSNICKNAME = 432,
    ERR_NICKNAMEINUSE = 433,
    ERR_USERNOTINCHANNEL = 441,
    ERR_NOTONCHANNEL = 442,
    ERR_USERONCHANNEL = 443,
	ERR_NOTREGISTERED = 451,
    ERR_NEEDMOREPARAMS = 461, //Include command name
    ERR_ALREADYREGISTERED = 462,
    ERR_PASSWDMISMATCH = 464,
    ERR_CHANNELISFULL = 471,
    ERR_INVITEONLYCHAN = 473,
    ERR_BADCHANNELKEY = 475,
    ERR_CHANOPRIVSNEEDED = 482,

    //Channel replies
    RPL_NOTOPIC = 331,
    RPL_TOPIC = 332,
    RPL_TOPICWHOTIME = 333,
    RPL_INVITING = 341
};

struct ParsedCommand
{
	std::string cmd;
	std::vector<std::string> params;
};

struct ParsedJoin
{
	std::string channel;
	std::string password;
};

class Server
{
	public:
		Server(int port, const std::string& password);
		~Server();

		/**
		 * @brief Starts the IRC server by creating, binding, and listening on a socket
		 *
		 * Creates a TCP/IPv4 socket, binds it to port_ on all network interfaces (0.0.0.0),
		 * and sets it to listen for incoming client connections.
		 *
		 * @throws std::runtime_error on socket(), bind(), or listen() failure
		 *
		 * @note - socket(): Creates TCP/IPv4 endpoint (AF_INET, SOCK_STREAM)
		 * @note - bind():   Associates socket with 0.0.0.0:port_ (all interfaces)
		 * @note - listen(): Marks socket as passive with backlog queue of 10
		*/
		void start();
		void run();

	private:
		//server data
		int listenFd_;
		int port_;
		std::string password_;
		const std::string serverName_; //"irc.server.com"
        const std::string creationDate_;
		std::unordered_map<int, std::shared_ptr<Client>> clients_;
		std::unordered_map<std::string, std::shared_ptr<Channel>> channels_;

		//related to poll()
		struct pollfd pollFds[MAX_CLIENTS];
		int nPollFds;

        //server setting
        const std::string getCreationDate() const;

		//reply messages map
		std::unordered_map<int, std::string> replyMessages_;

		//methods - connection handling
		std::shared_ptr<Client> acceptNewClient();
        void disconnectClient(int fd);

		// methods - message processing
		void readMsgFromClient(std::shared_ptr<Client> client);
		void processMessage(std::shared_ptr<Client>& client, const std::string& msg);
		ParsedCommand parseMessage(const std::string& msg);
        void sendWelcome(std::shared_ptr<Client>& client);

		//methods - commands handling
		void checkCommand(std::shared_ptr<Client>& client, const ParsedCommand& c);
		void handlePass(std::shared_ptr<Client>& client, const ParsedCommand& c);
		void handleNick(std::shared_ptr<Client>& client, const ParsedCommand& c);
		void handleUser(std::shared_ptr<Client>& client, const std::string& username, const std::string& realname, const size_t param_size);
        void handleQuit(std::shared_ptr<Client>& client, const ParsedCommand& c);
        bool isValidNickname(const std::string& nick);
        bool requiresRegistration(const std::string& cmd);
		ParsedJoin parseJoin(std::shared_ptr<Client>& client, const ParsedCommand& c);
        
        //JOIN cmd methods
		void handleJoin(std::shared_ptr<Client>& client, const ParsedCommand& c);
        bool createChannel(const ParsedJoin& joinCmd, std::shared_ptr<Client>& client);
        void joinReplyMessages(std::shared_ptr<Channel>& channel, std::shared_ptr<Client>& client);
        std::shared_ptr<Channel> findChannel(const std::string& channelName);
        std::string formatClientAction(std::shared_ptr<Client>& client, const std::string& command, const std::string& params);
        void broadcastQuitAndCleanup(Client* client_ptr, const std::string& quitReason);

        //TOPIC cmd methods
        void handleTopic(std::shared_ptr<Client>& client, const ParsedCommand& c);

        //PRIVMSG
        void handlePrivmsg(std::shared_ptr<Client>& client, const ParsedCommand& c);
        std::shared_ptr<Client> getClientByNick(const std::string& nick);

        //KICK
        void handleKick(std::shared_ptr<Client>& client, const ParsedCommand& c);

        //PART
        void handlePart(std::shared_ptr<Client>& client, const ParsedCommand& c);

        //INVITE
        void handleInvite(std::shared_ptr<Client>& client, const ParsedCommand& c);

		// methods - Reply/Error handling
		void initReplyMessages();
		std::string formatNumericReply(int code, const std::string& target, const std::string& param = "");
		void sendNumericReply(std::shared_ptr<Client>& client, int code, const std::string& param = "");
        void handleCap(std::shared_ptr<Client>& client, const ParsedCommand& c);
        void handlePing(std::shared_ptr<Client>& client, const ParsedCommand& c);
};