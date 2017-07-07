PHONY: help all clean

all:    clean
	mkdir -p bin
	gcc -Wall -o bin/main src/*.c
	
clean:
	rm -R -f bin