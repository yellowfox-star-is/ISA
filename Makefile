CFLAGS = -pedantic -Wall -Wextra

debug: CFLAGS += -g
debug: rebuild

all: secret

clean:
	rm -f *.o secret

rebuild: all

archive: tar
tar:
	tar -cvf xsysel09.tar *.c *.h secret.1 manual.pdf Makefile

secret: secret.o error.o error.h
	gcc $(CFLAGS) secret.o error.o -o secret

secret.o: secret.c
	gcc $(CFLAGS) -c secret.c -o secret.o

error.o: error.h error.c
	gcc $(CFLAGS) -c error.c -o error.o

