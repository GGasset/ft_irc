
#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "User.hpp"
#include "string"
#include "vector"

#include "Server.hpp"
#include <string>
#include <vector>

enum commands
{
	PASS,
	NICK,
	USER,
	JOIN,
	PRIVMSG,
	PING,
	PONG,
	QUIT,
	KICK,
	INVITE,
	TOPIC,
	MODE,
	HELP,
	last_command
};

// Canales # o &

struct command_args
{
	std::string prefix; // Prefix that the user sent
	std::vector<std::string> argv; // Doesn't contain the prefix. Is a split of spaces of the message that the user sent
	std::string raw_args; // Concatenate of the argvs after COMMAND argv [excluded]
};

class router
{
	std::string command_string[last_command];
	void (*fun[last_command])(command_args argv, Server&, User& sender);

	public:
		router();
		void operator()(std::string message, Server& server, User &sender);
};

void PASS_fn(command_args argv, Server& server, User& sender);
void NICK_fn(command_args argv, Server& server, User& sender);
void USER_fn(command_args args, Server& server, User& sender);
void JOIN_fn(command_args args, Server& server, User& sender);
void PRIVMSG_fn(command_args args, Server& server, User& sender);
void PING_fn(command_args args, Server& server, User& sender);
void PONG_fn(command_args args, Server& server, User& sender);
void QUIT_fn(command_args args, Server& server, User& sender);
void KICK_fn(command_args args, Server& server, User& sender);
void INVITE_fn(command_args args, Server& server, User& sender);
void TOPIC_fn(command_args args, Server& server, User& sender);
void MODE_fn(command_args args, Server& server, User& sender);
void HELP_fn(command_args args, Server& server, User& sender);


#endif
