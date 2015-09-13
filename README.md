Statsd-Proxy
============

Proxy for [etsy/statsd](https://github.com/etsy/statsd).

Why
---

[etsy/statsd](https://github.com/etsy/statsd) comes with a proxy in nodejs,
and we are running it on a single server, proxing a statsd cluster via an
udp port. But we found that this nodejs proxy is lossing packets, even to
30~40% sometimes!

Cpus are idle but packets are lossing, in our case, one api call makes one
statsd request, maybe the single udp socket is too busy.

We tried to use `SO_REUSEPORT` on the original nodejs proxy, this enables
us to bind multiple udp sockets on a single port, but nodejs(or libuv) has
disabled this option, and golang just dosen't have a method `setsockopt()`.

Therefore, we make it in C.

Features
--------

* Zero dependencies.
* Very very fast.
* Multiple threading.
* Reuseport support.
* Packets aggregation.

Limitations
-----------

* Only available on linux 3.9+ (option `SO_REUSEPORT`)
* Only support udp server and udp backends.

Requirements
-------------

Linux 3.9+.

Build
------

Just make:

    $ make

or install it via npm:

    npm install statsd-proxy -g

Usage
-----

    Usage:
      ./statsd-proxy -f ./path/to/config.cfg
    Options:
      -h, --help        Show this message
      -v, --version     Show version
      -d, --debug       Enable debug logging
    Copyright (c) https://github.com/hit9/statsd-proxy

License
-------

MIT (c) Chao Wang <hit9@github.com> 2015.
