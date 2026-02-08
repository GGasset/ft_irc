
#include "router.hpp"
#include "Server.hpp"
#include <cstddef>
#include <string>
#include <vector>

router::router()
{
	for (size_t i = 0; i < last_command; i++) fun[i] = 0;

	command_string[PASS] = "PASS";
	fun[PASS] = PASS_fn;

	command_string[NICK] = "NICK";
	fun[NICK] = NICK_fn;

	command_string[USER] = "USER";
	fun[USER] = USER_fn;

	command_string[JOIN] = "JOIN";
	fun[JOIN] = JOIN_fn;

	// command_string[NAMES] = "NAMES";
	// fun[NAMES] = NAMES_fn;

	command_string[PRIVMSG] = "PRIVMSG";
	fun[PRIVMSG] = PRIVMSG_fn;

	command_string[PING] = "PING";
	fun[PING] = PING_fn;

	command_string[PONG] = "PONG";
	fun[PONG] = PONG_fn;

	command_string[QUIT] = "QUIT";
	fun[QUIT] = QUIT_fn;

	command_string[KICK] = "KICK";
	fun[KICK] = KICK_fn;

	command_string[INVITE] = "INVITE";
	fun[INVITE] = INVITE_fn;

	command_string[TOPIC] = "TOPIC";
	fun[TOPIC] = TOPIC_fn;

	command_string[MODE] = "MODE";
	fun[MODE] = MODE_fn;

	command_string[HELP] = "HELP";
	fun[HELP] = HELP_fn;
}

static std::vector<std::string> split(std::string in, char splitter)
{
	size_t i = 0;
	std::vector<std::string> out;
	while (i < in.size())
	{
		std::string to_add;
		while (in[i] != splitter && i < in.size())
		{
			to_add += in[i];
			i++;
		}

		while (in[i] == splitter && i < in.size()) i++;

		if (to_add.size()) out.push_back(to_add);
	}
	return out;
}

void router::operator()(std::string message, Server& server, User &sender)
{
	if (message.size() > 512) return;
	std::vector<std::string> argv = split(sanitize(message), ' ');
	if (!argv.size()) return;

	command_args args;
	for (size_t i = 0; i < sizeof(args); i++) ((char*)&args)[i] = 0;

	bool contains_prefix = argv[0][0] == ':';
	if (contains_prefix)
	{
		args.prefix = argv[0];
		argv.erase(argv.begin());
	}

	size_t func_i = (size_t)-1;
	for (size_t i = 0; i < last_command && func_i == (size_t)-1; i++)
		if (argv[0] == command_string[i]) func_i = i;


	if (func_i == (size_t)-1 || !fun[func_i])
	{
		server.add_msg("NOTICE " + sender.get_nick() + " 127 command not found.", sender);
		return;
	}
	if (!sender.passwd_match_pop(0) && func_i != PASS && func_i != QUIT && func_i != HELP && func_i != PONG)
	{
		server.add_msg(server.get_prefix() + " 451 :You have not registered", sender);
		server.add_msg("NOTICE " + sender.get_nick() + " you must use PASS command first", sender);
		return;
	}
	if (sender.passwd_match_pop(0) && !sender.is_registered() && func_i != NICK && func_i != USER && func_i != QUIT && func_i != HELP && func_i != PONG)
	{
		server.add_msg(server.get_prefix() + " 451 :You have not registered", sender);
		server.add_msg("NOTICE " + sender.get_nick() + " you must use NICK and USER commands first", sender);
		return;
	}

	for (size_t i = 1; i < argv.size(); i++) args.raw_args += " " + sanitize(argv[i]);

	if (!args.raw_args.empty()) args.raw_args.erase(args.raw_args.begin());

	args.argv = argv;

	//std::cout << "Called parsing function " << command_string[func_i] << std::endl;
	//std::cout << "args:\nPrefix: " << args.prefix << "\nRaw args: " << args.raw_args << std::endl << "Argc: " << args.argv.size() << std::endl << "Args: " << std::endl;
	//for (size_t i = 0; i < args.argv.size(); i++) std::cout << i << ": " << args.argv[i] << std::endl;

	if (fun[func_i]) fun[func_i](args, server, sender);
}
