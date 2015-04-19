#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

void display_info();
char* command_input();
void execution(char* command);
void handle_command(char* command);

static bool lastSucc = true;
static bool jumpCmd = false;

int main()
{
    while(1)
    {
        display_info();
        char* command = command_input();
        if(command == NULL)
        {
            continue;
        }

        if(0 == strcmp(command, "exit"))
            exit(1);
        else
        {
            handle_command(command);
        }
    }

    return 0;
}

void display_info()    // print prompt "[rshell]user@host $ "
{
    char* userName;
    char hostName[100];
    userName = getlogin();
    gethostname(hostName, 100);
    printf("[rshell]%s@%s $ ", userName, hostName);
}

char* command_input()
{
    string input;
    getline(cin, input);
    if (input == "")
    {
        return '\0';
    }
    char* input_cstr = new char [input.length()+1];
    strcpy(input_cstr, input.c_str());
    int countSpace = 0;
    for (int i = 0; *(input_cstr+i) != '\0'; ++i)   //remove all space and tab
    {
        if (*(input_cstr+i)==' ' || *(input_cstr+i) == '\t')
            ++countSpace;
        else
            break;
    }
    input_cstr += countSpace;
    return strtok(input_cstr, "#");    //discard all char include and after '#'
}

void execution(char* command)    //deal with one single command
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
    if(-1 == pid)    //fork error
    {
        perror("fork() in execution()");
        exit(1);
    }
    else if(0 == pid)   //child process
    {
        if(-1 == execvp(argv[0], argv))    //execute one single command
        {
            perror("execvp() in execution()");
        }
        exit(1);
    }
    else    //parent process
    {
        int childStatus;
        if(-1 == waitpid(pid, &childStatus, 0))
            perror("wait() in execution()");
        if(WEXITSTATUS(childStatus) == 1)
        {
            lastSucc = false;
        }
        else if(WEXITSTATUS(childStatus) == 0)
        {
            lastSucc = true;
        }
    }
    return;
}

void handle_command(char* command)
{
    char* curr_cmd = command;
    jumpCmd = false;
    lastSucc = true;
    int i = 0;
    while(*(command+i) != '\0')
    {
        if(*(command+i) == ';')
        {
            if(jumpCmd)
            {
                curr_cmd = command+i+1;
            }
            else
            {
                *(command+i) = '\0';
                execution(curr_cmd);
                curr_cmd = command+i+1;
            }
            jumpCmd = false;
        }
        else if(*(command+i) == '&' && *(command+i+1) == '&')
        {
            if(jumpCmd)
            {
                curr_cmd = command+i+2;
            }
            else
            {
                *(command+i) = '\0';
                ++i;
                execution(curr_cmd);
                curr_cmd = command+i+1;
            }
            jumpCmd = !lastSucc;
        }
        else if(*(command+i) == '|' && *(command+i+1) == '|')
        {
            if(jumpCmd)
            {
                curr_cmd = command+i+2;
            }
            else
            {
                *(command+i) = '\0';
                ++i;
                execution(curr_cmd);
                curr_cmd = command+i+1;
            }
            jumpCmd = lastSucc;
        }
        ++i;
        if(*(command+i) == '\0')
        {
            if(!jumpCmd)
            {
                execution(curr_cmd);
            }
        }
    }
    return;
}
