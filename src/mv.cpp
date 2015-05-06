#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

using namespace std;

int main(int argc, char *argv[]) {
	struct stat buf;
	char temp[100];
	int i = stat(argv[1], &buf);
	if(i == -1) {
		cerr << "file does not exist" << endl;
		exit(1);
	}
	i = stat(argv[2], &buf);
	if(i == -1) {
		link(argv[1], argv[2]);
		unlink(argv[1]);
	}
	if(S_ISREG(buf.st_mode)) {
		cerr << "file already exists" << endl;
		exit(1);
	}
	else if(S_ISDIR(buf.st_mode)) {
		strcpy(temp, argv[2]);
		strcat(temp, "/");
		strcat(temp, argv[1]);
		i = stat(temp, &buf);
		if(i == -1) {
			link(argv[1], temp);
			unlink(argv[1]);
		}
		else {
			cerr << "file already exists" << endl;
			exit(1);
		}
	}
	
	return 0;
}
