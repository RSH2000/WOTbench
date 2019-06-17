LIBRARIES = -lcoap-1 -lpthread
INCLUDE = -I/usr/local/include/coap/ -I./
CXXFLAGS = -O2 -D_GNU_SOURCE -DWITH_POSIX

all: ge
	
ge:GatewayEmulator.c
	gcc GatewayEmulator.c coap_list.c $(INCLUDE) $(LIBRARIES) $(CXXFLAGS)  -g -o  GatewayEmulator
clean:
	rm ./GatewayEmulator