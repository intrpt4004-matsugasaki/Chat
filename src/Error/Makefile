AR = ar -cq

LIB = ../Error.a
OBJS = Error.o

default: ${LIB}

${LIB} : ${OBJS}
	${RM} $@
	${AR} $@ ${OBJS}

clean:
	${RM} ${OBJS}
	${RM} ${LIB}
