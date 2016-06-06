# vim:set noet:

NAME=statsd-proxy
CC=cc -std=c99 -O2 -D_GNU_SOURCE
CFLAGS=
LDFLAGS=-pthread
SRC:=$(wildcard *.c)
EV_EPOLL=$(wildcard event_epoll.c)
EV_TIMER=$(wildcard event_timer.c)
SRC:=$(filter-out $(EV_EPOLL), $(SRC))
SRC:=$(filter-out $(EV_TIMER), $(SRC))
INC:=$(wildcard *.h)
OBJ:=$(SRC:%.c=%.o)
BIN:=$(NAME)

$(BIN): $(OBJ)

clean:
	rm $(OBJ) $(BIN)

default: statsd-proxy
