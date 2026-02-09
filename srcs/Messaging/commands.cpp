
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
	std::cout << "AParece coo registrado sin haber escrito nada" << std::endl;
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
	std::cout << "AParece coo registrado sin haber escrito nada" << std::endl;
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

int	Server::check_modes(command_args args, Server &server, User &user, std::string &key)
{
	if (args.argv.size() == 3)
	{
	    const std::string &mask = args.argv[2];

	    if (mask[0] != '+' && mask[0] != '-')
		{
	        server.add_msg(std::string(RED "403: Bad Mask") + RESET, user);
			return 1;
		}
	    const std::string allowed = "iktol";
	    for (size_t j = 1; j < mask.size(); ++j)
	    {
	        if (allowed.find(mask[j]) == std::string::npos)
	    	{
	        	server.add_msg(std::string(RED "403: Bad Mask") + RESET, user);
				return 1;
			}
	    }
		key = args.argv[2];
	}
	return 0;
}

void JOIN_fn(command_args args, Server& server, User& sender)
{
	int		i;
	std::string	key;

	i = 0;
	key = "\0";
	if (args.argv.size() <= 1 || args.argv[1].empty())
		send_return(RED"461 :Not enough parameters" RESET);
	while (args.argv[1][i])
	{
		if ((args.argv[1][0] != '#'))
			send_return(RED"403 :Invalid channel format: JOIN #channel" RESET);
		i++;
	}
	// if (server.check_modes(args, server, sender, key) == 1)
	// 	return ;
	Channel c = Channel(args.argv[1], &sender, key);
	server.check_channels(c, sender, server, args);
};

void PRIVMSG_fn(command_args args, Server& server, User& sender) {	
	if (args.argv.size() <= 1)
		send_return(RED"411: No recipient given (PRIVMSG)" RESET)

	std::string recipients = args.argv[1];
	bool	is_nick = server.get_user_by_nick(recipients) != NULL;
	bool	is_channel = server.get_by_channel_name(recipients) != NULL; // NO se si hay que tener en cuenta lo del asterisco
	std::string priv_msg = "";
	for (std::size_t i = 2; i < args.argv.size() - 2; ++i) priv_msg += args.argv[i];

	if (args.argv.size() == 2) {
		if (is_nick || is_channel)
			send_return(RED"412: No text to send" RESET)
		else
			send_return(RED"411: No recipients given (PRIVMSG)" RESET)
	}

	if (is_nick)
		server.add_msg(":" + args.prefix + " PRIVMSG " + priv_msg, sender);
	else if (is_channel) {
		Channel *c = server.get_by_channel_name(recipients);
		std::vector<size_t> sender_chans = sender.get_joined_channels();
		size_t i = 0;
		for (; i < sender_chans.size(); i++) {
			if (server.get_by_channel_id(sender_chans[i]) == c) {
				c->broadcast(server, ":" + args.prefix + " PRIVMSG " + priv_msg);
				break ;
			}
		}
		if (i < sender_chans.size())
			send_back("404 #" + c->get_name() + ": Cannot send to channel");
	}
	else
		send_back(RED"401 " + recipients + ": No such nick/channel" RESET);
}

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

void KICK_fn(command_args args, Server& server, User& sender)
{
	if (args.argv.size() < 3)
        send_return(RED"461 :Not enough parameters" RESET);
    std::string target_nick = args.argv[1];
    std::string channel_name = args.argv[2];
    User *target = server.get_user_by_nick(target_nick);
    if (!target)
        send_return(RED"401 " + target_nick + " :No such nick" RESET);
    Channel *c = server.get_by_channel_name(channel_name);
    if (!c)
        send_return(RED"403 " + channel_name + " :No such channel" RESET);
    bool sender_is_member = false;
    const std::vector<size_t> &members = c->get_member_ids();
    for (size_t i = 0; i < members.size(); ++i)
        if (members[i] == static_cast<size_t>(sender.get_id()))
        { sender_is_member = true; break; }
    if (!sender_is_member)
        send_return(RED"442 " + channel_name + " :You're not on that channel" RESET);
    c->kick_user(target);
	 server.add_msg(RED":" + sender.get_nick() + " KICKED " + target->get_nick() + " :"  + "FROM: " + c->get_name() + RESET, *target);
};

void INVITE_fn(command_args args, Server& server, User& sender)
{
    if (args.argv.size() < 3)
        send_return(RED"461 :Not enough parameters" RESET);
    std::string target_nick = args.argv[1];
    std::string channel_name = args.argv[2];
    User *target = server.get_user_by_nick(target_nick);
    if (!target)
        send_return(RED"401 " + target_nick + " :No such nick" RESET);
    Channel *c = server.get_by_channel_name(channel_name);
    if (!c)
        send_return(RED"403 " + channel_name + " :No such channel" RESET);
    bool sender_is_member = false;
    const std::vector<size_t> &members = c->get_member_ids();
    for (size_t i = 0; i < members.size(); ++i)
        if (members[i] == static_cast<size_t>(sender.get_id()))
        { sender_is_member = true; break; }
    if (!sender_is_member)
        send_return(RED"442 " + channel_name + " :You're not on that channel" RESET);
    c->invite_user(target);
    server.add_msg(GREEN":" + sender.get_nick() + " INVITED  " + target->get_nick() + "TO :" + c->get_name() + RESET, *target);
};

void TOPIC_fn(command_args args, Server& server, User& sender)
{
    if (args.argv.size() < 2)
        send_return(RED"461 :Not enough parameters" RESET);
    Channel *c = server.get_by_channel_name(args.argv[1]);
    if (!c)
    {
        server.add_msg(std::string(RED "403 " ) + args.argv[1] + " :No such channel" + RESET, sender);
        return ;
    }
    if (args.argv.size() == 2)
    {
        std::string topic = c->get_topic();
        if (topic.empty())
            send_return(RED"331 :No topic defined" RESET);

        server.add_msg(":" + server.get_prefix() + " 332: " + sender.get_nick() + " " + c->get_name() + " :" + topic, sender);
        return;
    }
    std::string newtopic;
    if (!args.argv[2].empty() && args.argv[2][0] == ':')
        newtopic = args.argv[2].substr(1);
    else
        newtopic = args.argv[2];
    for (size_t i = 3; i < args.argv.size(); ++i)
        newtopic += std::string(" ") + args.argv[i];
    c->set_topic(newtopic, &sender, server);
};
void MODE_fn(command_args args, Server& server, User& sender) 
{
	if (args.argv.size() < 3)
        send_return(RED"461 :Not enough parameters" RESET);
	const std::string &mask = args.argv[2];
	if (mask[0] != '+' && mask[0] != '-')
	{
		server.add_msg(std::string(RED "403: Bad Mask") + RESET, sender);
		return ;
	}
	const std::string allowed = "iktol";
	for (size_t j = 1; j < mask.size(); ++j)
	{
		if (allowed.find(mask[j]) == std::string::npos)
		{
			server.add_msg(std::string(RED "403: Bad Mask") + RESET, sender);
			return ;
		}
	}
	Channel *c = server.get_by_channel_name(args.argv[1]);
	if (!c)
	{
		server.add_msg(std::string(RED "403 " ) + args.argv[1] + " :No such channel" + RESET, sender);
		return ;
	}
	c->set_mode(args.argv[2]);
};

void	WHOIS_fn(command_args args, Server& server, User& sender)
{
	if (args.argv.size() <= 1)
		send_return(RED"461 :Not enough parameters" RESET);
	Channel *ch = server.get_by_channel_name(args.argv[1]);
	if (!ch)
		send_return(RED"461 :No such channel" RESET);
    if (ch)
    {
        const std::vector<size_t> &member_ids = ch->get_member_ids();
        notice_back(CYAN"Members of " + ch->get_name() + ": " RESET);
		for (size_t idx = 0; idx < member_ids.size(); ++idx)
		{
			User *m = server.get_user_by_id(static_cast<ssize_t>(member_ids[idx]));
			if (!m) continue;
			if (ch->is_operator(m))
				server.add_msg(std::string(RED "@") + MAGENTA + m->get_nick() + RESET, sender);
			else
				server.add_msg(MAGENTA + m->get_nick() + RESET, sender);
		}
        std::cout << std::endl;
    }
}

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
	notice_back(BLUE"\t INVITE [user] [#channel]" RESET);
	notice_back(BLUE"\t TOPIC TODO" RESET);
	notice_back(BLUE"\t MODE [channel] [mode]" RESET);
	suppr()
}
