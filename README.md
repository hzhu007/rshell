# Rshell
## Introduction
This program is a command shell called **rshell**. It can perform the following steps:
1. Print a command prompt
2. Read in a command on one line. Commands will have the form:
''cmd         = executable [ argumentList ] [ connector cmd ]
connector   = || or && or ;''
## Installation
git clone https://github.com/hzhu007/rshell.git
cd rshell
git checkout hw0
make
bin/rshell
## Test cases
'#test comment'
''#test a wrong command''
'''    hahaha'''
#test one command:
    ls
#test with one parameter:
    ls -a
#test with one parameter and a wrong flag
    ls -bang
#test with one parameter and spaces :
    ls      -l
#test with two parameters:
    ls -a -l
    ls -al
#test with three parameters:
    ls -a -l -h
#test ;
    ls;ls
    ls;;;ls;;;;pwd
    ls; ;; ;; ;;;;;ls -a;
#test &&
    ls && ls -a
    wrong && ls
    &&
    &&&&
    ls&&&&ls
    ls&&wrong&&ls
    &&&
    ls   &&   ls
#test ||
    ls||pwd
    wrong || pwd
    pwd || wrong
    ||
    ||||
    |||
    pwd || ls    ||     pwd
#test combination
    ls -a; echo hello &&echo world || pwd; git status
#test commented exit
    #exit
# test spaces before commented exit
    #exit
# regular exit
    exit
# test exit with parameters
      exit --flag -a num 123
