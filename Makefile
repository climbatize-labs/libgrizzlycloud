SRC_FILES   := $(shell find src/ -type f -name *.c)
OBJ_FILES   := $(SRC_FILES:.c=.o)
LIB         := libgrizzlycloud.a
CFLAGS      += -g -Wall -Isrc/include -Ideps/libjson-c -Ideps/libssl/include -Ideps/libev

$(LIB): $(OBJ_FILES)
	ar rcs $@ $^
	ranlib $@

get-deps:
	git clone https://github.com/GrizzlyCloud/libssl.git deps/libssl/
	git clone https://github.com/GrizzlyCloud/libev.git  deps/libev/
	git clone https://github.com/GrizzlyCloud/json-c.git deps/libjson-c/

build-deps:
	cd deps/libssl/ && ./config -DPURIFY && make
	cd deps/libev/ && ./configure && make
	cd deps/libjson-c/ && git checkout json-c-0.11-20130402 && ./configure && make

clean:
	rm -f $(OBJ_FILES) $(LIB)
