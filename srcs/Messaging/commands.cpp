
#include "Channel.hpp"
#include "User.hpp"
#include "router.hpp"
#include <vector>

#define send_back(msg) server.add_msg(":" + server.get_prefix() + " " + args.prefix + " " + msg, sender)
#define notice_back(msg) server.add_msg("NOTICE " + sender.get_nick() + " " + msg, sender)
#define send_return(msg) {send_back(msg); return;}

#define register() {send_back("001 Welcome to the Internet Relay Network " + sender.get_nick() + "!" + sender.getUsername() + "@" + sender.getHostname()); \
					send_back("002 Your host is " + server.get_prefix() + ", running version 42"); \
					send_back("003 This server was created today, I bet ;)"); \
					send_back("004 " + server.get_prefix() + " 42 TODO(usermodes) TODO(channelmodes)"); \
					sender.register_user();}

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

	std::string prev_nick = sender.get_nick();

	sender.setNick(args.argv[1]);
	notice_back("Nick set to: " + args.argv[1]);
	if (sender.is_registered() && prev_nick.size())
	{
		std::vector<size_t> channels = sender.get_joined_channels();
		for (size_t i = 0; i < channels.size(); i++)
		{
			Channel &c = server.get_by_channel_id(channels[i]);
			c.broadcast(server, "NOTICE #" + c.get_name() + " " + prev_nick + " changed his nick to: " + sender.get_nick());
		}
	}
	if (!sender.getUsername().empty() && !prev_nick.size()) register();
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
	if (!sender.get_nick().empty()) register();
}


void JOIN_fn(command_args args, Server& server, User& sender) { suppr() };

// bool is_nick(Server &serv, std::string dest) {
	
// }

void PRIVMSG_fn(command_args args, Server& server, User& sender) {
	// 411, 412, 401,
	// Cuando no se especifica un usuario/canal 411
	// Cuando no se especifica un mensaje 412
	// 		Esto incluye meter ":" y despues no meter nada.
	// Cuando no existe el canal, entonces me sacas 401:
	// Cuando tratas de escribir a un canal que no eres miembro 404:
	//		Dejas hablar en el canal solo a miembros, y si no está dentro devuelves 404 igualmente como “cannot send to channel” 
	//		(aunque no estés implementando +n, 
	//		lo estás usando como “no puedes enviar porque no estás en el canal”)

	// 407 para error de envio de demasiados usuarios. En la practica me la suda.


	// Si el tamaño de argv es dos, se comprueba si el segundo argumento es un nick.
	// Si lo es, devuelves error 412. Si no lo es, 411.

	// Si el tamaño es tres entonces compruebas si está separado por comas. Si lo está.
	// Le haces un split. Y compruebas por cada uno si existe. Si alguno no existe le mandas
	// Un 401, y envias el resto a su destinatario. Si la lista es demasiado larga, 407

	// Finalmente compruebas si estas dentro del canal. Si no estas, 404.
	// Si no ocurre ninguna de las anteriores --> mandas el mensaje normal
	// 		Si empieza por #canal, broadcast, si es usuario, add_msg.

	// Para ver si el nick existe --> get_user_by_nick
	// Para ver si el canal existe --> 
	
	std::string recipients;
	int type_of_recipient = 0; // 0 para nicks, 1, para canales.

	if (args.argv.size() <= 1)
		send_return("411 :No recipient given (PRIVMSG)")
	
	recipients = args.argv[2];
	std::vector<std::string> recip_list;
	std::string::size_type start = 0;
	std::string::size_type pos = recipients.find(',');
	bool is_nick = server.get_user_by_nick(recipients);
	bool is_channel = server.get_by_channel_name(recipients).get_name() == recipients;

	if (args.argv.size() == 2) {
		if (is_nick || is_channel)
			send_return("411 :No recipients given (PRIVMSG)")
		else
			send_return("412 :No text to send");
	}

    while (true) {
		
        pos = recipients.find(',', start);
        if (pos == std::string::npos) {
			recip_list.push_back(recipients.substr(start));
            break;
		}
		recip_list.push_back(recipients.substr(start, pos - start));
        start = pos + 1; // saltar el delimitador
    }

	// if (recip_list)

	for (size_t i = 0; i < recip_list.size(); i++) {

	}
}

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
