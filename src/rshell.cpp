#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

void display_info();
char* command_input();
void execution(char* command);
void handle_command(char* command);

static bool lastSucc = true;    //if last command executed successfully
static bool jumpCmd = false;    //if ignore the next command

int main()
{
    char* command;
    while(1)
    {
        display_info();
        command = command_input();
        if(command == NULL)
        {
            continue;
        }
        handle_command(command);
    }
    delete[] command;
    return 0;
}

void display_info()    // print prompt "[rshell]user@host $ "
{
    char* userName;
    char hostName[100];
    if(NULL == (userName = getlogin()))
    {
        perror("getlogin()");
        exit(1);
    }
    if(-1 == gethostname(hostName, 100))
    {
        perror("gethostname()");
        exit(1);
    }
    printf("[rshell]%s@%s $ ", userName, hostName);
}

char* command_input()   //get input and implement some preprocessing
{
    string input;
    getline(cin, input);    //store input in a string
    char* input_cstr = new char [input.length()+1];
    strcpy(input_cstr, input.c_str());
    int countSpace = 0;
    for (int i = 0; *(input_cstr+i) != '\0'; ++i)   //replace the first '#' with terminal sign '\0'
    {
        if (*(input_cstr+i) == '#')
        {
            *(input_cstr+i) = '\0';
            break;
        }
    }
    for (int i = 0; *(input_cstr+i) != '\0'; ++i)   //remove all space and tab before the command
    {
        if (*(input_cstr+i) == ' ' || *(input_cstr+i) == '\t')
            ++countSpace;
        else
            break;
    }
    input_cstr += countSpace;
    return input_cstr;
}

void execution(char* command)    //deal with one single command
{
    //if(0 == strcmp(command, "exit"))
    //    exit(0);
    //else if(0 == strncmp(command, "exit ", 5))
    //    exit(0);
    char* temp;
    char* argv[1000];
    vector<char*> in_redir;
    vector<char*> out_redir;
    for(int i = 0; i < strlen(command); ++i)
    {
        if(command[i] == '>')
        {
            int begin = i+1;    //begin index of file name
            while(command[begin] == ' ' || command[begin] == '\0')
            {
                if(command[begin] == '\0')
                {
                    cerr << "Need input after '>'" << endl;
                    exit(1);
                }
                ++begin;
            }
            int end = begin;    //end index of file name
            while(command[end+1] != ' ' && command[end+1] != '\0')
            {
                ++end;
            }
            temp = new char[end-begin+2];
            strncpy(temp, command+begin, end-begin+1);
            temp[end-begin+1] = '\0';
            out_redir.push_back(temp);
            memset(command+i, ' ', end-i+1);
        }
    }
    cout << "temp: " << temp << endl;
    cout << "command: " << command << endl;
    exit(0);
    int i = 0;
    argv[i] = strtok(command, " ");    //split one command and flags into several cstring when meet ' '
    while(NULL != argv[i])             //store the command in argv[0] and flags in successive cstring
    {
        ++i;
        argv[i] = strtok(NULL, " ");
    }
    if(0 == strcmp(argv[0], "exit"))
        exit(0);

    pid_t pid = fork();
    if(-1 == pid)    //fork error
    {
        perror("fork() in execution()");
        exit(1);
    }
    else if(0 == pid)   //child process
    {
        if(out_redir.size() != 0)
        {
            int fd;
            if(-1 == (fd = open(out_redir.at(0), O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IRWXU)))
            {
                perror("open() in execution()");
                exit(1);
            }
            if(-1 == dup2(fd, 1))
            {
                perror("dup2() in execution()");
                exit(1);
            }
        }
        if(-1 == execvp(argv[0], argv))    //execute one single command, if succeed auto terminate with exit(0)
        {
            perror("execvp() in execution()");
        }
        exit(1);    //only execute when execvp() doesn't succeed
    }
    else    //parent process
    {
        int childStatus;    //used to store the child process's exit status
        if(-1 == waitpid(pid, &childStatus, 0))
            perror("wait() in execution()");    //wait error
        if(WEXITSTATUS(childStatus) == 1)   //child process's exit value is 1
        {                                   //meaning that the command isn't executed correctly
            lastSucc = false;
        }
        else if(WEXITSTATUS(childStatus) == 0)  //child process's exit value is 0
        {                                       //meaning that the command is executed correctly
            lastSucc = true;
        }
    }
    for(int i = 0; i < in_redir.size(); ++i)
    {
        delete[] in_redir.at(i);
    }
    for(int i = 0; i < out_redir.size(); ++i)
    {
        delete[] out_redir.at(i);
    }
    return;
}

void handle_command(char* command)  //handle the command or commands
{
    char* curr_cmd = command;   //pointer to a single command to be executed
    jumpCmd = false;    //if ignore the next command, default false
    lastSucc = true;    //if last command executed successfully, default true
    int i = 0;
    while(*(command+i) != '\0')  //traverse the command, execute one by one
    {                            //every time meeting ";", "&&", "||" and "\0"
        if(*(command+i) == ';')
        {
            if(jumpCmd)    //if jump flag is true, ignore the current command
            {
                curr_cmd = command+i+1; //point to the next command
            }
            else
            {
                *(command+i) = '\0';    //replace ';' with terminal sign '\0'
                execution(curr_cmd);    //execute the command before ';'
                curr_cmd = command+i+1; //point to the next command after ';'
            }
            jumpCmd = false;            //directly execute the next command, don't jump
        }
        else if(*(command+i) == '&' && *(command+i+1) == '&')
        {
            if(jumpCmd)
            {
                curr_cmd = command+i+2;
            }
            else
            {
                *(command+i) = '\0';    //replace the first '&' with terminal sign '\0'
                ++i;
                execution(curr_cmd);    //execute the command before "&&"
                curr_cmd = command+i+1; //point to the next command after "&&"
            }
            jumpCmd = !lastSucc;        //jump if current command isn't executed successfully
        }
        else if(*(command+i) == '|' && *(command+i+1) == '|')
        {
            if(jumpCmd)
            {
                curr_cmd = command+i+2;
            }
            else
            {
                *(command+i) = '\0';    //replace the first '|' with terminal sign '\0'
                ++i;
                execution(curr_cmd);    //execute the command before "||"
                curr_cmd = command+i+1; //point to the next command after "||"
            }
            jumpCmd = lastSucc;         //jump if current command is executed successfully
        }
        ++i;
        if(*(command+i) == '\0')        //reach the end of the command
        {
            if(!jumpCmd)
            {
                execution(curr_cmd);
            }
        }
    }
    return;
}
