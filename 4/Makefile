CC = g++ -Wno-write-strings
SERVER_FILE = simple_server.cpp
HTTP_SERVER_FILE = http_server.cpp
LOAD_GEN_FILE = load_gen.c

all : load_gen server
server: $(SERVER_FILE) $(HTTP_SERVER_FILE)
	$(CC) $(SERVER_FILE) $(HTTP_SERVER_FILE) -g -o server
	
load_gen: $(LOAD_GEN_FILE)
	gcc -g $(LOAD_GEN_FILE) -pthread -o load_gen

clean:
	rm -f server load_gen server *.log
