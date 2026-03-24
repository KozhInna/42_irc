#include "Channel.hpp"
#include "Server.hpp"

void Server::handleMode(std::shared_ptr<Client>& client, const ParsedCommand& c) {
    if (c.params.empty()) {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "MODE");
        return;
    }
    std::string target = c.params[0];
    if (target[0] != '#') {
        return;
    }

    std::string channelName = c.params[0];
    std::shared_ptr<Channel> channel = findChannel(channelName);
    
    if (!channel) {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName);
        return;
    }

    if (!channel->isMember(client.get())) {
        sendNumericReply(client, ERR_NOTONCHANNEL, channelName);
        return;
    }

    // Query mode: MODE #channel
    if (c.params.size() == 1) {
        std::string modes = "+";
        std::string params;
        
        if (channel->isInvitedOnly()) modes += "i";
        if (channel->isRestricted()) modes += "t";
        if (channel->hasPassword()) {
            modes += "k";
            params += " " + channel->getPassword();
        }
        if (channel->hasLimit()) {
            modes += "l";
            params += " " + std::to_string(channel->getMaxMembers());
        }
        
        std::string reply = channelName + " " + modes + params;
        sendNumericReply(client, RPL_CHANNELMODEIS, reply);
        return;
    }

    // Ban list query: MODE #channel b (allowed for all members)
    if (c.params.size() == 2 && c.params[1] == "b") {
        sendNumericReply(client, RPL_ENDOFBANLIST, channelName + " :End of channel ban list");
        return;
    }

    // Set mode: MODE #channel +/-modes [params]
    if (!channel->isOperator(client.get())) {
        sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName);
        return;
    }

    std::string modeString = c.params[1];
    size_t paramIndex = 2;
    bool adding = true;
    std::string appliedModes;
    std::string appliedParams;
    char lastSign = '\0';
	bool anyModeApplied = false;

    for (size_t i = 0; i < modeString.length(); ++i) {
        char mode = modeString[i];

        if (mode == '+') {
            adding = true;
            continue;
        } else if (mode == '-') {
            adding = false;
            continue;
        }

        char currentSign = adding ? '+' : '-';
        if (currentSign != lastSign) {
            appliedModes += currentSign;
            lastSign = currentSign;
        }

        switch (mode) {
            case 'i': // Invite-only
                channel->setInvitationFlag(adding);
                appliedModes += "i";
				anyModeApplied = true;
                break;

            case 't': // Topic restriction
                channel->setRestrictionFlag(adding);
                appliedModes += "t";
				anyModeApplied = true;
                break;

            case 'k': // Channel key (password)
                if (adding) {
                    if (paramIndex >= c.params.size()) {
                        sendNumericReply(client, ERR_NEEDMOREPARAMS, "MODE");
                        continue;
                    }
                    std::string key = c.params[paramIndex++];
                    channel->setPassword(key);
                    channel->setHasPassword(true);
                    appliedModes += "k";
					anyModeApplied = true;
                    appliedParams += " " + key;
                } else {
                    if (paramIndex < c.params.size())
						paramIndex++; // consume the * argument clients send with -k
					channel->setPassword("");
					channel->setHasPassword(false);
					appliedModes += "k";
					anyModeApplied = true;

                }
                break;

            case 'o': // Operator privilege
                if (paramIndex >= c.params.size()) {
                    sendNumericReply(client, ERR_NEEDMOREPARAMS, "MODE");
                    continue;
                }
                {
                    std::string targetNick = c.params[paramIndex++];
                    std::shared_ptr<Client> targetClient = getClientByNick(targetNick);
                    
                    if (!targetClient) {
                        sendNumericReply(client, ERR_NOSUCHNICK, targetNick);
                        continue;
                    }
                    
                    if (!channel->isMember(targetClient.get())) {
                        sendNumericReply(client, ERR_USERNOTINCHANNEL, targetNick + " " + channelName);
                        continue;
                    }
                    
                    if (adding) {
                        if (!channel->isOperator(targetClient.get())) {
                            channel->addOperator(targetClient.get());
                        }
                    } else {
                        if (channel->isOperator(targetClient.get())) {
                            channel->removeOperator(targetClient.get());
                        }
                    }
                    appliedModes += "o";
					anyModeApplied = true;
                    appliedParams += " " + targetNick;
                }
                break;

            case 'l': // User limit
                if (adding) {
                    if (paramIndex >= c.params.size()) {
                        sendNumericReply(client, ERR_NEEDMOREPARAMS, "MODE");
                        continue;
                    }
                    std::string limitStr = c.params[paramIndex++];
                    try {
                        size_t limit = std::stoul(limitStr);
						if (limit == 0) {
							continue; // treat 0 as invalid
						}
                        channel->setMaxMembers(limit);
                        channel->setLimitMembers(true);
                        appliedModes += "l";
						anyModeApplied = true;
                        appliedParams += " " + limitStr;
                    } catch (...) {
                        // Invalid number, skip
                        continue;
                    }
                } else {
                    channel->setLimitMembers(false);
                    channel->setMaxMembers(0);
                    appliedModes += "l";
					anyModeApplied = true;
                }
                break;

            default:
                sendNumericReply(client, ERR_UNKNOWNMODE, std::string(1, mode));
                continue;
        }
    }

    // Broadcast mode change to channel
    if (anyModeApplied) {
        std::string modeChange = formatClientAction(client, "MODE", channelName + " " + appliedModes + appliedParams);
        broadcastToChannel(channel.get(), modeChange);
    }
}
