#include <string>
#include <vector>
#include <cassert>
#include <iostream>

typedef std::vector<std::string> msgs;

enum msg_token_type
{
	PREFIX,
	SPACE,
	COLON,
	WORD,
	NUMBER,
	COMMA_LIST,
	TRAIL
};

typedef struct msg_token
{
	msg_token_type type;
	std::string str;
}	msg_token;

typedef std::vector<msg_token> msgTokens;

enum msgState
{
	PRIX = 0,
	CMD,
	PARAM
};

// Please, make message a POD type 😵🤙
struct MessageOut
{
    char msg[512];
    void    fillMsgOut();
};