MFLAGS = -Wall -Werror -ansi -pedantic

rshell: ./src/main.cpp
	mkdir bin
	g++ $(MFLAGS) ./src/main.cpp -o bin/rshell

ls: ./src/ls.cpp
	mkdir bin
	g++ $(MFLAGS) ./src/ls.cpp -o bin/ls

all: ./src/ls.cpp ./src/main.cpp
	mkdir bin
	g++ $(MFLAGS) ./src/ls.cpp -o bin/ls
	g++ $(MFLAGS) ./src/main.cpp -o bin/rshell

run: ./bin/rshell
	./bin/rshell

clean:
	rm -rf bin
