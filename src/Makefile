# vim:set noet:

NAME    = statsd-proxy
CC      = cc
CFLAGS  =
LDFLAGS = -pthread
SRC     = buf.c\
		  ctx.c\
		  ketama.c\
		  log.c\
		  parser.c\
		  proxy.c\
		  statsd-proxy.c\
		  utils.c\

OBJ     = $(SRC:c=o)
BIN     = $(NAME)

$(BIN): $(OBJ)

clean:
	rm $(OBJ) $(BIN)

default: statsd-proxy
