#include <iostream>
#include <signal.h>

#include "task1.h"
#include "task2.h"
#include "task3.h"
#include "task4.h"

using namespace std;

int main()
{
    signal(SIGINT, exitShell);
    initialise();
    return 0;
}
