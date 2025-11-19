/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parserMessage.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alvaro <alvaro@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 19:24:23 by alvmoral          #+#    #+#             */
/*   Updated: 2025/11/19 19:31:31 by alvaro           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

const std::string g_parseErrors[PERR_NONE] = {
    "Valid message",
	"Incorrect message length",
	"Message does not end with CRLF",
	"No space after prefix",
	"Invalid prefix length",
	"Missing nickname in prefix",
	"Missing user in prefix",
	"Missing host in prefix",
	"Invalid nick name in prefix",
	"Invalid user name in prefix",
	"Invalid host name in prefix",
	"Invalid server name in prefix",
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

// RFC 1035: 63 chars per label, 255 total max
bool isValidServerName(const std::string& name)
{
    if (name.empty() || name.size() > 255)
        return false;

    size_t label_len = 0;

    for (size_t i = 0; i < name.size(); ++i)
    {
        char c = name[i];
        // Solo caracteres válidos: letras, dígitos, '-' o '.'
        if (!(std::isalnum(c) || c == '-' || c == '.'))
            return false;
        if (c == '.')
        {
            // No puede empezar ni terminar con '.'
            if (i == 0 || i == name.size() - 1)
                return false;
            // No puede tener dos '.' consecutivos
            if (name[i - 1] == '.')
                return false;
            // Cada etiqueta <= 63 caracteres
            if (label_len == 0 || label_len > 63)
                return false;
            label_len = 0; // Reset para el siguiente label
        }
        else
            ++label_len;
    }
    // Última etiqueta no vacía ni mayor de 63 caracteres
    if (label_len == 0 || label_len > 63)
        return false;
    // No puede empezar ni terminar con '-'
    if (name.front() == '-' || name.back() == '-')
        return false;
    return true;
}

// Define sets of restricted characters
#define NOSPCRLFCL      "\r\n\0 :"
#define NOSPCRLFCL_TRAIL "\r\n\0"
#define RESERVED_CHARS  "\r\n\0 @"
#define SPECIAL_CHARS   "[]\\`^{}_"

inline bool isCharInSet(char c, const std::string& set)
{
	return set.find(c) != std::string::npos;
}

bool areAllCharsAllowed(const std::string& str, const std::string& disallowedSet)
{
	for (char c : str)
	{
		if (isCharInSet(c, disallowedSet))
			return false;
	}
	return true;
}

inline bool isSpecialChar(char c)
{
	return isCharInSet(c, SPECIAL_CHARS);
}

inline bool isReservedChar(char c)
{
	return isCharInSet(c, RESERVED_CHARS);
}

inline bool isInNospcrlfcl(const std::string& str)
{
	return areAllCharsAllowed(str, NOSPCRLFCL);
}

inline bool isInNospcrlfclTRAIL(const std::string& str)
{
	return areAllCharsAllowed(str, NOSPCRLFCL_TRAIL);
}


bool	isValidNickName(const std::string &nickname) {
	if (nickname.length() > 9)
		return (false);
	if (!isalpha(nickname[0])
		&& !isSpecialChar(nickname[0]))
		return (false);
	for (size_t i = 0; i < nickname.length(); i++) {
		if (!isalnum(nickname[i])
		&& !isSpecialChar(nickname[i])
		&& nickname[i] != '-')
		return (false);
	}
	return (true);
}

bool	isValidUserName(const std::string &username) {
	for (size_t i = 0; i < username.length(); i++) {
		if (isReservedChar(username[i]))
			return (false);
	}
	return (true);
}

bool	isValidHostName(const std::string &hostaname) {
	return (isValidServerName(hostaname));
}

ParseStatus	checkPrefix(const msgTokens &tokens, size_t &i) {
	if (i == tokens.size() || tokens[i].type != PREFIX)
		return (VALID_MSG);
	std::string prefix = tokens[i++].str;
	prefix.erase(0, 1);
	if (prefix.length() > 50)
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

		std::string nickname = prefix.substr(0, delUser);
		std::string username = prefix.substr(delUser, delHost - delUser);
		std::string	hostname = prefix.substr(delHost + 1);

		if (!isValidNickName(nickname))
			return (PERR_PREFIX_INVALID_NICK);
		else if (!isValidUserName(username))
			return (PERR_PREFIX_INVALID_USER);
		else if (!isValidHostName(hostname))
			return (PERR_PREFIX_INVALID_HOST);
		return (VALID_MSG);
	}
	//Comprobar el serverName: longitud <= 63, isAlnum || -, no incluye espacios ni comas.
	if (!isValidServerName(prefix))
		return (PERR_PREFIX_INVALID_SERVERNAME);
	i++;
	return (VALID_MSG);
}

ParseStatus	checkCommand(MessageIn &ret, const msgTokens &tokens, size_t &i) {
	msg_token	command = tokens[i++];
	COMMAND		ret_cmd = getCMD(command.str);

	// std::cout << "comando: " << command.str << "--" << std::endl;
	// if (command.type == SPACE)
	// 	return (PERR_INVALID_COMMAND);
	if (command.type == WORD
		&& (command.str.length() > 12
		|| ret_cmd == COMMAND0))
		return (PERR_INVALID_COMMAND);
	else if (command.type == NUMBER
		&& command.str.length() > 3)
		return (PERR_NUMERIC_COMMAND_TOO_LONG);
	else if (command.type != WORD
			&& command.type != NUMBER
			&& command.type != CRLF)
		return (PERR_MISSING_COMMAND);
	ret.setCommand(ret_cmd);
	return (VALID_MSG);
}

ParseStatus	checkParams(const msgTokens &tokens, size_t &i) {
	size_t	param_c = 0;

	while (i < tokens.size() && tokens[i].type != CRLF) {
		if (param_c == 15)
			return (PERR_EXCEED_15_PARAM);
		i++; //Esto para pasar los SPACE.
		if (tokens[i].type == CRLF)
			break ;
		if (!isInNospcrlfcl(tokens[i].str)
			|| (tokens[i].type == TRAIL
				&& !isInNospcrlfclTRAIL(tokens[i].str)))
			return (PERR_INVALID_CHARACTERS);
		param_c++;
	}
	if (tokens[i++].type == SPACE)
		return (PERR_EMPTY_SPACE);
	return (VALID_MSG);
}

ParseStatus	checkLenMsg(const msgTokens &tokens) {
	size_t	len = 0;

	for (size_t i = 0; i < tokens.size(); i++)
		len += tokens[i].str.length();
	return (len >= 512 ? PERR_MSG_LENGTH : VALID_MSG);
}

MessageIn   parseMessage(msgTokens tokens, ParseStatus &status) {
	MessageIn	ret;
	
	ret.tokens = tokens;
	size_t		i = 0;

	status = checkLenMsg(tokens);
	if (status != VALID_MSG) return (ret);
	status = checkPrefix(tokens, i);
	if (status != VALID_MSG) return (ret);
	status = checkCommand(ret, tokens, i);
	if (status != VALID_MSG) return (ret);
	status = checkParams(tokens, i);
	return (ret);
}