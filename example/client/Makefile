LIBS    := -lgrizzlycloud -lssl -lcrypto -lev -lm
INCS    := -I../../src/include
LIBDIR  := -L../../
BIN     := grizzlycloud

all: lib client

rebuild: clean lib client

lib:
	make -C ../../

client:
	$(CC) $(INCS) main.c $(LIBDIR) $(LIBS) -o $(BIN)

lib_clean:
	make clean -C ../../

client_clean:
	rm -f ./$(BIN)

clean: lib_clean client_clean
