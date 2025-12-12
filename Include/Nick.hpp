#pragma once

#include "MessageOut.hpp"
#include "MessageIn.hpp"

class ErrNoNicknamegiven: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		rpl_msg = ":No nickname given";
	}
	public:
		ErrNoNicknamegiven(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_NONICKNAMEGIVEN),
															  np(param) {}
		~ErrNoNicknamegiven() {}
};

class ErrErroneousNickname: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		std::string nickname = np->nickname;
		rpl_msg = nickname + " :Erroneous nickname";
	}

	public:
		ErrErroneousNickname(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_ERRONEUSNICKNAME),
															  np(param) {}
		~ErrErroneousNickname() {}
};

class ErrNicknameInUse: public NumericReply {
	NickParam *np;

	void	assemble_msg() {
		std::string nickname = np->nickname;
		rpl_msg = nickname + " :Nickname is already in use";
	}
	public:
		ErrNicknameInUse(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_NICKNAMEINUSE),
															  np(param) {}
		~ErrNicknameInUse() {}
};

class ErrUnavailResource: public NumericReply {
	NickParam 	*np;
	std::string	name;

	void	assemble_msg() {
		rpl_msg = name + " :Nick/channel is temporarily unavailable";
	}
	public:
		ErrUnavailResource(Server &server, NickParam *param): MessageOut(server),
															  NumericReply(server, ERR_UNAVAILRESOURCE),
															  np(param),
															  name(param->nickname) {}
		/* ..... Se ha de overridear para todos los Comandos que lo utilizen.  */
		~ErrUnavailResource() {}
};