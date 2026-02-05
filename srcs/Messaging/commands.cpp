
#include "Channel.hpp"
#include "User.hpp"
#include "router.hpp"
#include <vector>

#define send_back(msg) server.add_msg(":" + server.get_prefix() + RESET " " + args.prefix + " " + msg, sender)
#define notice_back(msg) server.add_msg(YELLOW"NOTICE " + sender.get_nick() + " " + msg, sender)
#define send_return(msg) {send_back(msg); return;}

#define register() {send_back("001 Welcome to the Internet Relay Network " + sender.get_nick() + "!" + sender.getUsername() + "@" + sender.getHostname()); \
					send_back("002 Your host is " + server.get_prefix() + ", running version 42"); \
					send_back("003 This server was created today, I bet ;)"); \
					send_back("004 " + server.get_prefix() + " 42 TODO(usermodes) TODO(channelmodes)" RESET); \
					sender.register_user();}

#define suppr() args.prefix = args.prefix; (User)sender; (std::vector<User>)server.getUsers();

void PASS_fn(command_args args,  Server& server, User& sender)
{
	if (sender.is_registered()) send_return(RED"462 :You may not reregister" RESET);
	if (args.argv.size() <= 1) send_return(RED"461 " + args.argv[0] + " :Not enough parameters" RESET);
	if (args.raw_args != server.get_server_password()) send_return(RED"464 :Password incorrect" RESET);

	sender.passwd_match_pop(true);
	notice_back(BLUE"Correct password, lets keep it a secret! Now send the combination of NICK and USER commands");
	notice_back(BLUE"\t NICK [nick]");
	notice_back(BLUE"\t USER [username] [hostname] [servername] :[realname ...]" RESET);
}

void NICK_fn(command_args args, Server& server, User& sender)
{
	if (args.argv.size() != 2) send_return(RED"431 :No nickname given" RESET);
	if (server.get_user_by_nick(args.argv[1])) send_return(RED"433 " + args.argv[1] + " :Nickname is already in use" RESET);

	std::string prev_nick = sender.get_nick();

	sender.setNick(args.argv[1]);
	notice_back("Nick set to: " + args.argv[1] + RESET);
	if (sender.is_registered() && prev_nick.size())
	{
		std::vector<size_t> channels = sender.get_joined_channels();
		for (size_t i = 0; i < channels.size(); i++)
		{
			Channel &c = *server.get_by_channel_id(channels[i]);
			c.broadcast(server, YELLOW"NOTICE #" + c.get_name() + " " + prev_nick + " changed his nick to: " + sender.get_nick() + RESET);
		}
	}
	if (!sender.getUsername().empty() && !prev_nick.size()) register();
}

void USER_fn(command_args args, Server& server, User& sender)
{
	if (sender.is_registered()) send_return(RED"462 :You may not reregister" RESET);
	if (args.argv.size() < 5 || (args.argv[4].begin()[0] != ':')) send_return(RED"461 " + args.argv[0] + ":Not enough parameters" RESET);

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

	notice_back(BLUE"Username set to: " + sender.getUsername() + ". Hostname set to: " + sender.getHostname() + ". Realname set to: " + sender.getRealname() + RESET);
	if (!sender.get_nick().empty()) register();
}

int Server::check_channels(Channel &c, User &sender, Server &s, command_args args)
{
    Channel *existing = get_by_channel_name(c.get_name());
    if (existing)
    {
        existing->add_member(&sender, &s, ":" + s.get_prefix() + RESET " " + args.prefix + " " + RED"443 :Already on channel" RESET);
        return 0;
    }
    addChannel(c);
	return 0;
}

void JOIN_fn(command_args args, Server& server, User& sender)
{
	int	i;

	i = 0;
	if (args.argv.size() <= 1 || args.argv[1].empty())
		send_return(RED"461 :Not enough parameters" RESET);
	while (args.argv[1][i])
	{
		if ((args.argv[1][0] != '#'))
			send_return(RED"403 :Invalid channel format: JOIN #channel" RESET);
		i++;
	}
	if (args.argv.size() == 3)
	{
	    const std::string &mask = args.argv[2];

	    if (mask[0] != '+' && mask[0] != '-')
	        send_return(RED"403 :BAD MASK" RESET);
	    const std::string allowed = "iktol";
	    for (size_t j = 1; j < mask.size(); ++j)
	    {
	        if (allowed.find(mask[j]) == std::string::npos)
	            send_return(RED"403 :BAD MASK" RESET);
	    }
	}
	Channel c = Channel(args.argv[1], &sender);
	server.check_channels(c, sender, server, args);
	// server.addChannel(c);
    // Channel *ch = server.get_by_channel_name(args.argv[1]);
    // if (ch)
    // {
    //     const std::vector<User *> &members = ch->get_members();
    //     std::cout << "Members of " << ch->get_name() << ": ";
    //     for (size_t idx = 0; idx < members.size(); ++idx)
    //     {
    //         User *m = members[idx];
    //         if (!m) continue;
    //         if (ch->is_operator(m))
    //             std::cout << "@" << m->get_nick();
    //         else
    //             std::cout << m->get_nick();
    //         if (idx + 1 < members.size()) std::cout << ", ";
    //     }
    //     std::cout << std::endl;
    // }
};

void PRIVMSG_fn(command_args args, Server& server, User& sender) { suppr() };
void NAMES_fn(command_args args, Server& server, User& sender) { suppr() };
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

void KICK_fn(command_args args, Server& server, User& sender) {suppr()};
void INVITE_fn(command_args args, Server& server, User& sender) {suppr()};
void TOPIC_fn(command_args args, Server& server, User& sender) {suppr()};
void MODE_fn(command_args args, Server& server, User& sender) {suppr()};

void HELP_fn(command_args args, Server& server, User& sender)
{
	notice_back("Commands:");
	notice_back(BLUE"\t PASS [passw]" RESET);
	notice_back(BLUE"\t NICK [nick]" RESET);
	notice_back(BLUE"\t USER [username] [hostname] [servername] :[realname ...]" RESET);
	notice_back(BLUE"\t JOIN [#channel] [key]" RESET);
	notice_back(BLUE"\t PRIVMSG TODO" RESET);
	notice_back(BLUE"\t PING <sender>" RESET);
	notice_back(BLUE"\t PONG" RESET);
	notice_back(BLUE"\t QUIT <reason>" RESET);
	notice_back(BLUE"\t KICK TODO" RESET);
	notice_back(BLUE"\t INVITE TODO" RESET);
	notice_back(BLUE"\t TOPIC TODO" RESET);
	notice_back(BLUE"\t MODE TODO" RESET);
	suppr()
}
