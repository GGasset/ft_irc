#pragma once

#include "Param.hpp"

/* Clase que guarda la información del mensaje del cliente. */
class MessageIn {
	private:
		COMMAND		cmd;
		// std::string prefix; Realmente es util?? Tengo el id del sender, puedo acceder a nick, user, host desde allí.
		Param		*params;

	public:
		MessageIn(): cmd(COMMAND0) {}
		MessageIn(COMMAND cmd): cmd(cmd) {}
								// prefix(prefix) {}
		MessageIn& operator=(const MessageIn& other) {
			if (this != &other) {
				cmd = other.cmd;
				sender_id = other.sender_id;
				// params = other.params; Estoy hay que verlo. Quizá con nuestra propia implementación de shared_ptr.
			}
			return (*this);
		}

		COMMAND	getCommand() {return (cmd);}
		void	setCommand(COMMAND command) {cmd = command;}
		Param	*getParams() { return (params);}
		void	setParams(Param *param) {this->params = param;}
		msgTokens tokens;
		size_t	sender_id; //id del cliente que envia el mensaje
};