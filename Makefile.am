#SUBDIRS = src

AM_CPPFLAGS = -Isrc/include -Ideps/libjson-c -Ideps/openssl/include -Ideps/libev

noinst_LIBRARIES = libgrizzlycloud.a
libgrizzlycloud_a_SOURCES = \
    src/async_client.c \
    src/async_server.c \
    src/backend.c \
    src/endpoint.c \
    src/gcapi.c \
    src/hashtable.c \
    src/log.c \
    src/pool.c \
    src/proto.c \
    src/ringbuffer.c \
    src/tunnel.c \
    src/utils.c

bin_PROGRAMS = grizzlycloud
grizzlycloud_SOURCES = example/client/client.c
grizzlycloud_LDADD = libgrizzlycloud.a \
            deps/openssl/libssl.a \
            deps/openssl/libcrypto.a \
            deps/libjson-c/.libs/libjson-c.a \
            deps/libev/.libs/libev.a \
            -ldl -lm

get-deps:
	git clone https://github.com/GrizzlyCloud/openssl.git deps/openssl/
	git clone https://github.com/GrizzlyCloud/libev.git  deps/libev/
	git clone https://github.com/GrizzlyCloud/json-c.git deps/libjson-c/

build-deps:
	cd deps/openssl/ && ./config -DPURIFY && make
	cd deps/libev/ && ./configure && make
	cd deps/libjson-c/ && ./autogen.sh && ./configure && make

uninstalled: get-deps build-deps all
