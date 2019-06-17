LIBRARIES = -lm
INCLUDE = -I./

all: wl_generator 
	
wl_generator:WLG.c
	gcc WLG.c  $(INCLUDE) $(LIBRARIES)  -g -o  WOT-WLG
	
clean:
	rm ./WOT-WLG
