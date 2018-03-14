LIBS    := -lgrizzlycloud -lev -lm -ljson-c -lssl -lcrypto -ldl
INCS    := -I. -I../../src/include -I../../../libssl/include
LIBDIR  := -L../../ -L../../../libssl/
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
