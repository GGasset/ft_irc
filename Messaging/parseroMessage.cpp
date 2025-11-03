/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseroMessage.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alvaro <alvaro@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 19:24:23 by alvmoral          #+#    #+#             */
/*   Updated: 2025/11/03 23:44:32 by alvaro           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

enum ParseStatus {
    VALID_MSG,
	PERR_MSG_LENGTH,
	PERR_NO_CRLF,
	PERR_NO_SPACE_AFTER_PREFIX,
	PERR_PREFIX_LENGTH,
	PERR_PREFIX_MISSING_NICK,
	PERR_PREFIX_MISSING_USER,
	PERR_PREFIX_MISSING_HOST,
	PERR_INVALID_COMMAND,
	PERR_NUMERIC_COMMAND_TOO_LONG,
	PERR_INVALID_CHARACTERS,
	PERR_EMPTY_SPACE,
	PERR_EXCEED_15_PARAM,
	PERR_NONE // must always be the last one
};

const std::string g_parseErrors[PERR_NONE] = {
    "Valid message",
	"Incorrect message length",
	"Message does not end with CRLF",
	"No space after prefix",
	"Invalid prefix length",
	"Missing nickname in prefix",
	"Missing user in prefix",
	"Missing host in prefix",
	"Invalid command",
	"Numeric command has more than three digits",
	"Invalid characters in nospcrlfcl range",
	"Empty space detected",
	"Number of paramenters exceeded 15."
};

COMMAND getCMD(const std::string &cmd) {
    if (cmd == "NICK")
        return NICK;
    else if (cmd == "USER")
        return USER;
    else if (cmd == "PING")
        return PING;
    else if (cmd == "PONG")
        return PONG;
    else if (cmd == "PASS")
        return PASS;
    else if (cmd == "QUIT")
        return QUIT;
    else if (cmd == "JOIN")
        return JOIN;
    else if (cmd == "PART")
        return PART;
    else if (cmd == "PRIVMSG")
        return PRIVMSG;
    else if (cmd == "MODE")
        return MODE;
    else if (cmd == "TOPIC")
        return TOPIC;
    else if (cmd == "INVITE")
        return INVITE;
    else if (cmd == "KICK")
        return KICK;
    else if (cmd == "NOTICE")
        return NOTICE;
    else if (cmd == "NAMES")
        return NAMES;
	else
		return COMMAND0;
}

ParseStatus	checkPrefix(const msgTokens &tokens, size_t &i) {
	std::string prefix = tokens[i++].str;
	if (i == tokens.size() || tokens[i].type != PREFIX)
		return (VALID_MSG);
	if (prefix.length() > 510)
		return (PERR_PREFIX_LENGTH);
	
	size_t	delUser = prefix.find("!");
	size_t	delHost = prefix.find("@");
	
	if (delUser != NPOS || delHost != NPOS)
	{
		if (!delUser)
			return (PERR_PREFIX_MISSING_NICK);
		if (delHost == NPOS || delHost == prefix.size() - 1)
			return (PERR_PREFIX_MISSING_HOST);
		if (delUser == NPOS || delHost - delUser == 1)
			return (PERR_PREFIX_MISSING_USER);
		if (tokens[i++].type != SPACE)
			return (PERR_NO_SPACE_AFTER_PREFIX);
	}
	return (VALID_MSG);
}

ParseStatus	checkCommand(MessageIn &ret, const msgTokens &tokens, size_t &i) {
	msg_token	command = tokens[i++];
	COMMAND		ret_cmd = getCMD(command.str);
	
	if (command.type == WORD
		&& (command.str.length() > 12
		|| ret_cmd == COMMAND0))
		return (PERR_INVALID_COMMAND);
	if (command.type == NUMBER
		&& command.str.length() > 3)
		return (PERR_NUMERIC_COMMAND_TOO_LONG);
	ret.cmd = ret_cmd;
	return (VALID_MSG);
}

bool isInNospcrlfcl(const std::string& str)
{
	for (unsigned char c : str)
	{
		// Excluded characters
		if (c == 0x00 || c == 0x0A || c == 0x0D || c == 0x20 || c == 0x3A)
			return false;
	}
	return true;
}


ParseStatus	checkParams(const msgTokens &tokens, size_t &i) {
	size_t	param_c = 0;

	while (i < tokens.size()) {
		if (param_c == 15)
			return (PERR_EXCEED_15_PARAM);
		i++; //Esto para pasar los SPACE.
		if (!isInNospcrlfcl(tokens[i].str))
			return (PERR_INVALID_CHARACTERS);
		param_c++;
		i++;
	}
	if (tokens[i++].type == SPACE)
		return (PERR_EMPTY_SPACE);
	return (VALID_MSG);
}

MessageIn   parseMessage(msgTokens tokens, ParseStatus &status) {
    MessageIn   ret = {.tokens = tokens, .cmd = COMMAND0};
	size_t		i = 0;

	// status = checkLenMsg(tokens, status);
	if (status != VALID_MSG) return (ret);
	status = checkPrefix(tokens, i);
	if (status != VALID_MSG) return (ret);
	checkCommand(ret, tokens, i);
	if (status != VALID_MSG) return (ret);
	checkParams(tokens, i);
	return (ret);
}