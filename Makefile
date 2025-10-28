
INCLUDE=
C_FILES=
O_FILES=$(patsubst %.c,%.o,${C_FILES})

NAME=ircserv

SUBJECT_FLAGS=--std=98 -Wall -Wextra -Werror
SHARED_FLAGS=${SUBJECT_FLAGS} -fsanitize=address,undefined

COMPILING_FLAGS=${SHARED_FLAGS} ${INCLUDE} -g3
LINKING_FLAGS=${SHARED_FLAGS}

all: ${NAME}

${NAME}: ${O_FILES}
	c++ -o ${NAME} ${LINKING_FLAGS} ${O_FILES}

%.o: %.c
	c++ -c ${COMPILING_FLAGS} $? -o $@
