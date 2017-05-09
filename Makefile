CFLAGS = -Wall -Wextra -lpthread -Isrc -g -O0

BIN = test/01-main.c \
	test/12-join-main.c \
	test/23-create-many-once.c \
	test/51-fibonacci.c \
	test/02-switch.c \
	test/21-create-many.c \
	test/31-switch-many.c \
	test/11-join.c \
	test/22-create-many-recursive.c \
	test/32-switch-many-join.c \
	test/52-big-sum.c \

NB_THREAD_1 = 2
NB_THREAD_2 = 10
NB_THREAD_3 = 100
NB_THREAD_4 = 1000
NB_THREAD_5 = 5000

NB_YIELD_1 = 4
NB_YIELD_2 = 20
NB_YIELD_3 = 100
NB_YIELD_4 = 500

NB_FIBO_1 = 4
NB_FIBO_2 = 8
NB_FIBO_3 = 12

NB_BIGSUM_1 = 100
NB_BIGSUM_2 = 500
NB_BIGSUM_3 = 1000

NB_SORT_1 = 100
NB_SORT_2 = 500
NB_SORT_3 = 1000

all: tests

# TEST D'EXECUTION #
tests: test01 test02 test11 test12 test21 test22 test23 test31 test32 test51 test52 test53 test61

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

test52: thread
	gcc $(CFLAGS) build/thread.o test/52*.c -o 52

test53: thread
	gcc $(CFLAGS) build/thread.o test/53*.c -o 53

test61: thread
	gcc $(CFLAGS) build/thread.o test/61*.c -o 61

thread:
	gcc $(CFLAGS) -c src/thread.c
	mv thread.o build/

# TEST COMPARAISON P_THREAD #

check: tests ptest01 ptest02 ptest11 ptest12 ptest21 ptest22 ptest23 ptest31 ptest32 ptest51 ptest52 ptest53 ptest61

ptest01: test01 
	./test.sh 01
ptest02: test02 
	./test.sh 02
ptest11: test11 
	./test.sh 11
ptest12: test12 
	./test.sh 12
ptest21: test21 
	./test.sh 21 $(NB_THREAD_1)
	./test.sh 21 $(NB_THREAD_2)
	./test.sh 21 $(NB_THREAD_3)
	./test.sh 21 $(NB_THREAD_4)
	./test.sh 21 $(NB_THREAD_5)
ptest22: test22 
	./test.sh 22 $(NB_THREAD_1)
	./test.sh 22 $(NB_THREAD_2)
	./test.sh 22 $(NB_THREAD_3)
# 	./test.sh 22 $(NB_THREAD_4)
#	./test.sh 22 $(NB_THREAD_5) 
ptest23: test23 
	./test.sh 23 $(NB_THREAD_1)
	./test.sh 23 $(NB_THREAD_2)
	./test.sh 23 $(NB_THREAD_3)
	./test.sh 23 $(NB_THREAD_4)
	./test.sh 23 $(NB_THREAD_5) 
ptest31: test31
	./test.sh 31 $(NB_THREAD_1) $(NB_YIELD_1)
	./test.sh 31 $(NB_THREAD_2) $(NB_YIELD_4)
	./test.sh 31 $(NB_THREAD_3) $(NB_YIELD_3)
	./test.sh 31 $(NB_THREAD_4) $(NB_YIELD_2)
# 	./test.sh 31 $(NB_THREAD_5) $(NB_YIELD_4)
ptest32: test32 
	./test.sh 32 $(NB_THREAD_1) $(NB_YIELD_4)
	./test.sh 32 $(NB_THREAD_2) $(NB_YIELD_2)
	./test.sh 32 $(NB_THREAD_3) $(NB_YIELD_1)
# 	./test.sh 32 $(NB_THREAD_4) $(NB_YIELD_2)
# 	./test.sh 32 $(NB_THREAD_5) $(NB_YIELD_3)
ptest51: test51 
	./test.sh 51 $(NB_FIBO_1)
	./test.sh 51 $(NB_FIBO_2)
	./test.sh 51 $(NB_FIBO_3)
ptest52: test52
	./test.sh 52 $(NB_BIGSUM_1)
	./test.sh 52 $(NB_BIGSUM_2)
#	./test.sh 52 $(NB_BIGSUM_3)
ptest53: test53
	./test.sh 53 $(NB_SORT_1)
	./test.sh 53 $(NB_SORT_2)
#	./test.sh 53 $(NB_SORT_3)
ptest61: test61 
	./test.sh 61 $(NB_THREAD_1)
	./test.sh 61 $(NB_THREAD_2)
	./test.sh 61 $(NB_THREAD_3)
#	./test.sh 61 $(NB_THREAD_4)
#	./test.sh 61 $(NB_THREAD_5)

clean:
	rm -rf build/* src/*~ src/#* test/*~ test/#* ./01* ./02* ./11* ./12* ./21* ./22* ./23* ./31* ./32* ./51* ./52* ./53* ./61*

.PHONY: all clean 

