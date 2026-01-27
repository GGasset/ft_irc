
#include "Channel.hpp"
#include "router.hpp"
#include <vector>

#define send_back(msg) server.add_msg(server.get_prefix() + " " + args.prefix + " " + msg, sender)
#define notice_back(msg) server.add_msg("NOTICE " + sender.get_nick() + " " + msg, sender)
#define send_return(msg) {send_back(msg); return;}

void PASS_fn(command_args args,  Server& server, User& sender)
{
	if (sender.is_registered()) send_return("462 :You may not reregister");
	if (args.argv.size() <= 1) send_return("461 " + args.argv[0] + " :Not enough parameters");
	if (args.raw_args != server.get_server_password()) send_return("464 :Password incorrect");

	sender.passwd_match_pop(true);
	notice_back("Correct password! Now send the combination of NICK and USER commands");
}

void NICK_fn(command_args args, Server& server, User& sender)
{
	if (args.argv.size() > 3 || args.argv.size() < 2) send_return("431 :No nickname given");
	if (server.get_user_by_nick(args.argv[1])) send_return("433 " + args.argv[1] + " :Nickname is already in use");

	bool had_nick = sender.get_nick().empty() == 0;

	sender.setNick(args.argv[1]);
	notice_back("Nick set to: " + args.argv[1]);
	if (!sender.getUsername().empty() && !had_nick) {sender.register_user(); notice_back("Registered!");}
}

void USER_fn(command_args args, Server& server, User& sender)
{
	if (sender.is_registered()) send_return("462 :You may not reregister");
	if (args.argv.size() != 5) send_return("461 " + args.argv[0] + ":Not enough parameters");

	sender.set_username(args.argv[1]);
	sender.set_realname(args.argv[4]);
	if (!sender.get_nick().empty()) sender.register_user();
}


void JOIN_fn(command_args args, Server& server, User& sender);
void PRIVMSG_fn(command_args args, Server& server, User& sender);
void PING_fn(command_args args, Server& server, User& sender)
{
	if (!sender.is_registered()) return;

	std::string me = "";
	if (args.argv.size() >= 2) me = args.argv[1];
	send_back("PONG " + server.get_prefix());
};

void PONG_fn(command_args args, Server& server, User& sender)
{
	server.set_pong_time(sender.get_id());
};

void QUIT_fn(command_args args, Server& server, User& sender)
{
	server.disconnect_user(server.get_user_index_by_id(sender.get_id()));
}
