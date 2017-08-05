all:    clean
	mkdir -p bin
	gcc -Wall -o bin/main src/*.c

PHONY: all clean
	
clean:
	rm -R -f bin