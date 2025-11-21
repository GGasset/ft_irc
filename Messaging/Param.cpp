#include "Param.hpp"

NickParam::NickParam(msgTokens tokens): Param(NICK, tokens) {}

void	NickParam::validateParam() {
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[0].type == PREFIX
			|| tokens[0].type == WORD
			|| tokens[0].type == NUMBER
			|| tokens[0].type == SPACE)
			continue ;	
	}
	if (tokens[0].type == CRLF)
		throw BadSyntax(NICK, ERR_NONICKNAMEGIVEN);
	nickname = tokens[0].str;
	if (!isValidNickName(tokens[0].str))
		throw BadSyntax(NICK, ERR_ERRONEUSNICKNAME);
}

NumericReply NickParam::mapSyntaxErrorToNumeric(int errCode) {
	//  switch (errCode) {
	// 	case ERR_ERRONEUSNICKNAME:
	// 		return replyFactory.makeErrNoNicknamegiven(user, e.detail());

	// 	case ERR_NONICKNAMEGIVEN:
	// 		return replyFactory.makeErrErroneousNickname(user, e.detail());
	// 	default:
	// 		return replyFactory.makeErrUnknownCommand(user, commandToString(cmd));
	// }
}