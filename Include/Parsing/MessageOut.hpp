#pragma once

#include "Param.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>

class MessageTarget {
	protected:
		Server &server;
		std::vector<size_t>	ids; //Esto sirve para tanto chanels como usuarios.

	public:
		MessageTarget(Server& server, std::vector<size_t> ids): server(server), ids(ids) {}
		virtual ~MessageTarget() {}
		virtual	void deliver(void *msg) = 0;
};

class UsersTarget : public MessageTarget {
	public:
		UsersTarget(Server& server, std::vector<size_t> ids): MessageTarget(server, ids) {}
		UsersTarget(const UsersTarget& other): MessageTarget(other.server, other.ids) {}
		~UsersTarget() {}
		UsersTarget&	operator=(const UsersTarget& other) {
			if (this != &other)
				ids = other.ids;
			return (*this);
		}
		void	deliver(void *msg) {
			if (!msg)
				return ;
			for (int i = 0; i < ids.size(); i++)
				server.add_msg(msg, 512, false, server.get_user_by_id(ids[i]));
		}
};

class ChannelTarget : public MessageTarget {
	public:
		ChannelTarget(Server& server, std::vector<size_t> ids): MessageTarget(server, ids) {}
		ChannelTarget(const ChannelTarget& other): MessageTarget(other.server, other.ids) {}
		~ChannelTarget() {}
		ChannelTarget&	operator=(const ChannelTarget& other) {
			if (this != &other)
				ids = other.ids;
			return (*this);
		}
		void	deliver(void *msg) {
			Channel ch;

			if (!msg)
				return ;
			// for (int i = 0; i < ids.size(); i++) {
			// 	ch = server.get_by_channel_id(ids[i]);
			// 	server.add_msg(msg, 512, false, ch);
			// }
			ch = server.get_by_channel_id(ids[0]);
			/* Esto esta fatal, es para que no toque los huevos. */
			server.add_msg(msg, 512, false, server.get_user_by_id(ch.get_members()[0]));
		}

};

class MessageTargetFactory {
	public:
		MessageTargetFactory() {}
		static MessageTarget	*create(Server &server, std::vector<size_t> ids, char t) {
			if (t == 'u')
				return new UsersTarget(server, ids);
			else if (t == 'c')
				return new ChannelTarget(server, ids);
			else
				return NULL;
		}
};

class MessageOut
{
	protected:
		Server			&server;
		std::string		prefix;
		char			msg[512]; //El mensaje que se serializa para mandar a GG.
		std::string		rpl_msg;
		MessageTarget	*target; //target envía indistintamente para canales como para usuarios.
		virtual void	assemble_msg() = 0;
		virtual void	fill_prefix() = 0; //Servername ó n!u@h
		virtual void	serialize() = 0;

	public:
		size_t	sender_id; //id del cliente que envia el mensaje
		MessageOut(Server &server): server(server), sender_id(0) {}
		// MessageOut	&operator=(const MessageOut& other);
		virtual	~MessageOut() = 0;
		void	*get_msg();
		void	setTarget(MessageTarget *target);
		void	deliver() {target->deliver(msg);}

		//Esto es solo de testeo
		std::string getRpl(void) {
			serialize();
			return rpl_msg;
		}//Esto solo de testeo
};

class NumericReply: virtual public MessageOut {
	protected:
		unsigned int code; //Para no tener que comprobar si es alfanumerico. Solo se comprueba rango 001-552

		void	fill_prefix() {
			// prefix = server.getServerName(); Nombre del archivo de configuración
			prefix = "irc.local";
		}

		void	serialize() {
			// std::string nick_sender = server.get_user_by_id(sender_id).get_nick();

			fill_prefix();
			assemble_msg();
			rpl_msg = prefix +  " " + codetoa() + " " + rpl_msg + "\r\n";
			// rpl_msg = prefix +  " " + codetoa() + " " + nick_sender + " " + rpl_msg + "\r\n";
			memcpy(msg, rpl_msg.c_str(), rpl_msg.length());
		}

	public:
		NumericReply(Server &server, unsigned int code): MessageOut(server),
													   				code(code) {}
		~NumericReply() {}
		std::string	codetoa() {
			std::ostringstream ss;
			ss << std::setw(3) << std::setfill('0') << code;
			return ss.str();
		}
		unsigned int getCode() {return code;}
};


class RplWelcome: public NumericReply {
	UserParam	*up;

	void	assemble_msg() {
		User u = server.get_user_by_id(sender_id);
		std::string	sender_info = u.get_nick() + "!" + u.getUsername() + "@";
		rpl_msg = "Welcome to the Internet Relay Network " + sender_info;
	}

	public:
		RplWelcome(Server &server, UserParam *param): MessageOut(server),
													  NumericReply(server, RPL_WELCOME),
													  up(param) {}
		~RplWelcome() {}
};

class RplYourHost: public NumericReply {
	UserParam	*up;

	void	assemble_msg() {
		User u = server.get_user_by_id(sender_id);
		std::string	servername = "irc.local"; //Esto deberia estar dentro de Server, que viene de la configuracion.
		std::string ver = "1.1"; //Me lo invento. Tambien deberia estar dentro de la configuración.
		rpl_msg = "Your host is " + servername + ", running version " + ver;
	}

	public:
		RplYourHost(Server &server, UserParam *param): MessageOut(server),
													  NumericReply(server, RPL_YOURHOST),
													  up(param) {}
		~RplYourHost() {}
};

class RplCreated: public NumericReply {
	UserParam	*up;

	void	assemble_msg() {
		std::time_t now = std::time(nullptr);
		std::tm *ltm = std::localtime(&now);

		std::ostringstream oss;
		oss << (1900 + ltm->tm_year) << "-"
			<< std::setw(2) << std::setfill('0') << (1 + ltm->tm_mon) << "-"
			<< std::setw(2) << std::setfill('0') << ltm->tm_mday;
		rpl_msg = "This server was created " + oss.str();
	}

	public:
		RplCreated(Server &server, UserParam *param): MessageOut(server),
													  NumericReply(server, RPL_CREATED),
													  up(param) {}
		~RplCreated() {}
};

// class RplMyInfo: public NumericReply {

// };

/* A registrar en create */
class RplNoTopic : public NumericReply {
	JoinParam *jp;

	void assemble_msg() {
		User u = server.get_user_by_id(sender_id);
		std::string nick = u.get_nick();
		std::string channel = jp->channels.empty() ? "" : jp->channels[0];

		rpl_msg = nick + " " + channel + " :No topic is set";
	}

	public:
		RplNoTopic(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, RPL_NOTOPIC),
			jp(param) {}
};

/* A registrar en create */
class RplTopic : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        std::string topic = server.get_by_channel_name(channel).get_topic(); // adapta si hace falta

        rpl_msg = nick + " " + channel + " :" + topic;
    }

	public:
		RplTopic(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, RPL_TOPIC),
			jp(param) {}
};

/* A registrar en create */
class RplNamReply : public NumericReply {
    NamesParam *np; // o JoinParam, depende de tu diseño
	Channel		channel;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        // std::string symbol = np->symbol; // normalmente "=" o "@" La obtiene del channel_mode
        std::string symbol = "@"; // Por ahora me lo invento
        std::vector<size_t> members = channel.get_members();

		std::string member_names = "";
		for (int i = 0; i < members.size(); i++) member_names += server.get_user_by_id(members[i]).get_nick() + " ";

        rpl_msg = u.get_nick() + " " + symbol + " " + channel.get_name() + " :" + member_names;
    }
public:
    RplNamReply(Server &server, NamesParam *param, std::string &ch)
        : MessageOut(server),
          NumericReply(server, RPL_NAMREPLY),
          np(param),
		  channel(server.get_by_channel_name(ch)) {}
};

/* A registrar en create */
class RplEndOfNames : public NumericReply {
    NamesParam *np;
	Channel		channel;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + channel.get_name() + " :End of /NAMES list.";
    }
public:
    RplEndOfNames(Server &server, NamesParam *param, std::string &ch)
        : MessageOut(server),
          NumericReply(server, RPL_ENDOFNAMES),
          np(param),
		  channel(server.get_by_channel_name(ch)) {}
};

/* A registrar en create */
class RplChannelModeIs : public NumericReply {
    ModeParam *mp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string channel = mp->channel;
        std::string modes = mp->modeStr;
        std::string args = mp->modeArg;

        rpl_msg = u.get_nick() + " " + channel + " " + modes;
        if (!args.empty())
            rpl_msg += " " + args;
    }
	public:
		RplChannelModeIs(Server &server, ModeParam *param)
			: MessageOut(server),
			NumericReply(server, RPL_CHANNELMODEIS),
			mp(param) {}
};

/* A registrar en create */
class RplInviting : public NumericReply {
    InviteParam *ip;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + ip->nick + " " + ip->channel;
    }
public:
    RplInviting(Server &server, InviteParam *param)
        : MessageOut(server),
          NumericReply(server, RPL_INVITING),
          ip(param) {}
};

//class RplWhoisUser : public NumericReply {
    //WhoisParam	*p;
//public:
    //RplWhoisUser(Server &serv, WhoisParam *param, int nick_idx)
        //: MessageOut(serv), NumericReply(serv, RPL_WHOISUSER), p(param) {}

    //void assemble_msg() {
		////User *u = server.get_user_by_nick();
        //rpl_msg = u->get_nick() + " "
                //+ u->getUsername() + " "
                //+ u->getHostname() + " * :"
                //+ u->getRealname();
    //}
//};


// class RplWhoReply : public NumericReply {
//     User *u;
//     std::string channel;
//     std::string hopcount;
//     std::string realname;
// public:
//     RplWhoReply(Server &serv, size_t target_id,
//                 const std::string& channel,
//                 User *u)
//         : MessageOut(server), NumericReply(serv, RPL_WHOREPLY), u(u), channel(channel)
//     {
//         sender_id = target_id;
//         hopcount = "0";
//         realname = u->getRealname();
//     }

//     void assemble_msg() {
//         std::string H_G = u->isAway() ? "G" : "H";
//         std::string status = H_G;
//         // if (u->isOperator())
//         //     status += "*";
//         if (server.userHasPrefix(channel, u->id()))
//             status += server.getPrefixSymbol(channel, u->id());

//         rpl_msg = channel + " "
//             + u->getUsername() + " "
//             + u->getHostname() + " "
//             + "irc.local.fr" + " "
//             // + server.get_servername() + " "
//             + u->get_nick() + " "
//             + status + " :"
//             + hopcount + " "
//             + realname;
//     }
// };


class ErrBadChannelKey : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        rpl_msg = nick + " " + channel + " :Cannot join (+k)";
    }

	public:
		ErrBadChannelKey(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_BADCHANNELKEY),
			jp(param) {}
};

class ErrGeneric: public NumericReply {
	void	assemble_msg() {
		rpl_msg = "Generic";
	}
	public:
		ErrGeneric(Server &server): MessageOut(server),
									NumericReply(server, 0) {}
		~ErrGeneric() {}
};

// class ErrErroneousNickname: public NumericReply {
// 	NickParam *np;

// 	void	assemble_msg() {
// 		std::string nickname = np->nickname;
// 		rpl_msg = nickname + " :Erroneous nickname";
// 	}

// 	public:
// 		ErrErroneousNickname(Server &server, NickParam *param): MessageOut(server),
// 															  NumericReply(server, ERR_ERRONEUSNICKNAME),
// 															  np(param) {}
// 		~ErrErroneousNickname() {}
// };

// class ErrNoNicknamegiven: public NumericReply {
// 	NickParam *np;

// 	void	assemble_msg() {
// 		rpl_msg = ":No nickname given";
// 	}
// 	public:
// 		ErrNoNicknamegiven(Server &server, NickParam *param): MessageOut(server),
// 															  NumericReply(server, ERR_NONICKNAMEGIVEN),
// 															  np(param) {}
// 		~ErrNoNicknamegiven() {}
// };

// class ErrNicknameInUse: public NumericReply {
// 	NickParam *np;

// 	void	assemble_msg() {
// 		std::string nickname = np->nickname;
// 		rpl_msg = nickname + " :Nickname is already in use";
// 	}
// 	public:
// 		ErrNicknameInUse(Server &server, NickParam *param): MessageOut(server),
// 															  NumericReply(server, ERR_NICKNAMEINUSE),
// 															  np(param) {}
// 		~ErrNicknameInUse() {}
// };

// class ErrUnavailResource: public NumericReply {
// 	NickParam 	*np;
// 	std::string	name;

// 	void	assemble_msg() {
// 		rpl_msg = name + " :Nick/channel is temporarily unavailable";
// 	}
// 	public:
// 		ErrUnavailResource(Server &server, NickParam *param): MessageOut(server),
// 															  NumericReply(server, ERR_UNAVAILRESOURCE),
// 															  np(param),
// 															  name(param->nickname) {}
// 		/* ..... Se ha de overridear para todos los Comandos que lo utilizen.  */
// 		~ErrUnavailResource() {}
// };

class ErrNeedMoreParams: public NumericReply {
	Param 	*p;
	std::string	commandname;

	void	assemble_msg() {
		rpl_msg = commandname + " :Not enough parameters";
	}

	public:
		ErrNeedMoreParams(Server &server, COMMAND command): MessageOut(server),
															  NumericReply(server, ERR_NEEDMOREPARAMS),
															  commandname(getCommandname(command)) {}
		/* ..... Se ha de overridear para todos los Comandos que lo utilizen.  */
		~ErrNeedMoreParams() {}
};

class ErrRestricted: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		std::string nickname = np->nickname;
		rpl_msg = nickname + " :Nickname is already in use";
	}
	public:
		ErrRestricted(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_RESTRICTED),
															  np(param) {}
		~ErrRestricted() {}
};

class ErrAlredyRegistered: public NumericReply {
	Param	*p;

		void	assemble_msg() {
		rpl_msg = ":Unauthorized command (already registered)";
	}
	public:
		ErrAlredyRegistered(Server &server, UserParam *param): MessageOut(server),
															  NumericReply(server, ERR_ALREADYREGISTRED),
															  p(param) {}
		ErrAlredyRegistered(Server &server, PassParam *param): MessageOut(server),
															   NumericReply(server, ERR_ALREADYREGISTRED),
															   p(param) {}
		~ErrAlredyRegistered() {}
};

class ErrNoOrigin: public NumericReply {
	PingPongParam	*pip;

		void	assemble_msg() {
		rpl_msg = ":No origin specified";
	}
	public:
		ErrNoOrigin(Server &server, PingPongParam *param): MessageOut(server),
													   NumericReply(server, ERR_NOORIGIN),
													   pip(param) {}
		~ErrNoOrigin() {}
};

class ErrNoSuchServer: public NumericReply {
	PingPongParam	*pip;

	void	assemble_msg() {
		rpl_msg = pip->server2 + " :No such server";
	}
	public:
		ErrNoSuchServer(Server &server, PingPongParam *param): MessageOut(server),
													   NumericReply(server, ERR_NOSUCHSERVER),
													   pip(param) {}
		~ErrNoSuchServer() {}
};

class ErrUnknownCommand: public NumericReply {
	void	assemble_msg() {
		rpl_msg = "UnknownCommand";
	}
	public:
		ErrUnknownCommand(Server &server): MessageOut(server),
										   NumericReply(server, 421) {}
		~ErrUnknownCommand() {}
};

/* A registrar en create */
class ErrNoSuchChannel : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        rpl_msg = nick + " " + channel + " :No such channel";
    }

	public:
		ErrNoSuchChannel(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_NOSUCHCHANNEL),
			jp(param) {}
};

/* A registrar en create */
class ErrTooManyChannels : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        rpl_msg = nick + " " + channel + " :Too many channels";
    }

	public:
		ErrTooManyChannels(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_TOOMANYCHANNELS),
			jp(param) {}
};

/* A registrar en create */
class ErrChannelIsFull : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        rpl_msg = nick + " " + channel + " :Cannot join (+l)";
    }

	public:
		ErrChannelIsFull(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_CHANNELISFULL),
			jp(param) {}
};

/* A registrar en create */
class ErrInviteOnlyChan : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        rpl_msg = nick + " " + channel + " :Cannot join (+i)";
    }

	public:
		ErrInviteOnlyChan(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_INVITEONLYCHAN),
			jp(param) {}
};

/* A registrar en create */
class ErrBannedFromChan : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        rpl_msg = nick + " " + channel + " :Cannot join (+b)";
    }

	public:
		ErrBannedFromChan(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_BANNEDFROMCHAN),
			jp(param) {}
};

/* A registrar en create */
class ErrBadChanMask : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        rpl_msg = nick + " " + channel + " :Bad channel mask";
    }

	public:
		ErrBadChanMask(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_BADCHANMASK),
			jp(param) {}
};

/* A registrar en create */
class ErrUnsupportedChanMode : public NumericReply {
    JoinParam *jp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = jp->channels.empty() ? "" : jp->channels[0];

        rpl_msg = nick + " " + channel + " :Channel doesn't support modes";
    }

	public:
		ErrUnsupportedChanMode(Server &server, JoinParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_UNSUPPORTEDCHANMODE),
			jp(param) {}
};

/* A registrar en create */
class ErrNotOnChannel : public NumericReply {
    PartParam *pp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string nick = u.get_nick();
        std::string channel = pp->channels.empty() ? "" : pp->channels[0];

        rpl_msg = nick + " " + channel + " :You're not on that channel";
    }
	public:
		ErrNotOnChannel(Server &server, PartParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_NOTONCHANNEL),
			pp(param) {}
};

/* A registrar en create */
class ErrNoRecipient : public NumericReply {
    PrivMsgParam *pm;
    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " :No recipient given (PRIVMSG)";
    }
	public:
		ErrNoRecipient(Server &server, PrivMsgParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_NORECIPIENT),
			pm(param) {}
};

/* A registrar en create */
class ErrNoTextToSend : public NumericReply {
    PrivMsgParam *pm;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " :No text to send";
    }
	public:
		ErrNoTextToSend(Server &server, PrivMsgParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_NOTEXTTOSEND),
			pm(param) {}
};

/* A registrar en create */
class ErrNoSuchNick : public NumericReply {
    PrivMsgParam *pm;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        std::string target = pm->target;
        rpl_msg = u.get_nick() + " " + target + " :No such nick/channel";
    }
	public:
		ErrNoSuchNick(Server &server, PrivMsgParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_NOSUCHNICK),
			pm(param) {}
};

/* A registrar en create */
class ErrCannotSendToChan : public NumericReply {
    PrivMsgParam *pm;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + pm->target + " :Cannot send to channel";
    }
	public:
		ErrCannotSendToChan(Server &server, PrivMsgParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_CANNOTSENDTOCHAN),
			pm(param) {}
};

/* A registrar en create */
class ErrTooManyTargets : public NumericReply {
    PrivMsgParam *pm;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + pm->target + " :Too many targets";
    }
	public:
		ErrTooManyTargets(Server &server, PrivMsgParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_TOOMANYTARGETS),
			pm(param) {}
};

/* A registrar en create */
class ErrNoTopLevel : public NumericReply {
    PrivMsgParam *pm;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + pm->target + " :No toplevel domain specified";
    }
	public:
		ErrNoTopLevel(Server &server, PrivMsgParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_NOTOPLEVEL),
			pm(param) {}
};

/* A registrar en create */
class ErrWildTopLevel : public NumericReply {
    PrivMsgParam *pm;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + pm->target + " :Wildcard in toplevel domain";
    }
	public:
		ErrWildTopLevel(Server &server, PrivMsgParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_WILDTOPLEVEL),
			pm(param) {}
};

/* A registrar en create */
class ErrUnknownMode : public NumericReply {
    ModeParam *mp;
    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + mp->modeStr + " :is unknown mode character";
    }
	public:
		ErrUnknownMode(Server &server, ModeParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_UNKNOWNMODE),
			mp(param) {}
};

/* A registrar en create */
class ErrUModeUnknownFlag : public NumericReply {
    ModeParam *mp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " :Unknown MODE flag";
    }
	public:
		ErrUModeUnknownFlag(Server &server, ModeParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_UMODEUNKNOWNFLAG),
			mp(param) {}
};

/* A registrar en create */
class ErrKeySet : public NumericReply {
    ModeParam *mp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + mp->channel + " :Channel key already set";
    }
	public:
		ErrKeySet(Server &server, ModeParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_KEYSET),
			mp(param) {}
};

/* A registrar en create */
class ErrUserNotInChannel : public NumericReply {
    ModeParam *mp;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + mp->modeArg + " " + mp->channel
                + " :They aren't on that channel";
    }
	public:
		ErrUserNotInChannel(Server &server, ModeParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_USERNOTINCHANNEL),
			mp(param) {}
};

class ErrUserOnChannel : public NumericReply {
    InviteParam *ip;

    void assemble_msg() {
        User u = server.get_user_by_id(sender_id);
        rpl_msg = u.get_nick() + " " + ip->nick + " " + ip->channel
                + " :Is already on channel";
    }
	public:
		ErrUserOnChannel(Server &server, InviteParam *param)
			: MessageOut(server),
			NumericReply(server, ERR_USERONCHANNEL),
			ip(param) {}
};



class ForwardedCommand: virtual public MessageOut {
	Param	*param;

	void	fill_prefix() {
		User u = server.get_user_by_id(sender_id);
		prefix = u.get_nick() + "!" + u.getUsername() + "@"; //+ u.getHostname;
	}
	void	serialize() {
		fill_prefix();
		assemble_msg();
		rpl_msg = prefix +  " " + getCommandname(param->command()) + rpl_msg + "\r\n";
		memcpy(msg, rpl_msg.c_str(), rpl_msg.length());
	}

	public:
		ForwardedCommand(Server &server, Param *param): MessageOut(server), param(param) {}
		~ForwardedCommand() {}
};

class NickForwardedCommand: public ForwardedCommand {
	NickParam	*np;

	public:
		NickForwardedCommand(Server &server, NickParam *param): MessageOut(server),
																ForwardedCommand(server, param),
																np(param) {}
		void	assemble_msg() {
			rpl_msg = "NICK " + np->nickname;
		}
};

class PongForwardedCommand: public ForwardedCommand {
	PingPongParam	*p;

	public:
		PongForwardedCommand(Server &server, PingPongParam *param): MessageOut(server),
																	ForwardedCommand(server, param),
																	p(param) {}

		void	assemble_msg() {
			rpl_msg = "PONG " + p->server1 + " " + p->server2;
		}
};

class QuitForwardedCommand: public ForwardedCommand {
	QuitParam	*p;

	public:
		QuitForwardedCommand(Server &server, QuitParam *param): MessageOut(server),
																ForwardedCommand(server, param),
																p(param) {}

		void	assemble_msg() {
			rpl_msg = "PASS" + p->quit_msg;
		}
};

class JoinForwardedCommand : public ForwardedCommand {
    JoinParam *jp;

	public:
		JoinForwardedCommand(Server &server, JoinParam *param)
			: MessageOut(server),
			ForwardedCommand(server, param),
			jp(param) {}

		void assemble_msg() {
			// En forwarding el servidor solo manda el JOIN de 1 canal
			// Si tu parser permite múltiples, forwardeamos solo el primero
			if (jp->channels.empty())
				rpl_msg = "JOIN";
			else
				rpl_msg = "JOIN " + jp->channels[0];
		}
};

class PartForwardedCommand : public ForwardedCommand {
    PartParam *pp;

	public:
		PartForwardedCommand(Server &server, PartParam *param)
			: MessageOut(server),
			ForwardedCommand(server, param),
			pp(param) {}

		void assemble_msg() {
			// Igual que JOIN: solo se forwardea un canal a la vez
			std::string channel = pp->channels.empty() ? "" : pp->channels[0];
			rpl_msg = "PART " + channel;

			if (!pp->partMsg.empty())
				rpl_msg += " :" + pp->partMsg;
		}
};

class PrivMsgForwardedCommand : public ForwardedCommand {
    PrivMsgParam *pm;

	public:
		PrivMsgForwardedCommand(Server &server, PrivMsgParam *param)
			: MessageOut(server),
			ForwardedCommand(server, param),
			pm(param) {}

		void assemble_msg() {
			rpl_msg = "PRIVMSG " + pm->target + " :" + pm->text;
		}
};

class NoticeForwardedCommand : public ForwardedCommand {
    NoticeParam *np;

	public:
		NoticeForwardedCommand(Server &server, NoticeParam *param)
			: MessageOut(server),
			ForwardedCommand(server, param),
			np(param) {}

		void assemble_msg() {
			if (np->text.empty())
				rpl_msg = "NOTICE " + np->target;
			else
				rpl_msg = "NOTICE " + np->target + " :" + np->text;
		}
};

class TopicForwardedCommand : public ForwardedCommand {
    TopicParam *tp;

public:
    TopicForwardedCommand(Server &server, TopicParam *param)
        : MessageOut(server),
          ForwardedCommand(server, param),
          tp(param) {}

    void assemble_msg() {
        rpl_msg = "TOPIC " + tp->channel;
        if (!tp->topic.empty())
            rpl_msg += " :" + tp->topic;
    }
};

class InviteForwardedCommand : public ForwardedCommand {
    InviteParam *ip;

	public:
		InviteForwardedCommand(Server &server, InviteParam *param)
			: MessageOut(server),
			ForwardedCommand(server, param),
			ip(param) {}

		void assemble_msg() {
			rpl_msg = "INVITE " + ip->nick + " " + ip->channel;
		}
};

class KickForwardedCommand : public ForwardedCommand {
    KickParam *kp;

	public:
		KickForwardedCommand(Server &server, KickParam *param)
			: MessageOut(server),
			ForwardedCommand(server, param),
			kp(param) {}

		void assemble_msg() {
			rpl_msg = "KICK " + kp->channel + " " + kp->user;
			if (!kp->comment.empty())
				rpl_msg += " :" + kp->comment;
		}
};

class ModeForwardedCommand : public ForwardedCommand {
    ModeParam *mp;

public:
    ModeForwardedCommand(Server &server, ModeParam *param)
        : MessageOut(server),
          ForwardedCommand(server, param),
          mp(param) {}

    void assemble_msg() {
        rpl_msg = "MODE " + mp->channel + " " + mp->modeStr;
        if (!mp->modeArg.empty())
            rpl_msg += " " + mp->modeArg;
    }
};
