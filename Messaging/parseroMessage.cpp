/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseroMessage.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alvmoral <alvmoral@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 19:24:23 by alvmoral          #+#    #+#             */
/*   Updated: 2025/11/03 20:42:50 by alvmoral         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

enum ParseStatus {
    VALID_MSG,
	ERR_MSG_LENGTH,
	ERR_NO_COMMAND,
	ERR_NO_CRLF,
	ERR_NO_SPACE_AFTER_PREFIX,
	ERR_PREFIX_LENGTH,
	ERR_PREFIX_MISSING_NICK,
	ERR_PREFIX_MISSING_USER,
	ERR_PREFIX_MISSING_HOST,
	ERR_INVALID_COMMAND,
	ERR_NUMERIC_COMMAND_TOO_LONG,
	ERR_INVALID_CHARACTERS,
	ERR_EMPTY_SPACE,
	ERR_NONE // must always be the last one
};

const std::string g_parseErrors[ERR_NONE] = {
    "Valid message",
	"Incorrect message length",
	"Command token missing",
	"Message does not end with CRLF",
	"No space after prefix",
	"Invalid prefix length",
	"Missing nickname in prefix",
	"Missing user in prefix",
	"Missing host in prefix",
	"Invalid command",
	"Numeric command has more than three digits",
	"Invalid characters in nospcrlfcl range",
	"Empty space detected"
};

COMMAND getCMD(msg_token cmd) {
    // if (cmd.str == "JOIN")
    //     return (JOIN);
    // ... Y así con todos ...
}

MessageIn   parseMessage(msgTokens tokens, ParseStatus &status) {
    MessageIn   ret;
    ret.tokens = tokens;
    // ret.cmd = getCMD();
    
}