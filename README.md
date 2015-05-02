#Rshell
##Introduction
###rshell
This program is a command shell called *rshell*. Rshell uses syscalls ```fork()```, ```waitpid()``` and ```execvp()``` to execute commands from user input. It can perform the following steps:

1. Print a command prompt `[rshell]username@hostname $ `

2. Read in a command on one line. Commands will have the form:

    ```
    cmd         = executable [ argumentList ] [ connector cmd ];
    connector   = || or && or ;
    ```
where executable is an executable program in the PATH and argumentList is a list of zero or more arguments separated by spaces. The connector is an optional way you can run multiple commands at once. If a command is followed by ```;```, then the next command is always executed; if a command is followed by ```&&```, then the next command is executed only if the first one succeeds; if a command is followed by ```||```, then the next command is executed only if the first one fails.

Anything between two connectors(including the beginning and the end of an input line) would be considered as an executable command and be passed into ```execvp()```. For example, ```;;```would be considered as three empty commands. And ```&&&```would be considered as an empty command and ```&```, which is a wrong command, connected by ```&&```. Notice this is **different** from original bash.

Anything that appears after a ```#``` character will be considered as comment.
###ls
I also implement my own ```ls``` which is a subset of the GNU ```ls``` command.

In my ```ls```, only ```-a```(show hidden), ```-l```(long list format) and ```-R```(recursively) optional flags are supported and it has the ability deal with zero, one, or many files. Files and folders will be handled differently, the same as original GNU ```ls``` command.

Also it will display different types of files in different colors. To be more specific, it print directories in blue, executables in green, and hidden files (with the ```-a``` flag only) with a gray background. These effects should be combinable. So if you have a hidden directory, it should be displayed as blue text on top of a gray background.

## Prerequisites
OS: Linux

g++ compiler

## Installation
####rshell
    git clone https://github.com/hzhu007/rshell.git
    cd rshell
    git checkout hw0
    make
    bin/rshell
####ls
    git clone https://github.com/hzhu007/rshell.git
    cd rshell
    git checkout hw1
    make
    bin/ls

##Test cases
###rshell
    # test comment
    # test a wrong command
        hahaha
    # test one command:
        ls
    # test comment after a command
        ls#ls
        ls # ls
    # test with one parameter:
        ls -a
    # test with one parameter and a wrong flag
        ls -hahaha
    # test with one parameter and spaces :
            ls      -l
    # test with two parameters:
        ls -a -l
        ls -al
    # test with three parameters:
        ls -a -l -h
        ls -alh
    # test ;
        ls;ls
        ls;;;ls;;;;pwd
        ls; ;; ;; ;;;;;ls -a;
    # test &&
        ls && ls -a
        wrong && ls
        &&
        &&&&
        ls&&&&ls
        ls&&wrong&&ls
        &&&
        ls   &&   ls
    # test ||
        ls||pwd
        wrong || pwd
        pwd || wrong
        ||
        ||||
        |||
        pwd || ls    ||     pwd
    # test combination
        ls -a; echo hello  && echo world || pwd;  git   status
    # test commented exit
        #exit
    # test spaces before commented exit
           #exit
    # regular exit
        exit
    # rshell is back up when this is run
    # test exit appended with invalid input
        exitwqew
    # test exit with parameters
        exit --flag -a num 123
###ls
    # no param
    $ bin/ls

    #one file, multiple files
    $ bin/ls src/a.out
    $ bin/ls src/a.out/
    $ bin/ls nonexist
    $ bin/ls nonexist/
    $ bin/ls nonexist src/a.out
    $ bin/ls nonexist/ src/a.out/ Makefile
    $ bin/ls scr/a.out ./README.md  ../.vimrc Makefile ./src/ls.cpp
    $ bin/ls src/a.out src/a.out src/a.out
    $ bin/ls src/a.out src/a.out src/a.out Makefile  ../.vimrc

    #one folder, multiple folders
    $ bin/ls src
    $ bin/ls ../test/core/
    $ bin/ls ../test/core
    $ bin/ls ./ src
    $ bin/ls ../ ./   src/  bin .git
    $ bin/ls ../  ../test/core/  .git

    #file and folder
    $ bin/ls src/a.out ./
    $ bin/ls src/a.out ./ Makefile ../.vimrc
    $ bin/ls src/a.out src ./ ../  Makefile ../.vimrc
    $ bin/ls ../ scr/a.out ../test/core/ ./src/ls.cpp  .git  Makefile
    $ bin/ls nonexist src/a.out
    $ bin/ls nonexist src/a.out
    $ bin/ls nonexist ./  src/a.out src/a.out Makefile  ../.vimrc
    $ bin/ls nonexist ./ src/a.out ../ nonexist ./README.md ../.vimrc Makefile src/a.out

    #flag
    $ bin/ls -a
    $ bin/ls -l
    $ bin/ls -L
    $ bin/ls -R
    $ bin/ls -r
    $ bin/ls a
    $ bin/ls - a
    $ bin/ls -l a
    $ bin/ls -Rl a
    $ bin/ls -al
    $ bin/ls -l   -a
    $ bin/ls -Rl
    $ bin/ls -l   -R
    $ bin/ls -error
    $ bin/ls -a -l -error

    #file with flag
    $ bin/ls Makefile -a
    $ bin/ls -l Makefile
    $ bin/ls Makefile -R
    $ bin/ls Makefile   -al
    $ bin/ls Makefile   ./src/ls.cpp   -l -a
    $ bin/ls src/a.out   ./README.md   ../.vimrc Makefile   ./src/ls.cpp -Ra -l
    $ bin/ls src/a.out   ./README.md -a  ../.vimrc Makefile  -R ./src/ls.cpp  -l

    #folder with flag
    $ bin/ls . -a
    $ bin/ls  ../ -l
    $ bin/ls ~/ucr-cs100/textbook   -R
    $ bin/ls ../.vim -lR
    $ bin/ls ../.vim/    -la
    $ bin/ls -l ../.vim/  -a
    $ bin/ls ../ucr-cs100/people -aR
    $ bin/ls ../ucr-cs100/people  -R  -a
    $ bin/ls ../.vim  -lRa
    $ bin/ls ../.vim  -l -R -a
    $ bin/ls ../ucr-cs100/textbook -aR -l
    $ bin/ls ../ucr-cs100/textbook -aR ./src src/ -l bin -l -R -a ../ucr-cs100/people

    #combination
    $ bin/ls ./ -a src/ls.cpp
    $ bin/ls ./ -a src/ls.cpp -l ..
    $ bin/ls ../ucr-cs100/textbook -Rl src/ls.cpp ..
    $ bin/ls ../ucr-cs100/textbook -al Makefile ../.vim/ -R
    $ bin/ls -R scr/a.out   ./README.md   ../.vimrc Makefile   ./src/ls.cpp -Ra -l  ../ucr-cs100/textbook .git -aR ./src src/ -l bin -l -R -a
## Known bugs
When ```exit``` appears after a connector with spaces before it, it doesn't work properly. For example, when input ```ls;  exit``` only ```ls``` will be executed and *rshell* won't exit.
When files in my folder are too many (more than 150), my ```ls``` may crash (seem to use up stack).
