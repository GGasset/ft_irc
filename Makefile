
INCLUDE=-I Authentication -I Messaging -I Messaging/Channels -I Socket

SOCKET_CPP_FILES=function_router.cpp server_loop.cpp Server.cpp

OPERATOR_CPP_FILES=invite.cpp  kick.cpp  mode.cpp  topic.cpp
CHANNEL_CPP_FILES=Channel.cpp $(addprefix Operators/,${OPERATOR_CPP_FILES})
MESSAGING_CPP_FILES=$(addprefix Channels/,${CHANNEL_CPP_FILES}) MessageOut.cpp fnHandlers.cpp Message.cpp Param.cpp ParserMessage.cpp

CPP_FILES=$(addprefix Socket/,${SOCKET_CPP_FILES}) $(addprefix Socket/,${MESSAGING_CPP_FILES})
O_FILES=$(patsubst %.cpp,%.o,${CPP_FILES})

NAME=ircserv

SUBJECT_FLAGS= #-Wall -Wextra -Werror #--std=c++98
SHARED_FLAGS=${SUBJECT_FLAGS} -fsanitize=address,undefined

COMPILING_FLAGS=${SHARED_FLAGS} ${INCLUDE} -g3
LINKING_FLAGS=${SHARED_FLAGS} -lc

all: ${NAME}

${NAME}: ${O_FILES}
	c++ -o ${NAME} ${LINKING_FLAGS} ${O_FILES}

%.o: %.cpp
	c++ -c ${COMPILING_FLAGS} $? -o $@

re: fclean all

fclean: clean
	rm -f ${NAME}

clean:
	rm -f ${O_FILES}

.PHONY: all re fclean clean
