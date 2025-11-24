#include "fnHandlers.hpp"
#include "ParserMessage.hpp"

/* Comandito para compilar.*/
//c++ -g main.cpp Message.cpp ParserMessage.cpp Param.cpp MessageOut.cpp fnHandlers.cpp ../Authentication/User.cpp ../Socket/Server.cpp ../Socket/function_router.cpp Channels/Channel.cpp -I../Socket/ -I../Authentication/ -IChannels/

int main(void) {
	Server server;
	std::string packet = "NICK Rata\r\n";

	msgTokens	tokens;
	ParseStatus	status = VALID_MSG;
	MessageIn	in;

	tokens = msgTokenizer(packet);
	in = parseMessage(tokens, status);
	
	MessageOut	*ret = fnHandlers()(in.getCommand(), in, server);
	std::cout << "Respuesta que se enviaría por el socket ante [packet]: " << packet << " --> " << ret->getRpl() << std::endl;
	// g_Handler_Queue.push_back(ret);
}