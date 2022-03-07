all : server client

server : new_server.c data_structure.c rio_package.c constant.h
	gcc -o server new_server.c data_structure.c rio_package.c constant.h -lpthread

client : new_client.c rio_package.c constant.h
	gcc -o client new_client.c rio_package.c constant.h -lpthread

clean :
	rm -rf server client