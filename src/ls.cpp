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
#include <errno.h>
#include <sys/ioctl.h>
#include <cmath>

using namespace std;

static int max_col;
static bool param_a = false;
static bool param_l = false;
static bool param_R = false;
static bool newLine = false;
static bool addr_input = false;
static bool addr_display = false;

bool sortFunc(char* s1, char* s2)
{
    char str1[256];
    char str2[256];
    strcpy(str1, s1);
    strcpy(str2, s2);
    int i = 0;
    while(str1[i])
    {
        str1[i] = tolower(str1[i]);
        i++;
    }
    i = 0;
    while(str2[i])
    {
        str2[i] = tolower(str2[i]);
        i++;
    }
    return strcmp(str1, str2) < 0;
}
void get_param(int argc, char** argv, vector<char*> &v_addr);
void handle_ls(const char* addr);
void long_list_display(const char* addr, const vector<char*> &files);
void norm_display(const vector<char*> &files);
void format(int &row, int &col, const vector<char*> &files, int* maxLen);

int main(int argc, char** argv)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    //max_col = min((int)w.ws_col, 80);
    max_col = w.ws_col;
    //printf ("lines %d\n", w.ws_row);
    //printf ("columns %d\n", w.ws_col);
    vector<char*> v_addr;
    get_param(argc, argv, v_addr);
    for(int i = 0; i < v_addr.size(); ++i)
        handle_ls(v_addr.at(i));
    return 0;
}

void get_param(int argc, char** argv, vector<char*> &v_addr)
{
    char* addr;
    for(int i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {//handle as a flag
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
        {//handle as an address
            addr_input = true;
            addr = new char[strlen(argv[i])+2];
            strcpy(addr, argv[i]);
            if(addr[strlen(addr)-1] != '/')  //ensure address ends with '/'
                strcat(addr, "/");           //to visit certain file with filename
            v_addr.push_back(addr);
        }
    }
    if(!addr_input)
    {//use current directory as default if no address input
        addr = new char[3];
        strcpy(addr, "./");
        v_addr.push_back(addr);
    }
    if(v_addr.size() > 1 || param_R)
        addr_display = true;
    return;
}

void handle_ls(const char* addr)
{
    DIR* dirp;
    vector<char*> files;
    char* filesBuf;
    if(NULL == (dirp = opendir(addr)))
    {

        perror("opendir()");
        cerr << addr << " not found." << endl;
        return;
    }
    struct dirent* filespecs;
    errno = 0;
    while(NULL != (filespecs = readdir(dirp)))
    {
        if(!(filespecs->d_name[0]=='.' && !param_a))  //if is hidden and not -a, don't print
        {
            filesBuf = new char[strlen(filespecs->d_name)+1];
            strcpy(filesBuf, filespecs->d_name);
            files.push_back(filesBuf);
        }
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
    if(newLine)
        cout << endl;
    if(addr_display)
        cout << addr << ":" << endl;
    if(param_l)
        long_list_display(addr, files);
    else
        norm_display(files);
    newLine = true;
    if(param_R)
    {
        struct stat buf[files.size()];
        char path[1000];
        for (int i = 0; i < files.size(); ++i)
        {
            if(strcmp(files.at(i), ".") == 0 ||
               strcmp(files.at(i), "..") == 0)
               continue;
            strcpy(path, addr);
            strcat(path, files.at(i));
            //cout << path << endl;
            if(-1 == stat(path, &buf[i]))
            {
                perror("stat()");
                exit(1);
            }
            if(S_ISDIR(buf[i].st_mode))
            {
                //strcat(path, "/");
                handle_ls(path);
                //cout << path << endl;
            }
            //cout << files.at(i) << endl;
        }
    }
    for(int i = 0; i < files.size(); ++i)
        delete[] files.at(i);
    return;
}

void long_list_display(const char* addr, const vector<char*> &files)
{
    //for (int i = 0; i < files.size(); ++i)
    //{
    //    cout << files.at(i) << endl;
    //}
    vector<char*> path;
    char* pathTemp;
    int total = 0;  //total blocks used
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
    struct stat buf[files.size()];

    for(int i = 0; i < files.size(); ++i)
    {
        pathTemp = new char[1000];
        strcpy(pathTemp, addr);
        strcat(pathTemp, files.at(i));
        path.push_back(pathTemp);
        //cout << pathTemp << endl;
        if(-1 == stat(path.at(i), &buf[i]))
        {
            perror("stat()");
            exit(1);
        }
        total += buf[i].st_blocks;
        /* hard links */
        linkBuf = new char[5];
        sprintf(linkBuf, "%d", buf[i].st_nlink);
        linkNum.push_back(linkBuf);
        max_link = max(max_link, (int)strlen(linkNum.at(i)));
        /* user name */
        errno = 0;
        ppwd = getpwuid(buf[i].st_uid);
        if(errno != 0)
        {
            perror("getpwuid()");
            exit(1);
        }
        userName.push_back(ppwd->pw_name);
        max_uname = max(max_uname, (int)strlen(userName.at(i)));
        /* group name */
        errno = 0;
        pgr = getgrgid(buf[i].st_gid);
        if(errno != 0)
        {
            perror("getgrgid()");
            exit(1);
        }
        groupName.push_back(pgr->gr_name);
        max_grname = max(max_grname, (int)strlen(groupName.at(i)));
        /* size in bytes */
        sizeBuf = new char[100];
        sprintf(sizeBuf, "%d", buf[i].st_size);
        fsize.push_back(sizeBuf);
        max_size = max(max_size, (int)strlen(fsize.at(i)));
        /* time of last modification */
        timeBuf = new char[50];
        ptm = localtime(&buf[i].st_mtime);
        strftime(timeBuf, 50, "%b %d %H:%M", ptm);
        mtime.push_back(timeBuf);
        delete[] pathTemp;
    }
    //for(int i = 0; i < files.size(); ++i)
    //{
    //    cout << files.at(i) << " ";
    //    cout << path.at(i) << endl;
    //}
    cout << "total " << total/2 << endl;
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
        delete[] linkNum.at(i);
        printf("%*s ", max_uname, userName.at(i));
        printf("%*s ", max_grname, groupName.at(i));
        printf("%*s ", max_size, fsize.at(i));
        delete[] fsize.at(i);
        printf("%s ", mtime.at(i));
        delete[] mtime.at(i);
        printf("%s\n", files.at(i));
    }
    return;
}

void norm_display(const vector<char*> &files)
{
    int col, row;  //row number and column number
    int* col_len;  //length of each column
    col_len = new int[30];
    format(row, col, files, col_len);
    //cout << row << " " << col << endl;
    //for(int i = 0; i < 3; ++i)
    //    cout << col_len[i] << " ";
    //exit(0);
    for(int r = 0; r < row; ++r)
    {
        printf("%-*s", col_len[0], files.at(r));
        //exit(0);
        for(int c = 1; c < col && c*row+r < files.size(); ++c)
        {
            printf("  ");
            printf("%-*s", col_len[c], files.at(c*row+r));
        }
        printf("\n");
    }
    //cout << endl;
    delete[] col_len;
    return;
}

void format(int &row, int &col, const vector<char*> &files, int* maxLen)
{
    //max length of a column
    int row_len;  //length of a row
    row = 1;
    while(1)
    {
        row_len = 0;
        for(int i = 0; i < 30; ++i)
        {
            maxLen[i] = 0;
        }
        col = ceil(files.size() / (double)row);
        for(int c = 0; c < col; ++c)
        {//colomn
            for(int r = 0; c*row+r < files.size() && r < row; ++r)
            {//row
                maxLen[c] = max(maxLen[c], (int)strlen(files.at(c*row+r)));
            }
            //printf("%d\n", maxLen[c]);
            row_len += maxLen[c];
        }
        row_len += 2 * (col - 1);
        //cout << row_len << endl;
        if(row_len <= max_col)
            break;
        ++row;
    }
    return;
}
