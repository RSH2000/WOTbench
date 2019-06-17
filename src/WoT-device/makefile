LIBRARIES = -lcoap-1 -lpthread
INCLUDE = -I/usr/local/include/coap/ -I./
CXXFLAGS = -O2 -D_GNU_SOURCE -DWITH_POSIX

all: server 
	
server:server.c
	gcc server.c thpool.c $(INCLUDE) $(LIBRARIES) $(CXXFLAGS)  -g -o  CDB-server
	
clean:
	rm ./CDB-server
