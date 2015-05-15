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
void get_redirection(char* command, vector<struct redirection*> &v_redir);
void handle_redirect(const vector<struct redirection*> &v_redir);

static bool lastSucc = true;    //if last command executed successfully
static bool jumpCmd = false;    //if ignore the next command

struct redirection
{
    char* fileName;
    int type;   //0 - >; 1 - >>; 2 - <; 3 - <<<
    int fd;     //STDIN_FILENO = 0; STDOUT_FILENO = 1; STDERR_FILENO = 2;
    redirection(const char* fileName, const int type, const int fd)
    {
        this->fileName = new char[strlen(fileName)+1];
        strcpy(this->fileName, fileName);
        this->type = type;
        this->fd = fd;
    }
    ~redirection()
    {
        delete[] this->fileName;
    }
};

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
        delete[] command;
    }
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
    char input_cstr[input.length()+1];
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
    char* command = new char[input.length()+1];
    strcpy(command, input_cstr+countSpace);
    return command;
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

void execution(char* command)    //deal with one single command
{
    char *lhs, *rhs = NULL;
    for(int i = 0; i < strlen(command); ++i)
    {
        if(command[i] == '|')
        {
            lhs = command;
            command[i] = '\0';
            rhs = command+i+1;
            //cout << "L: " << lhs << " R: " << rhs << endl;
            //exit(0);
            const int PIPE_READ = 0;
            const int PIPE_WRITE = 1;
            int fd[2];
            if(-1 == pipe(fd))
            {
                perror("pipe() in execution()");
                exit(1);
            }
            int pid = fork();
            if(-1 == pid)
            {
                perror("fork() in execution()");
                exit(1);
            }
            else if(pid == 0)
            {//child process, handle left-hand side command
                //cout << "in child" << endl;
                if(-1 == dup2(fd[PIPE_WRITE], 1))
                {
                    perror("dup2() in execution()");
                    exit(1);
                }
                if(-1 == close(fd[PIPE_READ]))
                {
                    perror("close() in execution()");
                    exit(1);
                }
                execution(lhs);
                exit(0);    //fork() in execution(), the parent process would return here
            }
            else
            {//parent process, handle right-hand side command
                int save_stdin;
                if(-1 == (save_stdin = dup(0))) //need to restore later or infinite loop
                {
                    perror("dup() in execution()");
                    exit(1);
                }
                if(-1 == dup2(fd[PIPE_READ],0))//make stdin the read end of the pipe
                {
                    perror("dup2() in execution()");
                    exit(1);
                }
                if(-1 == close(fd[PIPE_WRITE]))//close the write end of the pipe because we're not doing anything with it right now
                {
                    perror("close() in execution()");
                    exit(1);
                }
                if(-1 == wait(0)) //wait for the child process to finish executing
                {
                    perror("wait() in execution()");
                    exit(1);
                }
                execution(rhs);
                if(-1 == dup2(save_stdin,0))//restore stdin
                {
                    perror("There is an error with dup2. ");
                    exit(1);
                }
                return;
            }
        }
    }

    char* argv[1000];
    vector<struct redirection*> v_redir;
    get_redirection(command, v_redir);
    //for(int i = 0; i < v_redir.size(); ++i)
    //{
    //    cout << "temp: " << v_redir.at(i) << endl;
    //}
    //cout << "command: " << command << endl;
    //exit(0);
    int i = 0;
    argv[i] = strtok(command, " ");    //split one command and flags into several cstring when meet ' '
    while(NULL != argv[i])             //store the command in argv[0] and flags in successive cstring
    {
        ++i;
        argv[i] = strtok(NULL, " ");
    }
    if(NULL == argv[0])
        return;
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
        handle_redirect(v_redir);
        //cout << argv[0] << endl;
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
        if(WEXITSTATUS(childStatus) != 0)   //child process's exit value is not 0
        {                                   //meaning that the command isn't executed correctly
            lastSucc = false;
        }
        else if(WEXITSTATUS(childStatus) == 0)  //child process's exit value is 0
        {                                       //meaning that the command is executed correctly
            lastSucc = true;
        }
    }
    //for(int i = 0; i < in_redir.size(); ++i)
    //{
    //    delete[] in_redir.at(i);
    //}
    return;
}

void get_redirection(char* command, vector<struct redirection*> &v_redir)
{
    struct redirection* pstruct;
    char nameTemp[1000];
    for(int i = 0; i < strlen(command); ++i)
    {//traverse one single command to find redirect signs
        if(command[i] == '>' && command[i+1] != '>')    //meet single >
        {//output redirection >
            int begin = i + 1;    //begin index of file name
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
            while(command[end+1] != ' ' && command[end+1] != '\0'
                    && command[end+1] != '>' && command[end+1] != '<')
            {
                ++end;
            }
            strncpy(nameTemp, command+begin, end-begin+1);
            nameTemp[end-begin+1] = '\0';
            int fd = STDOUT_FILENO;
            if(i != 0 && isdigit(command[i-1]) && (i == 1 || command[i-2] == ' '))
            {
                //cout << "Get fd! " << endl;
                fd = command[i-1] - '0';
                memset(command+i-1, ' ', end-i+2);
            }
            else
                memset(command+i, ' ', end-i+1);
            //cout << command << endl;
            pstruct = new struct redirection(nameTemp, 0, fd);
            v_redir.push_back(pstruct);
        }
        else if(command[i] == '>' && command[i+1] == '>')    //meet >>
        {//output redirection >>
            int begin = i + 2;    //begin index of file name
            while(command[begin] == ' ' || command[begin] == '\0')
            {
                if(command[begin] == '\0')
                {//cannot detect any character
                    cerr << "Need input after \">>\"" << endl;
                    exit(1);
                }
                ++begin;
            }
            int end = begin;    //end index of file name
            while(command[end+1] != ' ' && command[end+1] != '\0'
                    && command[end+1] != '>' && command[end+1] != '<')
            {
                ++end;
            }
            strncpy(nameTemp, command+begin, end-begin+1);
            nameTemp[end-begin+1] = '\0';
            int fd = STDOUT_FILENO;
            if(i != 0 && isdigit(command[i-1]) && (i == 1 || command[i-2] == ' '))
            {
                //cout << "Get fd" << endl;
                fd = command[i-1] - '0';
                memset(command+i-1, ' ', end-i+2);
            }
            else
                memset(command+i, ' ', end-i+1);
            pstruct = new struct redirection(nameTemp, 1, fd);
            v_redir.push_back(pstruct);
        }
        else if(command[i] == '<' && command[i+1] == '<' && command[i+2] == '<')
        {//input redirection <<<
            int begin = i + 3;    //begin index of file name
            while(command[begin] == ' ' || command[begin] == '\0')
            {
                if(command[begin] == '\0')
                {
                    cerr << "Need input after \"<<<\"" << endl;
                    exit(1);
                }
                ++begin;
            }
            if(command[begin] != '\"')
            {
                cerr << "Need a string quoted by \" after \"<<<\"" << endl;
                exit(1);
            }
            int end = strlen(command) - 1;    //end index of file name
            while(command[end] != '\"')
            {
                --end;
            }
            if(end == begin)
            {
                cerr << "Need a string quoted by \" after \"<<<\"" << endl;
                exit(1);
            }
            strncpy(nameTemp, command+begin+1, end-begin-1);
            nameTemp[end-begin-1] = '\0';
            pstruct = new struct redirection(nameTemp, 3, STDIN_FILENO);
            v_redir.push_back(pstruct);
            memset(command+i, ' ', end-i+1);
        }
        else if(command[i] == '<')
        {//input redirection <
            int begin = i + 1;    //begin index of file name
            while(command[begin] == ' ' || command[begin] == '\0')
            {
                if(command[begin] == '\0')
                {
                    cerr << "Need input after '<'" << endl;
                    exit(1);
                }
                ++begin;
            }
            int end = begin;    //end index of file name
            while(command[end+1] != ' ' && command[end+1] != '\0'
                    && command[end+1] != '>' && command[end+1] != '<')
            {
                ++end;
            }
            strncpy(nameTemp, command+begin, end-begin+1);
            nameTemp[end-begin+1] = '\0';
            pstruct = new struct redirection(nameTemp, 2, STDIN_FILENO);
            v_redir.push_back(pstruct);
            memset(command+i, ' ', end-i+1);
        }
    }
    return;
}

void handle_redirect(const vector<struct redirection*> &v_redir)
{
    if(v_redir.size() == 0)
        return;
    for(unsigned i = 0; i < v_redir.size(); ++i)
    {
        int fd;

        const int PIPE_READ = 0;
        const int PIPE_WRITE = 1;
        int fd2[2];
        int strLen = strlen(v_redir.at(i)->fileName)+1;
        char temp[strLen+1];

        switch(v_redir.at(i)->type)
        {
            /* > */
            case 0:
                fd = open(v_redir.at(i)->fileName, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
                if(-1 == fd)
                {
                    perror("open() in handle_redirect()");
                    exit(1);
                }
                if(-1 == dup2(fd, v_redir.at(i)->fd))
                {
                    perror("dup2() in handle_redirect()");
                    exit(1);
                }
                break;
            /* >> */
            case 1:
                fd = open(v_redir.at(i)->fileName, O_CREAT|O_RDWR|O_APPEND, S_IRUSR|S_IWUSR);
                if(-1 == fd)
                {
                    perror("open() in handle_redirect()");
                    exit(1);
                }
                if(-1 == dup2(fd, v_redir.at(i)->fd))
                {
                    perror("dup2() in handle_redirect()");
                    exit(1);
                }
                break;
            /* < */
            case 2:
                fd = open(v_redir.at(i)->fileName, O_RDONLY);
                if(-1 == fd)
                {
                    perror("open() in handle_redirect()");
                    exit(1);
                }
                if(-1 == dup2(fd, v_redir.at(i)->fd))
                {
                    perror("dup2() in handle_redirect()");
                    exit(1);
                }
                break;
            /* <<< */
            case 3:
                if(-1 == pipe(fd2))
                {
                    perror("pipe() in handle_redirect()");
                    exit(1);
                }
                strcpy(temp, v_redir.at(i)->fileName);
                strcat(temp, "\n");
                //cout << temp;
                if(-1 == write(fd2[PIPE_WRITE], temp, strLen+1))
                {
                    perror("write() in handle_redirect()");
                    exit(1);
                }
                if(-1 == close(fd2[PIPE_WRITE]))
                {
                    perror("close() in handle_redirect()");
                    exit(1);
                }
                if(-1 == dup2(fd2[PIPE_READ], 0))
                {
                    perror("dup2() in handle_redirect()");
                    exit(1);
                }
                break;
        }
        v_redir.at(i)->~redirection();
    }
}

