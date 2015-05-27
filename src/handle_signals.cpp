#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int int_cnt, quit_cnt, stop_cnt;

void intHandler(int)
{
    cout << "C" << endl;
    ++int_cnt;
    return;
}

void quitHandler(int)
{
    cout << "\\" << endl;
    ++quit_cnt;
}

void stopHandler(int)
{
    ++stop_cnt;
    if(3 == stop_cnt)
    {
        cout << endl
            << "^C: " << int_cnt << endl
            << "^\\: " << quit_cnt << endl
            << "^Z: " << stop_cnt << endl;
        exit(0);
    }
    cout << "S" << endl;
    raise(SIGSTOP);
}

int main()
{
    if(SIG_ERR == signal(SIGINT, intHandler))
    {
        perror("signal");
        exit(1);
    }
    if(SIG_ERR == signal(SIGQUIT, quitHandler))
    {
        perror("signal");
        exit(1);
    }
    if(SIG_ERR == signal(SIGTSTP, stopHandler))
    {
        perror("signal");
        exit(1);
    }
    while(1)
    {
        cout << "x ";
        cout.flush();
        sleep(1);
    }
    return 0;
}
