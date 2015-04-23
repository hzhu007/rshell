#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

//#define PARAM_NONE  0
//#define PARAM_A     1
//#define PARAM_L     2
//#define PARAM_R     3

static bool param_a = false;
static bool param_l = false;
static bool param_R = false;
static bool param_none = true;
static bool addr_input = false;


void get_param(int argc, char** argv);
void handle_ls();
int main(int argc, char** argv)
{
    get_param(argc, argv);
    handle_ls();
    return 0;
}

void get_param(int argc, char** argv)
{
    for(int i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            for(int j = 1; j < strlen(argv[i]); ++j)
            {
                if(argv[i][j] == 'a')
                    param_a = true;
                else if(argv[i][j] == 'l')
                    param_l = true;
                else if(argv[i][j] == 'R')
                    param_R = true;
                else
                {
                    cerr << "wrong parameter: " << argv[i][j]
                        << " in " << argv[i] << endl;
                    exit(1);
                }
            }
        }
        else
        {
            addr_input = true;
        }
    }
    param_none = !(param_a|param_l|param_R);
}

void handle_ls()
{
    DIR* dirp;

    if(NULL == (dirp = opendir("./")))
    {
        perror("opendir()");
        exit(1);
    }
    struct dirent* filespecs;
    int errno = 0;
    while(NULL != (filespecs = readdir(dirp)))
    {
        cout << filespecs->d_name << "  ";
    }
    if(errno != 0)
    {
        perror("readdir()");
        exit(1);
    }
    cout << endl;
    if(-1 == closedir(dirp))
    {
        perror("closedir()");
        exit(1);
    }
}
