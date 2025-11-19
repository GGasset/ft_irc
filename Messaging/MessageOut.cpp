#include "Message.hpp"

void	MessageOut::fillMsgOut(User u, std::string servername, std::string cmd, std::string params)
{
	(void) u;
	std::string message = "";
	/* 
		Por cada id:
			- Se construye el mensaje con Nick!user@nombre del servidor.
			- cmd que es un numerito.
			- Parametros que los mismo.
	*/
	for (size_t i = 0; i < ids.size(); i++) {
		message += servername + cmd + params;
		// send Algo.
	}
}