all:
	gcc -Wall -o bin/ffile src/sample/ffile.c src/core/compile.c src/core/search.c src/core/prefix_tree.c src/encoding/utf_8.c

PHONY: all clean configure sample

configure:
	mkdir -p bin

clean:
	rm -R -f bin/*

sample:	clean
	gcc -Wall -o bin/sample src/sample/sample.c src/core/compile.c src/core/search.c src/core/prefix_tree.c src/encoding/utf_8.c