all:    clean
	mkdir -p bin
	gcc -Wall -o bin/single_line  src/sample/single_line.c src/rexpr.c
	gcc -Wall -o bin/multi_line  src/sample/multi_line.c src/rexpr.c

PHONY: all clean
	
clean:
	rm -R -f bin