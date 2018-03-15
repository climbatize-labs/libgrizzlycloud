JSON_C_DIR=../../deps/libjson-c
SSL_DIR=../../deps/libssl
EV_DIR=../../deps/libev
GC_DIR=../../

LIBS    := -l:libgrizzlycloud.a -l:libev.a -lm -l:libjson-c.a -l:libssl.a -l:libcrypto.a -ldl
INCS    := -I. -I$(GC_DIR)/src/include -I$(SSL_DIR)/include -I$(JSON_C_DIR) -I$(EV_DIR)
LIBDIR  := -L$(GC_DIR) -L$(SSL_DIR) -L$(JSON_C_DIR)/.libs/ -L$(EV_DIR)/.libs/
BIN     := grizzlycloud

all: lib client

rebuild: clean lib client

lib:
	make -C ../../

client:
	$(CC) -g -Wall $(INCS) *.c $(LIBDIR) $(LIBS) -o $(BIN)

lib_clean:
	make clean -C ../../

client_clean:
	rm -f ./$(BIN)

clean: lib_clean client_clean
