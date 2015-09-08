# vim:set noet:

NAME    = statsd-proxy
CC      = cc
CFLAGS  =
LDFLAGS = -pthread
SRC     = buf.c\
		  ctx.c\
		  ketama.c\
		  log.c\
		  utils.c\
		  proxy.c\
		  statsd-proxy.c

OBJ     = $(SRC:c=o)
BIN     = $(NAME)

$(BIN): $(OBJ)

clean:
	rm $(OBJ) $(BIN)

default: statsd-proxy
