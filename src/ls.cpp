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
#include <grp.h>
#include <pwd.h>

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

bool sortFunc(char* s1, char* s2){return strcmp(s1, s2) < 0;};
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
    sort(files.begin(), files.end(), sortFunc);
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
    char* pathTemp;
    vector<char*> linkNum;
    char* linkBuf;
    int max_link = 0;
    vector<char*> userName;
    int max_uname = 0;
    vector<char*> groupName;
    int max_grname = 0;
    struct passwd *ppwd;
    struct group *pgr;
    vector<char*> mtime;
    struct tm *ptm;
    char* timeBuf;
    vector<char*> fsize;
    char* sizeBuf;
    int max_size = 0;
    int max = 0;
    struct stat buf[files.size()];

    for(int i = 0; i < files.size(); ++i)
    {
        pathTemp = new char[1000];
        max = std::max(max, (int)strlen(files.at(i)));
        strcpy(pathTemp, addr);
        strcat(pathTemp, files.at(i));
        path.push_back(pathTemp);
        if(-1 == stat(path.at(i), &buf[i]))
        {
            perror("stat()");
            exit(1);
        }
        /* hard links */
        linkBuf = new char[5];
        sprintf(linkBuf, "%d", buf[i].st_nlink);
        linkNum.push_back(linkBuf);
        max_link = std::max(max_link, (int)strlen(linkNum.at(i)));
        /* user name */
        ppwd = getpwuid(buf[i].st_uid);
        userName.push_back(ppwd->pw_name);
        max_uname = std::max(max_uname, (int)strlen(userName.at(i)));
        /* group name */
        pgr = getgrgid(buf[i].st_gid);
        groupName.push_back(pgr->gr_name);
        max_grname = std::max(max_grname, (int)strlen(groupName.at(i)));
        /* size in bytes */
        sizeBuf = new char[100];
        sprintf(sizeBuf, "%d", buf[i].st_size);
        fsize.push_back(sizeBuf);
        max_size = std::max(max_size, (int)strlen(fsize.at(i)));
        /* time of last modification */
        timeBuf = new char[50];
        ptm = localtime(&buf[i].st_mtime);
        strftime(timeBuf, 50, "%b %d %H:%M", ptm);
        mtime.push_back(timeBuf);
    }
    //for(int i = 0; i < files.size(); ++i)
    //{
    //    cout << files.at(i) << " ";
    //    cout << path.at(i) << endl;
    //}
    for(int i = 0; i < files.size(); ++i)
    {
        cout << ((S_ISDIR(buf[i].st_mode)) ? "d":"-")
             << ((buf[i].st_mode & S_IRUSR)? "r":"-")
             << ((buf[i].st_mode & S_IWUSR)? "w":"-")
             << ((buf[i].st_mode & S_IXUSR)? "x":"-")
             << ((buf[i].st_mode & S_IRGRP)? "r":"-")
             << ((buf[i].st_mode & S_IWGRP)? "w":"-")
             << ((buf[i].st_mode & S_IXGRP)? "x":"-")
             << ((buf[i].st_mode & S_IROTH)? "r":"-")
             << ((buf[i].st_mode & S_IWOTH)? "w":"-")
             << ((buf[i].st_mode & S_IXOTH)? "x":"-") << " ";
        printf("%*s ", max_link, linkNum.at(i));
        printf("%*s ", max_uname, userName.at(i));
        printf("%*s ", max_grname, groupName.at(i));
        printf("%*s ", max_size, fsize.at(i));
        printf("%s ", mtime.at(i));
        printf("%s\n", files.at(i));
    }
}
