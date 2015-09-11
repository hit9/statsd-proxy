# vim:set noet:

NAME    = statsd-proxy
CC      = cc
CFLAGS  =
LDFLAGS = -pthread
SRC     = buf.c\
		  ctx.c\
		  cfg.c\
		  config.c\
		  event.c\
		  ketama.c\
		  log.c\
		  md5.c\
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
