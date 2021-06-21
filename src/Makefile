CC		= gcc

EXEC	= Main
SRCS	= Main.c
OBJS	= Main.o
LIBS	= Chat.a Error.a -lpthread

default: ${EXEC}

${EXEC}: ${LIBS}
	${CC} -c ${SRCS}
	${CC} ${OBJS} ${LIBS} -o $@ -g

${LIBS}:
	make -C Chat
	make -C Error

clean:
	${RM} ${EXEC} ${OBJS}
	make clean -C Chat
	make clean -C Error
