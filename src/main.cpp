#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string>

using namespace std;

void display_info();

int main(int argc, char** argv)
{
    string input;
    while(1)
    {
        display_info();
        getline(cin, input);
        char* input_cstr = new char [input.length()+1];
        strcpy(input_cstr, input.c_str());
        char* command = strtok(input_cstr, "#");
        if(0 == strncmp(command, "exit", sizeof(command)))
            break;
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

