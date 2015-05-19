#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Timer.h"
using namespace std;

void cp_stream(const char *src, const char *dest) {
    ifstream ifs(src);
    ofstream ofs(dest);
    char c;
    while (ifs.get(c)) {
        ofs.put(c);
    }
    ifs.close();
    ofs.close();
}

void cp_rdwr(const char *src, const char *dest) {
    int fdnew,fdold;
    if (-1==(fdnew=open(dest,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))) {
        perror("open");
        exit(1);
    }
    if (-1==(fdold=open(src,O_RDONLY))) {
        perror("open");
        exit(1);
    }
    int size;
    char c;
    if (-1==(size=read(fdold,&c,sizeof(c)))) {
        perror("read");
        exit(1);
    }
    while (size>0) {
        if (-1==write(fdnew,&c,size)) {
            perror("write");
            exit(1);
        }
        if (-1==(size=read(fdold,&c,sizeof(c)))) {
            perror("read");
            exit(1);
        }
    }
    if (-1==close(fdnew)) {
        perror("close");
        exit(1);
    }
    if (-1==close(fdold)) {
        perror("close");
        exit(1);
    }
}

void cp_rdwr1(const char *src, const char *dest) {
    int fdnew,fdold;
    if (-1==(fdnew=open(dest,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))) {
        perror("open");
        exit(1);
    }
    if (-1==(fdold=open(src,O_RDONLY))) {
        perror("open");
        exit(1);
    }
    int size;
    char c[BUFSIZ];
    if (-1==(size=read(fdold,c,sizeof(c)))) {
        perror("read");
        exit(1);
    }
    while (size>0) {
        if (-1==write(fdnew,c,size)) {
            perror("write");
            exit(1);
        }
        if (-1==(size=read(fdold,c,sizeof(c)))) {
            perror("read");
            exit(1);
        }
    }
    if (-1==close(fdnew)) {
        perror("close");
        exit(1);
    }
    if (-1==close(fdold)) {
        perror("close");
        exit(1);
    }
}

int main(int argc, char **argv) {
    if (argc<=1) {
        cout<<"cp: missing file operand"<<endl;
        return 1;
    }
    else if (argc<=2) {
        cout<<"cp: missing destination file operand after '"<<argv[1]<<"'"<<endl;
        return 1;
    }
    else if (argc>4) {
        cout<<"cp: too many arguments"<<endl;
        return 1;
    }
    struct stat buf;
    if (0==stat(argv[2],&buf)) {
        cout<<"Fail: "<<argv[2]<<" already exists"<<endl;
        exit(1);
    }
    else
        perror("stat");
    if (argc==4) {
        Timer t1,t2,t3;
        double eTime1,eTime2,eTime3;
        t1.start();
        cp_stream(argv[1],argv[2]);
        cout<<"cp_stream"<<endl;
        t1.elapsedUserTime(eTime1);
        cout<<"user runtime: "<<eTime1<<endl;
        t1.elapsedWallclockTime(eTime1);
        cout<<"wallclock runtime: "<<eTime1<<endl;
        t1.elapsedSystemTime(eTime1);
        cout<<"system runtime: "<<eTime1<<endl;
        t2.start();
        cp_rdwr(argv[1],argv[2]);
        cout<<"cp_rdwr"<<endl;
        t2.elapsedUserTime(eTime2);
        cout<<"user runtime: "<<eTime2<<endl;
        t2.elapsedWallclockTime(eTime2);
        cout<<"wallclock runtime: "<<eTime2<<endl;
        t2.elapsedSystemTime(eTime2);
        cout<<"system runtime: "<<eTime2<<endl;
        t3.start();
        cp_rdwr1(argv[1],argv[2]);
        cout<<"cp_rdwr1"<<endl;
        t3.elapsedUserTime(eTime3);
        cout<<"user runtime: "<<eTime3<<endl;
        t3.elapsedWallclockTime(eTime3);
        cout<<"wallclock runtime: "<<eTime3<<endl;
        t3.elapsedSystemTime(eTime3);
        cout<<"system runtime: "<<eTime3<<endl;
        return 0;
    }
    //cp_stream(argv[1],argv[2]);
    //cp_rdwr(argv[1],argv[2]);
    cp_rdwr1(argv[1],argv[2]);
    return 0;
}
