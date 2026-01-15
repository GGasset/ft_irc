#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel(void){}
Channel::Channel(std::string _name) : name(_name) {}
Channel::~Channel(void){}
ssize_t Channel::get_id() {return id;}
void Channel::set_id(ssize_t id) {id = id;}
std::vector<size_t>	Channel::get_members() {return member_user_ids;}
std::string Channel::get_name() {return name;}
std::string Channel::get_topic() {return topic;}
void        Channel::set_topic(std::string topic) {topic = topic;}

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
	size_t				len;
	User				*catcher;
	char				*buffer;

	users = member_user_ids;
	for (size_t user_id : users)
	{
		catcher = &serv.get_user_by_id(user_id);
		if (!catcher)
			continue;
		len = msg.size();
		buffer = new char[len];
		std::memcpy(buffer, msg.data(), len);
		serv.add_msg(buffer, len, true, *catcher);
	}
}
