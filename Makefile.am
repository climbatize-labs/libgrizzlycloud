ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -Isrc/include -Ideps/libjson-c -Ideps/openssl/include -Ideps/libev

lib_LTLIBRARIES = libgrizzlycloud.la
libgrizzlycloud_la_SOURCES = \
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

libgrizzlycloud_la_LDFLAGS = -version-info 0:0:0

bin_PROGRAMS = grizzlycloud
grizzlycloud_SOURCES = example/client/client.c
grizzlycloud_LDADD = .libs/libgrizzlycloud.a \
            deps/openssl/libssl.a \
            deps/openssl/libcrypto.a \
            deps/libjson-c/.libs/libjson-c.a \
            deps/libev/.libs/libev.a \
            -ldl -lm

get-deps:
	git submodule update --init --recursive

build-deps:
	cd deps/openssl/ && ./config -d -DPURIFY && make
	cd deps/libev/ && ./configure -d && make
	cd deps/libjson-c/ && libtoolize && aclocal && autoconf && automake --add-missing && ./configure -d && make

uninstalled: get-deps build-deps all
