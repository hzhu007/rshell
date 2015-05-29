#Rshell
##Introduction
###rshell
This program is a command shell called *rshell*. Rshell uses syscalls ```fork()```, ```waitpid()``` and ```execvp()``` to execute commands from user input.
It can perform the following steps:

1. Print a command prompt `[rshell]username@hostname $ `

2. Read in a command on one line. Commands will have the form:

    ```
    cmd         = executable [ argumentList ] [ connector cmd ];
    connector   = || or && or ;
    ```
where executable is an executable program in the PATH and argumentList is a list of zero or more arguments separated by spaces.
The connector is an optional way you can run multiple commands at once.
If a command is followed by ```;```, then the next command is always executed; if a command is followed by ```&&```, then the next command is executed only if the first one succeeds; if a command is followed by ```||```, then the next command is executed only if the first one fails.

Anything between two connectors(including the beginning and the end of an input line) would be considered as an executable command and be passed into ```execvp()```. For example, ```;;```would be considered as three empty commands.
And ```&&&```would be considered as an empty command and ```&```, which is a wrong command, connected by ```&&```.
Notice this is **different** from original bash.

Anything that appears after a ```#``` character will be considered as comment.

>Redirection and Piping
>Rshell is extended so that it properly handles input redirection ```<``` and ```<<<```(string input redirection need to be quoted by a pair of ```"``` and anything in a pair of quote except connectors will be considered as a string), output redirection ```>``` and ```>>```(also support arbitrary file descriptors within ten you want by placing a number before the ```>``` command), and piping ```|```.

###ls
I also implement my own ```ls``` which is a subset of the GNU ```ls``` command.

In my ```ls```, only ```-a```(show hidden), ```-l```(long list format) and ```-R```(recursively) optional flags are supported and it has the ability deal with zero, one, or many files.
Files and folders will be handled differently, the same as original GNU ```ls``` command.

Also it will display different types of files in different colors.
To be more specific, it print directories in blue, executables in green, and hidden files (with the ```-a``` flag only) with a gray background.
These effects should be combinable.
So if you have a hidden directory, it should be displayed as blue text on top of a gray background.

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
###Redirection and piping
    #input redirection <
    cat < src/Timer.h
    grep cout<src/Timer.h
       cat    <   src/Timer.h
    cat < nonexist
    cat <
    <  src/Timer.h
      <  src/test
    cat<src/ls.cpp<src/cp.cpp<  src/mv.cpp  <  src/rshell.cpp  <  src/Timer.h

    #input redirection <<<
        cat   <<<    "haha"
    grep 2 -a<<<"1a 2b 3c 4d"
    cat <<< """"
       cat <<< """
    cat <<<
    <<<
        cat  <<<"""<<< "haha" <<<"""<<<"hoho" <<< "!"

    #output redirection >
    ls #There is not a or b
       ls   >    a
    ls;cat a
    ls -al>b
    ls;cat b
    ls ---  >a
    cat a
    ls>
    > c
    cat c
    >
    ls -al > a1>a2 > a3 >   a4   >a5
    ls;cat a1;cat a2;cat a3;cat a4
    cat a5
    cat a
    ls --- 0>a
    cat a
    ls --- 1>a
    cat a
    ls  1>a
    cat a
    ls --- 3>b
    cat b
    cat c
    ls ---2>c
    cat c
    ls ---  2>c
    cat c
    ls -al 2> b12>b2 > b3 >   b4   2>b5
    ls
    cat b12;cat b2;cat b3
    cat b4
    cat b5

    #output redirection >>
      ls   >>  d
    ls;cat d
    cat a
    ls -l>>a
    cat a
    ls -w >> d
    cat d
    ls>>
    cat c
    >> c
    cat c
    >>
    ls -a >> a1>>a2 >> a3 >>   a4   >>a5
    cat a1;cat a2;cat a3;cat a4
    cat a5
    ls --- 0>>a
    cat a
    ls --- 1>>a
    cat a
    ls  1>>a
    cat a
    cat b
    ls --- 3>>b
    cat b
    ls ---2>>c
    cat c
    cat a
    ls ---  2>>a
    cat a

    #pipe
    ls|cat
      ls  |  cat
    |
      |ls
    ls|
    cat LICENSE| grep IR  #long file test
    ls | ls -al src | cat | tr a-z A-Z |grep CPP

    #combination
    cat a
    cat < a | tr A-Z a-z|tee new1  |  tr a-z A-Z > new2
    cat new1
    cat new2
    cat <<< "wow!" cpp" | cat > x>> a|ls -al 2> b |   grep cpp |   tr a-z A-Z | sort -n >>c  | cat c
    cat x
    cat a
    cat b
    cat tests/ls.script | grep I #ls.script is a long text more than 20K
    cat tests/ls.script | grep I | cat tests/ls.script |grep --

###cd and signals
    # cd <path>
    $ cd ./
    $ cd .
    $ cd ../
    $ cd   rshell/src
    $ cd ../test/
    $ cd /
    $ cd .////
    $ cd .////home///csmajs/..///

    # cd -
    $ cd csmajs/hzhu007/rshell/ #go back
    $ cd -
    $ cd -

    # cd
    $ cd
    $ pwd
    $ cd -

    # interrupt
    $ ^C
    $ ps
    $ do.out
    $ ^C
    $ ps
    $ hello.out
    $ ^C
    $ ps

    # STOP
    $ ^Z
    $ ps
    $ do.out
    $ ^Z
    $ ps
    $ fg
    $ ^Z
    $ do.out
    $ ^Z
    $ hello.out
    $ ^Z
    $ ps
    $ fg
    $ ^C
    $ ps
    $ fg
    $ ^C
    $ ps
    $ bg
    $ kill do.out

## Known bugs
When ```exit``` appears after a connector with spaces before it, it doesn't work properly
. For example, when input ```ls;  exit``` only ```ls``` will be executed and *rshell* won't exit.

When files in my folder are too many (more than 150), my ```ls``` may crash (seems to use too much `new` and use up heap).

In my program `^C` will only print a new line while there is no job running in foreground.
But if I stop a process and send it to background then press `^C`, it is supposed to do nothing and in fact it merely print a newline in this case.
However then I input `fg`, the background process mysteriously kills itself.
    #Example code
    $ do.out
    $ ^Z
    $ ps
    $ ^C  #simply print a new line here
    $ ps  #background process still exists
    $ fg  #should bring it back to foreground
    $ ps  #process disappears
