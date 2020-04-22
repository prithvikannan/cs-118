.SILENT:

default: server.c
	gcc -g -o server -Wall -Wextra server.c -lm

clean:
	rm -rf server *.tar.gz

dist: default
	tar -czvf 405110096.tar.gz server.c Makefile README.md
