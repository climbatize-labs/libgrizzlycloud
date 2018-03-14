SRC_FILES   := $(shell find src/ -type f -name *.c)
OBJ_FILES   := $(SRC_FILES:.c=.o)
LIB         := libgrizzlycloud.a
CFLAGS      += -g -Wall -Isrc/include -I../libssl/include

$(LIB): $(OBJ_FILES)
	ar rcs $@ $^
	ranlib $@

clean:
	rm -f $(OBJ_FILES) $(LIB)
