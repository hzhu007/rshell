#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

void display_info();
char* command_input();
void execution(char* command);

int main(int argc, char** argv)
{
    while(1)
    {
        display_info();
        char* command = command_input();
        if(0 == strcmp(command, "exit"))
        //if(0 == strncmp(command, "exit", sizeof(command)))
            break;
        else
        {
            execution(command);
        }
    }

    return 0;
}

void display_info()
{
    char* userName;
    char hostName[100];
    userName = getlogin();
    gethostname(hostName, 100);
    printf("[rshell]%s@%s $ ", userName, hostName); //user@host$
}

char* command_input()
{
    string input;
    getline(cin, input);
    char* input_cstr = new char [input.length()+1];
    strcpy(input_cstr, input.c_str());
    return strtok(input_cstr, "#");
}

void execution(char* command)
{
    char* argv[10000];
    int i = 0;
    argv[i] = strtok(command, " ");
    while(NULL != argv[i])
    {
        ++i;
        argv[i] = strtok(NULL, " ");
    }

    pid_t pid = fork();
    if(-1 == pid)
    {
        perror("fork() in handle_command()");
        exit(1);
    }
    else if(0 == pid)
    {
        if(-1 == execvp(argv[0], argv))
            perror("execvp() in handle_command()");
        exit(1);
    }
    else
    {
        if(-1 == wait(NULL))
            perror("wait() in handle_command()");
    }
    return;
}
