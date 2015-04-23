#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cstring>

using namespace std;

int main(int argc, char** argv)
{
	for(int i = 0; i < argc; ++i)
	{
		if(argv[i][0] == '-')
		{
			for(int j = 1; j < strlen(argv[i]); ++j)
			{
				if(argv[i][j] == 'a' || argv[i][j] == 'l' || argv[i][j] == 'R')
				{

				}
				else
				{
					cerr << "wrong parameter: " << argv[i] << endl;
					exit(1);
				}
			}
		}
	}
}
