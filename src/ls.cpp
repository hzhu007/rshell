#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <algorithm>

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
void long_list_display(const char* addr, const vector<char*> &files);
void handle_ls(const char* addr);
int main(int argc, char** argv)
{
    get_param(argc, argv);
    char addr[1000];
    if(addr_input)
        strcpy(addr, "./");
    else
        strcpy(addr, "./");
    if(addr[strlen(addr)-1] != '/')  //ensure address ends with '/'
        strcat(addr, "/");           //to visit certain file with filename
    cout << addr << endl;
    handle_ls(addr);
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

void handle_ls(const char* addr)
{
    DIR* dirp;
    vector<char*> files;
    if(NULL == (dirp = opendir(addr)))
    {
        perror("opendir()");
        exit(1);
    }
    struct dirent* filespecs;
    int errno = 0;
    while(NULL != (filespecs = readdir(dirp)))
    {
        if(!(filespecs->d_name[0]=='.' && !param_a))  //if is hidden and not -a, don't print
            files.push_back(filespecs->d_name);
    }
    if(errno != 0)
    {
        perror("readdir()");
        exit(1);
    }
    if(-1 == closedir(dirp))
    {
        perror("closedir()");
        exit(1);
    }
    sort(files.begin(), files.end());
    if(param_l)
        long_list_display(addr, files);
    else
    {
        for(int i = 0; i < files.size(); ++i)
            cout << files.at(i) << "  ";
        cout << endl;
    }
}

void long_list_display(const char* addr, const vector<char*> &files)
{
    vector<char*> path;
    char pathTemp[1000];
    for(int i = 0; i < files.size(); ++i)
    {
        strcpy(pathTemp, addr);
        strcat(pathTemp, files.at(i));
        path.push_back(pathTemp);
        cout << files.at(i) << " " << path.at(i) << endl;
    }
    struct stat buf;
    for(int i = 0; i < files.size(); ++i)
    {
        stat(path.at(i), &buf);
        cout << ((S_ISDIR(buf.st_mode)) ? "d":"-")
             << ((buf.st_mode & S_IRUSR)? "r":"-")
             << ((buf.st_mode & S_IWUSR)? "w":"-")
             << ((buf.st_mode & S_IXUSR)? "x":"-")
             << ((buf.st_mode & S_IRGRP)? "r":"-")
             << ((buf.st_mode & S_IWGRP)? "w":"-")
             << ((buf.st_mode & S_IXGRP)? "x":"-")
             << ((buf.st_mode & S_IROTH)? "r":"-")
             << ((buf.st_mode & S_IWOTH)? "w":"-")
             << ((buf.st_mode & S_IXOTH)? "x":"-") << " ";

        cout << files.at(i) << endl;
    }
}
