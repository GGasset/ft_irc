#include "fnHandlers.hpp"
#include "ParserMessage.hpp"

/* Comandito para compilar.*/
//c++ -g main.cpp Message.cpp ParserMessage.cpp Param.cpp MessageOut.cpp fnHandlers.cpp ../Authentication/User.cpp ../Socket/Server.cpp ../Socket/function_router.cpp Channels/Channel.cpp -I../Socket/ -I../Authentication/ -IChannels/

int main(void) {
	/* ---------- Init Server ---------------*/
	Server server;
	server.addUser(User("ratata", 0));

	/* ----------- Lexing and Parsing -------------*/
	std::string packet = "NICK \r\n";
	msgTokens	tokens;
	ParseStatus	status = VALID_MSG;
	MessageIn	in;

	tokens = msgTokenizer(packet);
	in = parseMessage(tokens, status);
	if (status != VALID_MSG)
		return 0;
	in.sender_id = 0; //Esto me lo deberia pasar GG por route_message, de su lista de client_fd.

	/* --------------Handleleo del mensaje ---------- */
	Param	*params = ParamsFactory(in.getCommand(), tokens);
	try {
		params->validateParam();
	} catch (Param::BadSyntax &e) {
		MessageOut	*ret = NumericReplyFactory::create(e.getErrCode(),
													   server,
													   params); //esto va a ver que testearlo
		ret->setTarget(
			MessageTargetFactory::create(
				server, std::vector<size_t> {in.sender_id}, 'u')
		);
		std::cout << "[NumericReply]: " << ret->getRpl() << std::endl;
		return 0;
	}
	in.setParams(params);
	MessageOut	*ret = fnHandlers()(in.getCommand(), in, server);

	if (ret != NULL)
		std::cout << "Respuesta que se enviaría por el socket ante [packet]: " << packet << " --> " << ret->getRpl() << std::endl;
	// g_Handler_Queue.push_back(ret);
}