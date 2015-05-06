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

void removeDir(const char *curr) {
	struct stat buf;
	DIR *dirp;
	struct dirent *filespecs;
	char temp[100];
	if(NULL == (dirp = opendir(curr))) {
		perror("Error with opendir");
		exit(1);
	}
	while(NULL != (filespecs = readdir(dirp))) {
		if(strcmp(filespecs->d_name, ".") == 0 || strcmp(filespecs->d_name, "..") == 0)
		continue;
		strcpy(temp, curr);
		strcat(temp, "/");
		strcat(temp, filespecs->d_name);
		stat(temp, &buf);
		if(S_ISREG(buf.st_mode))
		{
			unlink(temp);
		}
		else if(S_ISDIR(buf.st_mode))
			removeDir(temp);
	}
	rmdir(curr);
	return;
}

bool param_r = false;
int main(int argc, char *argv[]) {
	struct stat buf;
	for(int i = 1; i < argc; ++i)
	{
		if(argv[i][0] == '-')
		{
			if(strlen(argv[i]) != 2 || argv[i][1] != 'r')
			{
				cerr << "wrong flag";
				exit(1);
			}
			param_r = true;
		}
		else{
			stat(argv[i], &buf);
			if(S_ISREG(buf.st_mode))
			{
				unlink(argv[i]);
			}
			else if(S_ISDIR(buf.st_mode))
			{
				if(param_r)
					removeDir(argv[i]);
				else
					cerr << "It's a folder";
			}
		}						

	}

	return 0;
}
