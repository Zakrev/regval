all:	clean
	gcc -Wall -o bin/main src/sample/main.c src/core/compile.c src/core/search.c src/core/prefix_tree.c src/encoding/utf_8.c 

sample_multi:	clean
	gcc -Wall -o bin/multi_line  src/sample/multi_line.c src/rexpr.c

sample_singl:	clean
	gcc -Wall -o bin/single_line  src/sample/single_line.c src/rexpr.c

PHONY: all clean sample_singl sample_multi
	
clean:
	rm -R -f bin/*