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

#define BLUE printf("\x1b[34;1m")
#define GREEN printf("\x1b[32;1m")
#define GRAY printf("\x1b[48;5;243m")
#define RESET printf("\x1b[0m")

static unsigned int max_col;                //terminal width
static bool param_a = false;       //-a show hidden
static bool param_l = false;       //-l long list
static bool param_R = false;       //-R recursion
static bool newLine = false;       //output a new line every time after file display
static bool addr_input = false;    //whether input address
static bool file_input = false;    //whether input file
static bool addr_display = false;  //show directory

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
void get_param(int argc, char** argv, vector<char*> &v_addr, vector<char*> &v_files);
void handle_files(const vector<char*> &v_files);
void handle_addr(const char* addr);
void long_list_display(const char* addr, const vector<char*> &files);
void norm_display(const char* addr, const vector<char*> &files);
void format(unsigned &row, unsigned &col, const vector<char*> &files, unsigned* maxLen);

int main(int argc, char** argv)
{
    struct winsize w;
    if(-1 == ioctl(STDOUT_FILENO, TIOCGWINSZ, &w))   //get column of terminal
    {
        perror("ioctl()");
        exit(1);
    }
    max_col = min((int)w.ws_col, 132);
    //max_col = w.ws_col;
    //printf ("lines %d\n", w.ws_row);
    //printf ("columns %d\n", max_col);
    vector<char*> v_addr;  //vector storing input file address
    vector<char*> v_files;
    get_param(argc, argv, v_addr, v_files);
    //long_list_display("", v_addr);
    //exit(0);
    if(file_input)
    {
        handle_files(v_files);
    }
    for(unsigned i = 0; i < v_addr.size(); ++i)
        handle_addr(v_addr.at(i));
    return 0;
}

void get_param(int argc, char** argv, vector<char*> &v_addr, vector<char*> &v_files)
{
    char* addr;
    struct stat buf;
    for(int i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {//handle as a flag
            for(unsigned j = 1; j < strlen(argv[i]); ++j)
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
            if(-1 == stat(argv[i], &buf))
            {
                perror("stat()");
                cerr << argv[i] << " not found" << endl;
                addr_input = true;
                continue;
            }
            addr = new char[strlen(argv[i])+1];
            strcpy(addr, argv[i]);
            if(S_ISDIR(buf.st_mode))
            {
                addr_input = true;
                v_addr.push_back(addr);
            }
            else if(S_ISREG(buf.st_mode))
            {
                file_input = true;
                v_files.push_back(addr);
            }
        }
    }
    if(!(addr_input|file_input))
    {//use current directory as default if no address or file input
        addr = new char[2];
        strcpy(addr, ".");
        v_addr.push_back(addr);
    }
    if(v_addr.size() > 1 || param_R)    //if -R or multiple address
        addr_display = true;            //display the directory
    return;
}

void handle_files(const vector<char*> &v_files)
{
    if(param_l)
        long_list_display("", v_files);
    else
        norm_display("", v_files);
    newLine = true;
    addr_display = true;
    return;
}

void handle_addr(const char* addr)
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
    if(files.size() == 0)
        return;
    sort(files.begin(), files.end(), sortFunc);
    if(newLine)
        cout << endl;
    if(addr_display)
        cout << addr << ":" << endl;
    if(param_l)
        long_list_display(addr, files);
    else
        norm_display(addr, files);
    newLine = true;
    if(param_R)
    {
        struct stat *buf = new struct stat[files.size()];
        char path[10000];
        for (unsigned i = 0; i < files.size(); ++i)
        {
            if(strcmp(files.at(i), ".") == 0 ||
               strcmp(files.at(i), "..") == 0)
               continue;
            strcpy(path, addr);
            if(addr[strlen(addr)-1] != '/')  //ensure address ends with '/'
                strcat(path, "/");           //to visit certain file with filename
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
                handle_addr(path);
                //cout << path << endl;
            }
            //cout << files.at(i) << endl;
        }
        delete[] buf;
    }
    for(unsigned i = 0; i < files.size(); ++i)
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
    struct stat *buf = new struct stat[files.size()];
    for(unsigned i = 0; i < files.size(); ++i)
    {
        pathTemp = new char[10000];
        strcpy(pathTemp, addr);
        if(strlen(addr) != 0 &&           //if addr is a directory,
            addr[strlen(addr)-1] != '/')  //ensure address ends with '/'
            strcat(pathTemp, "/");        //to visit certain file with filename
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
        linkBuf = new char[10];
        sprintf(linkBuf, "%lu", buf[i].st_nlink);
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
        sprintf(sizeBuf, "%lu", buf[i].st_size);
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
    char *pch, *filename;
    for(unsigned i = 0; i < files.size(); ++i)
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
        filename = files.at(i);
        pch = strtok(files.at(i), "/");
        while (pch != NULL){
            filename = pch;
            pch = strtok(NULL, "/");
        }
        if(filename[0] == '.')
            GRAY;
        if(buf[i].st_mode & S_IXUSR)
            GREEN;
        if(S_ISDIR(buf[i].st_mode))
            BLUE;
        printf("%s", files.at(i));
        RESET;
        printf("\n");
    }
    delete[] buf;
    return;
}

void norm_display(const char* addr, const vector<char*> &files)
{//normally display the files in current addr
    struct stat *buf = new struct stat[files.size()];
    char* path;
    unsigned int col, row;  //row number and column number
    unsigned* col_len = new unsigned[100];  //width of each column
    //cout << files.size() << endl;
    format(row, col, files, col_len);
    //cout << row << " " << col << endl;
    //for(int i = 0; i < 3; ++i)
    //    cout << col_len[i] << " ";
    //exit(0);
    for(unsigned i = 0; i < files.size(); ++i)
    {
        path = new char[10000];
        strcpy(path, addr);
        if(strlen(addr) != 0 &&           //if addr is a directory,
            addr[strlen(addr)-1] != '/')  //ensure address ends with '/'
            strcat(path, "/");            //to visit certain file with filename
        strcat(path, files.at(i));
        if(-1 == stat(path, &buf[i]))
        {
            perror("stat()");
            cerr << path << " not found" << endl;
            continue;
        }
        delete[] path;
    }
    char *pch, *fileTemp, *fileName;
    for(unsigned r = 0; r < row; ++r)
    {
        fileTemp = new char[strlen(files.at(r))+1];
        fileName = files.at(r);
        strcpy(fileTemp, files.at(r));
        pch = strtok(fileTemp, "/");
        while (pch != NULL){
            fileName = pch;
            pch = strtok(NULL, "/");
        }
        if(fileName[0] == '.')
        //if(files.at(r)[0] == '.')
            GRAY;
        delete[] fileTemp;
        if(buf[r].st_mode & S_IXUSR)
            GREEN;
        if(S_ISDIR(buf[r].st_mode))
            BLUE;
        printf("%-*s", col_len[0], files.at(r));
        RESET;
        //exit(0);
        for(unsigned c = 1; c < col && c*row+r < files.size(); ++c)
        {
            printf("  ");
            fileTemp = new char[strlen(files.at(c*row+r))];
            fileName = files.at(c*row+r);
            strcpy(fileTemp, files.at(c*row+r));
            pch = strtok(fileTemp, "/");
            while (pch != NULL){
                fileName = pch;
                pch = strtok(NULL, "/");
            }
            if(fileName[0] == '.')
                GRAY;
            delete[] fileTemp;
            if(buf[c*row+r].st_mode & S_IXUSR)
                GREEN;
            if(S_ISDIR(buf[c*row+r].st_mode))
                BLUE;
            printf("%-*s", col_len[c], files.at(c*row+r));
            RESET;
        }
        //cout << files.at(r);
        printf("\n");
    }
    //cout << endl;
    delete[] col_len;
    delete[] buf;
    return;
}

void format(unsigned &row, unsigned &col, const vector<char*> &files, unsigned* maxLen)
{//get the row number, column number and column width
    //max length of a column
    unsigned row_len;  //length of a row
    row = 1;
    while(1)
    {
        row_len = 0;
        for(int i = 0; i < 100; ++i)
        {
            maxLen[i] = 0;
        }
        col = ceil(files.size() / (double)row);
        for(unsigned c = 0; c < col; ++c)
        {//colomn
            for(unsigned r = 0; c*row+r < files.size() && r < row; ++r)
            {//row
                maxLen[c] = max(maxLen[c], (unsigned)strlen(files.at(c*row+r)));
                //cout << "strlen: " << strlen(files.at(c*row+r)) << endl;
            }
            //printf("column width: %d\n", maxLen[c]);
            row_len += maxLen[c];
        }
        row_len += 2 * (col - 1);
        //cout << row_len << endl;
        if(row_len <= max_col)
            return;
        ++row;
    }
}
