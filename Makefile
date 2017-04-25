CFLAGS = -Wall -Wextra -lpthread -Isrc

BIN = test/01-main.c \
	test/12-join-main.c \
	test/23-create-many-once.c \
	test/51-fibonacci.c \
	test/02-switch.c \
	test/21-create-many.c \
	test/31-switch-many.c \
	test/61-mutex.c \
	test/11-join.c \
	test/22-create-many-recursive.c \
	test/32-switch-many-join.c

all: $(BIN)

tests: test01 test02 test11 test12 test21 test22 test23 test31 test32 test51 test61

test01: thread
	gcc $(CFLAGS) build/thread.o test/01-main.c -o 01

test02: thread
	gcc $(CFLAGS) build/thread.o test/02*.c -o 02

test11: thread
	gcc $(CFLAGS) build/thread.o test/11*.c -o 11

test12: thread
	gcc $(CFLAGS) build/thread.o test/12*.c -o 12

test21: thread
	gcc $(CFLAGS) build/thread.o test/21*.c -o 21

test22: thread
	gcc $(CFLAGS) build/thread.o test/22*.c -o 22

test23: thread
	gcc $(CFLAGS) build/thread.o test/23*.c -o 23

test31: thread
	gcc $(CFLAGS) build/thread.o test/31*.c -o 31

test32: thread
	gcc $(CFLAGS) build/thread.o test/32*.c -o 32

test51: thread
	gcc $(CFLAGS) build/thread.o test/51*.c -o 51

test61: thread
	gcc $(CFLAGS) build/thread.o test/61*.c -o 61

thread:
	gcc $(CFLAGS) -c src/thread.c
	mv thread.o build/
clean:
	$(RM) $(BIN) *~

.PHONY: all clean 

