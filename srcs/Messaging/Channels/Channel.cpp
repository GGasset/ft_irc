#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel(void){}
Channel::Channel(std::string _name, User* _user) : id(-1), name(_name)
{
	this->name = _name;
	this->mode.passw = "";
	this->mode.topic = "";
	this->mode.user_limit = 100;
	member_user_ids.push_back(_user->get_id());
	users.push_back(_user);
	mode.operator_user_id.clear();
	mode.operator_user_id.push_back(_user->get_id());
	this->topic = "";
}

Channel::~Channel(void){}
ssize_t Channel::get_id() {return id;}
void Channel::set_id(ssize_t id) {this->id = id;}
const std::vector<User *>	&Channel::get_members() const {return users;}
// const std::vector<size_t>& Channel::get_member_ids() const { return member_user_ids; }
std::string Channel::get_name() {return name;}
std::string Channel::get_topic() {return topic;}
void        Channel::set_topic(std::string topic) {this->topic = topic;}

int Channel::add_member(User* user, Server *s, std::string msg)
{
    if (!user) return 1;
    ssize_t uid = user->get_id();
    for (size_t i = 0; i < member_user_ids.size(); ++i)
        if (member_user_ids[i] == (size_t)uid)
		{
			s->add_msg(msg, *user);
            return 1;
		}
    member_user_ids.push_back(uid);
    users.push_back(user);
	return 0;
}

bool Channel::is_operator(const User* user) const
{
	if (!user) return false;
	size_t uid = static_cast<size_t>(const_cast<User*>(user)->get_id());
	for (size_t i = 0; i < mode.operator_user_id.size(); ++i)
		if (mode.operator_user_id[i] == uid)
			return true;
	return false;
}

/*
	boradcast (aun debo hacer el join bien)

	params:
			Server& serv -> referencia a server que controla los clientes y colas de salida,
			se llama para obtener el usuario destino y encolar el envio

			string &msg -> texto que se tiene que enviar a cada miembro del canal


	loop:
			obtener user id y guardar direccion en cather
			copiar bytes de msg a buffer
			encolar el mensaje y marcar true para asegurar que el serv libera el buffer

	return: void
	exception:
			si !catcher salta ese user
*/
void	Channel::broadcast(Server& serv, const std::string &msg)
{
	std::vector<size_t>	users;
	//size_t				len;
	User				*catcher;
	//char				*buffer;

	users = member_user_ids;
	for (size_t i = 0; i < users.size(); i++)
	{
		catcher = serv.get_user_by_id(users[i]);
		if (!catcher)
			continue;
		// len = msg.size();
		// buffer = new char[len];
		// std::memcpy(buffer, msg.data(), len);
		// serv.add_msg(buffer, len, true, *catcher);
		serv.add_msg(msg, *catcher);
	}
}
