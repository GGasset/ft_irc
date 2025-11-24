#include "fnHandlers.hpp"
#include "ParserMessage.hpp"

int main(void) {
	Server server;
	std::string packet = "NICK Rata\r\n";

	msgTokens	tokens;
	ParseStatus	status = VALID_MSG;
	MessageIn	in;

	tokens = msgTokenizer(packet);
	in = parseMessage(tokens, status);

	MessageOut	*ret = fnHandlers()(in.getCommand(), in, server);
	g_Handler_Queue.push_back(ret);
}