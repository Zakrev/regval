PHONY: help all clean

all:
	mkdir -p bin
	gcc -Wall -o bin/main src/*.c
	
clean:
	rm -R -f bin