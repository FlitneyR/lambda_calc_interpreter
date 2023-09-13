INCLUDES = -Isrc/headers

LFLAGS = -g
CFLAGS = $(INCLUDES) -Wall -std=c++2a -g -c

bin/main: build/main.o build/ast.o build/evaluator.o build/interpreter.o
	clang++ $(LFLAGS) -o $@ $^

build/%.o: src/%.cpp src/headers/%.hpp
	clang++ $(CFLAGS) -o $@ $<

build/%.o: src/%.cpp
	clang++ $(CFLAGS) -o $@ $<

memtest: bin/main
	leaks -atExit -- bin/main

bench: bin/main
	time bin/main

run: bin/main
	bin/main

clean:
	rm -f build/* bin/*
