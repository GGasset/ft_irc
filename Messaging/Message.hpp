#ifndef MESSAGE_H
# define MESSAGE_H 

# include "Server.hpp"
# include <iostream>
# include <string>
# include <vector>
# include <cassert>
# include <string.h>
# include <ctype.h>
# include <assert.h>
# include <memory>
# include "LexerMessage.hpp"

/* Tokenizador */
msgs getMsgs(std::string packet);
std::string getSPACE(std::string &packet, size_t &beginSpace);
std::string getTRAIL(std::string &packet, size_t &beginWord);
std::string getWORD(std::string &packet, size_t &beginWord);
bool isNUMBER(const std::string &param);
char iterStr(const std::string& str);
msgTokens msgTokenizer(std::string msg);
void newSPACE(msgTokens &ret, std::string &msg, size_t &begin);

/* Parser */
COMMAND getCMD(const std::string &cmd);
std::string	getCommandname(COMMAND cmd);
inline bool isCharInSet(char c, const std::string& set);
inline bool isInNospcrlfcl(const std::string& str);
inline bool isInNospcrlfclTRAIL(const std::string& str);
inline bool isReservedChar(char c);
inline bool isSpecialChar(char c);
bool isUserChar(char c);
bool isValidHostName(const std::string &hostaname);
bool isValidNickName(const std::string &nickname); 
bool isValidServerName(const std::string& name);
bool isValidUserName(const std::string &username);

#endif