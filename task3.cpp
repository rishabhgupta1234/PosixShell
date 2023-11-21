#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>
#include "task3.h"
#include "task4.h"
void alarmMessage(std::vector<std::string> mymsg, int sec, std::string &recordFilePath) // if (args[0] == "alarm") alarmMessage(args, stoi(args[1]));
{
    int mysize;
    mysize = mymsg.size();
    if (mysize < 3)
    {
        std::cout << std::endl
                  << "Invalid alarm";
        std::cout << std::endl;
        return;
    }
    else
    {

        std::string sms;
        sms = "";

        int i;
        i = 2;

        while (i < mysize)
        {
            sms = sms + mymsg[i];
            sms = sms + " ";
            i = i + 1;
        }
        pid_t pid = fork();
        if (pid == 0)
        {
            sleep(sec);
            std::cout << sms;
            handleRecordingOutput(recordFilePath, sms);
            putchar(10);
            exit(EXIT_SUCCESS);
        }
        else
        {
            return;
        }
    }
}

void fileOpen(const char *filename)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        close(2);
        execlp("/usr/bin/open", "open", filename, NULL);
        exit(0);
    }
}
