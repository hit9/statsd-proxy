# vim:set noet:

default: statsd-proxy

.DEFAULT:
	make $@ -C src
