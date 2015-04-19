MFLAGS = -Wall -Werror -ansi -pedantic

rshell: ./src/main.cpp
	mkdir bin
	g++ $(MFLAGS) ./src/main.cpp -o bin/rshell

all: rshell

clean:
	rm -rf bin
