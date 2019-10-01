FROM ubuntu:18.04
RUN apt-get update  && apt-get install -y \
	build-essential \
        autoconf
COPY . /app
WORKDIR /app
RUN ./autogen.sh
RUN ./configure
RUN make
EXPOSE 8125
CMD ./statsd-proxy -f ./example.cfg

