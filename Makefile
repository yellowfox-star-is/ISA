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

secret: secret.o error.o error.h bit.h arguments.o arguments.h networking.o networking.h
	gcc $(CFLAGS) secret.o error.o arguments.o networking.o -o secret

secret.o: secret.c error.h bit.h networking.h arguments.h
	gcc $(CFLAGS) -c secret.c -o secret.o

error.o: error.h error.c
	gcc $(CFLAGS) -c error.c -o error.o

arguments.o: arguments.h arguments.c networking.h error.h bit.h
	gcc $(CFLAGS) -c arguments.c -o arguments.o

networking.o: networking.h networking.c
	gcc $(CFLAGS) -c networking.c -o networking.o