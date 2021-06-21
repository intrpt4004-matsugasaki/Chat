AR = ar -cq

LIB = ../Chat.a
OBJS = ChatClient.o ChatServer.o ChatData.o

default: ${LIB}

${LIB} : ${OBJS}
	${RM} $@
	${AR} $@ ${OBJS}

clean:
	${RM} ${OBJS}
	${RM} ${LIB}
