
INCLUDE=-I Authentication -I Messaging -I Messaging/Channels -I Socket

SOCKET_CPP_FILES=function_router.cpp server_loop.cpp Server.cpp

CPP_FILES=$(addprefix Socket/,${SOCKET_CPP_FILES})
O_FILES=$(patsubst %.cpp,%.o,${CPP_FILES})

NAME=ircserv

SUBJECT_FLAGS=--std=98 -Wall -Wextra -Werror
SHARED_FLAGS=${SUBJECT_FLAGS} -lc -fsanitize=address,undefined

COMPILING_FLAGS=${SHARED_FLAGS} ${INCLUDE} -g3
LINKING_FLAGS=${SHARED_FLAGS}

all: ${NAME}

${NAME}: ${O_FILES}
	c++ -o ${NAME} ${LINKING_FLAGS} ${O_FILES}

%.o: %.c
	c++ -c ${COMPILING_FLAGS} $? -o $@

re: fclean all

fclean: clean
	rm -f ${NAME}

clean:
	rm -f ${O_FILES}

.PHONY: all re fclean clean
