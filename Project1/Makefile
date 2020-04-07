.SILENT:

default: server.c
	gcc -g -o server -Wall -Wextra server.c -lm

clean:
	rm -rf server *.tar.gz

dist: default
	tar -czvf lab0-405110096.tar.gz lab0.c README Makefile backtrace.png breakpoint.png checker.sh
