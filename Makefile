FLAGS=
CFLAGS = -Wall -Wextra $(FLAGS)-lpthread -Isrc -g -O0

BIN = test/01-main.c \
	test/12-join-main.c \
	test/23-create-many-once.c \
	test/02-switch.c \
	test/21-create-many.c \
	test/31-switch-many.c \
	test/11-join.c \
	test/22-create-many-recursive.c \
	test/32-switch-many-join.c \
	test/51-fibonacci.c \
	test/52-big-sum.c \
	test/62-preemption.c \

NB_THREAD = 2
NB_YIELD = 4
NB_FIBO = 4
NB_BIGSUM = 100
NB_SORT = 100


all: tests

# TEST D'EXECUTION #
tests: test01 test02 test11 test12 test21 test22 test23 test31 test32 test51 test52 test53 test62

test01: thread test/01-main.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/01-main.c -o 01

test02: thread test/02*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/02*.c -o 02

test11: thread test/11*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/11*.c -o 11

test12: thread test/12*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/12*.c -o 12

test21: thread test/21*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/21*.c -o 21

test22: thread test/22*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/22*.c -o 22

test23: thread test/23*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/23*.c -o 23

test31: thread test/31*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/31*.c -o 31

test32: thread test/32*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/32*.c -o 32

test51: thread test/51*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/51*.c -o 51

test52: thread test/52*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/52*.c -o 52

test53: thread test/53*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/53*.c -o 53

test61: thread test/61*.c src/thread.c
	@gcc $(CFLAGS) build/thread.o test/61*.c -o 61

test62: thread test/62*.c src/thread.c
	@gcc $(CFLAGS) -DPREEMPTION src/thread.c test/62*.c -o 62

test63: thread test/62*.c src/thread.c
	@gcc $(CFLAGS) -DPREEMPTION src/thread.c test/63*.c -o 63

thread: src/thread.c
	@gcc $(CFLAGS) -c src/thread.c
	@mv thread.o build/

# TEST COMPARAISON P_THREAD #

check: tests ptest01 ptest02 ptest11 ptest12 ptest21 ptest22 ptest23 ptest31 ptest32 ptest51 ptest52 ptest53 ptest61 ptest62 ptest63

ptest01: test01 src/thread.c
	@./test.sh 01
ptest02: test02 src/thread.c
	@./test.sh 02 
ptest11: test11 src/thread.c
	@./test.sh 11
ptest12: test12 src/thread.c
	@./test.sh 12
ptest21: test21 src/thread.c
	@./test.sh 21 $(NB_THREAD)
ptest22: test22 src/thread.c
	@./test.sh 22 $(NB_THREAD)
ptest23: test23 src/thread.c
	@./test.sh 23 $(NB_THREAD) 
ptest31: test31 src/thread.c
	@./test.sh 31 $(NB_THREAD) $(NB_YIELD)
ptest32: test32 src/thread.c
	@./test.sh 32 $(NB_THREAD) $(NB_YIELD)
ptest51: test51 src/thread.c
	@./test.sh 51 $(NB_FIBO)
ptest52: test52 src/thread.c
	@./test.sh 52 $(NB_BIGSUM)
ptest53: test53 src/thread.c
	@./test.sh 53 $(NB_SORT)
ptest61: test61 src/thread.c
	@./test.sh 61 $(NB_THREAD)
ptest62: test62 src/thread.c
	@./62 
ptest63: test63 src/thread.c
	@./63 

clean:
	@rm -rf build/* correct/* src/*~ src/#* test/*~ test/#* ./01* ./02* ./11* ./12* ./21* ./22* ./23* ./31* ./32* ./51* ./52* ./53* ./61* ./62* ./63*

.PHONY: all clean 

