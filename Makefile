MFLAGS = -Wall -Werror -ansi -pedantic

all: ./src/ls.cpp ./src/main.cpp
	mkdir bin
	g++ $(MFLAGS) ./src/ls.cpp -o bin/ls
	g++ $(MFLAGS) ./src/main.cpp -o bin/rshell

rshell: ./src/main.cpp
	mkdir bin
	g++ $(MFLAGS) ./src/main.cpp -o bin/rshell

ls: ./src/ls.cpp
	mkdir bin
	g++ $(MFLAGS) ./src/ls.cpp -o bin/ls

cp: ./src/cp.cpp ./src/Timer.h
	mkdir bin
	g++ $(MFLAGS) ./src/cp.cpp -o bin/cp

run: ./bin/rshell
	./bin/rshell

clean:
	rm -rf bin
