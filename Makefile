MFLAGS = -Wall -Werror -ansi -pedantic

rshell: ./src/main.cpp
	mkdir bin
	g++ $(MFLAGS) ./src/main.cpp -o bin/rshell

cp: ./src/cp.cpp ./src/Timer.h
	g++ $(MFLAGS) ./src/cp.cpp -o bin/cp

all: rshell cp

run: ./bin/rshell
	./bin/rshell

clean:
	rm -rf bin
