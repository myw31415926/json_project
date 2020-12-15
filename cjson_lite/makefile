# cjson makefile

all: cjson_test

cjson_test: cjson_test.c cjson.c
	gcc -g -Wall -o $@ $^ -lm

clean:
	rm -rf cjson_test 
