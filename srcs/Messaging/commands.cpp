
#include "Channel.hpp"
#include "User.hpp"
#include "router.hpp"
#include <vector>

#define send_back(msg) server.add_msg(":" + server.get_prefix() + " " + args.prefix + " " + msg, sender)
#define notice_back(msg) server.add_msg("NOTICE " + sender.get_nick() + " " + msg, sender)
#define send_return(msg) {send_back(msg); return;}

#define register() send_back("Welcome to the Internet Relay Network " + sender.get_nick() + "!" + sender + "@" + sender.get_hostname()); \
					send_back();

#define suppr() args.prefix = args.prefix; (User)sender; (std::vector<User>)server.getUsers();

void PASS_fn(command_args args,  Server& server, User& sender)
{
	if (sender.is_registered()) send_return("462 :You may not reregister");
	if (args.argv.size() <= 1) send_return("461 " + args.argv[0] + " :Not enough parameters");
	if (args.raw_args != server.get_server_password()) send_return("464 :Password incorrect");

	sender.passwd_match_pop(true);
	notice_back("Correct password, lets keep it a secret! Now send the combination of NICK and USER commands");
}

void NICK_fn(command_args args, Server& server, User& sender)
{
	if (args.argv.size() != 2) send_return("431 :No nickname given");
	if (server.get_user_by_nick(args.argv[1])) send_return("433 " + args.argv[1] + " :Nickname is already in use");

	bool had_nick = sender.get_nick().empty() == 0;

	sender.setNick(args.argv[1]);
	notice_back("Nick set to: " + args.argv[1]);
	if (!sender.getUsername().empty() && !had_nick) {sender.register_user(); notice_back("Registered!");}
}

void USER_fn(command_args args, Server& server, User& sender)
{
	if (sender.is_registered()) send_return("462 :You may not reregister");
	if (args.argv.size() < 5 || (args.argv[4].begin()[0] != ':')) send_return("461 " + args.argv[0] + ":Not enough parameters");

	sender.set_username(args.argv[1]);
	sender.set_hostname(args.argv[2]);

	std::string realname = args.argv[4];
	if (args.argv[4] == ":" || *args.argv[4].begin() == ':')
	{
		realname = "";

		size_t start_index = 4 + (args.argv[4] == ":");
		if (*args.argv[4].begin() == ':') args.argv[4].erase(args.argv[4].begin());

		for (size_t i = start_index; i < args.argv.size(); i++) realname += " " + args.argv[i];
		realname.erase(realname.begin());
	}
	sender.set_realname(realname);

	notice_back("Username set to: " + sender.getUsername() + ". Hostname set to: " + sender.getHostname() + ". Realname set to: " + sender.getRealname());
	if (!sender.get_nick().empty()) {sender.register_user(); notice_back("Registered!");}
}


void JOIN_fn(command_args args, Server& server, User& sender) { suppr() };
void PRIVMSG_fn(command_args args, Server& server, User& sender) { suppr() };
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
	notice_back("PONG received!");
	suppr()
};

void QUIT_fn(command_args args, Server& server, User& sender)
{
	server.disconnect_user(server.get_user_index_by_id(sender.get_id()));
	suppr()
}

void HELP_fn(command_args args, Server& server, User& sender)
{
	notice_back("Commands:");
	notice_back("\t PASS [passw]");
	notice_back("\t NICK [nick]");
	notice_back("\t USER [username] [hostname] [servername] :[realname]");
	notice_back("\t JOIN TODO");
	notice_back("\t PRIVMSG TODO");
	notice_back("\t PING <sender>");
	notice_back("\t PONG");
	notice_back("\t QUIT <reason>");
	notice_back("\t KICK TODO");
	notice_back("\t INVITE TODO");
	notice_back("\t TOPIC TODO");
	notice_back("\t MODE TODO");
	suppr()
}
