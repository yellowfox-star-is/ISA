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

secret: secret.o error.o error.h bit.h arguments.o arguments.h networking.o networking.h client.h client.o smrcka_bat.o
	gcc $(CFLAGS) secret.o error.o arguments.o networking.o smrcka_bat.o client.o -o secret

secret.o: secret.c error.h bit.h networking.h arguments.h
	gcc $(CFLAGS) -c secret.c -o secret.o

error.o: error.h error.c
	gcc $(CFLAGS) -c error.c -o error.o

arguments.o: arguments.h arguments.c networking.h error.h bit.h smrcka_bat.h
	gcc $(CFLAGS) -c arguments.c -o arguments.o

networking.o: networking.h networking.c smrcka_bat.h
	gcc $(CFLAGS) -c networking.c -o networking.o

client.o: client.h client.c smrcka_bat.h
	gcc $(CFLAGS) -c client.c -o client.o

smrcka_bat.o: smrcka_bat.c smrcka_bat.h
	gcc $(CFLAGS) -c smrcka_bat.c -o smrcka_bat.o