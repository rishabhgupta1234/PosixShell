#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>

#include "task1.h"
#include "task4.h"

using namespace std;
void handlePipes(string cmd)
{
    string temp = "";
    std::vector<std::string> individualCommand;
    for (int i = 0; i < (int)cmd.size(); i++)
    {
        if (cmd[i] != '|')
        {
            temp += cmd[i];
        }
        else
        {
            if (temp[0] == ' ')
            {
                temp = temp.substr(1);
                individualCommand.push_back(temp);
                temp = "";
            }
            else
            {
                individualCommand.push_back(temp);
                temp = "";
            }
        }
    }
    if (temp[0] == ' ')
    {
        temp = temp.substr(1);
        individualCommand.push_back(temp);
    }
    ///////////////////////////////////////////////////
    // pipe will create a theta channel between two communication channel
    // creates a unidirectional pipe
    // pipe fxn will return two file discripter
    //  for writing end fd[1]  //512B at a time
    // for reading end fd[0]    //1B at a time
    // sender(writring end)--> receiver(reading end)

    int fd[2], receiveData = 0;
    pid_t newProcess;
    if (getEnvVariable("HOME").empty())
    {
        for (int i = 0; !individualCommand[i].empty(); i++)
        {
            pipe(fd);
            newProcess = fork();
            if (newProcess > 0) // parent process
            {
                // wait is an inbuilt command in the Linux shell.
                // It waits for the process to change its state i.e.
                // it waits for any running process to complete and returns the exit status.
                wait(NULL);
                close(fd[1]);
                receiveData = fd[0];
            }
            else if (newProcess == 0) // child process
            {
                dup2(receiveData, 0);
                if (individualCommand[i + 1] != "\0" && (!individualCommand[i + 1].empty()))
                {
                    dup2(fd[1], 1);
                }
                close(fd[0]);
                parseInputString(individualCommand[i]);

                exit(EXIT_SUCCESS);
            }
            else if (newProcess < 0)
            {
                perror("Fork Failed");
            }
        }
    }
    cout << endl;
    int procId = fork();
    if (procId == 0)
    {
        execlp("/bin/sh", "/bin/sh", "-c", cmd.c_str(), (char *)NULL);
    }
    else
    {
        wait(&procId);
        return;
    }
}
void pipeCmd(string cmd)
{
    handlePipes(cmd);
}

pair<string, string> aliasHandle(string cmd)
{
    std::vector<std::string> arguments;
    string temp = "";
    int flag = 1;
    for (int i = 0; i < (int)cmd.size(); i++)
    {
        if (flag)
        {

            if (cmd[i] != ' ')
            {
                temp += cmd[i];
            }
            else
            {
                arguments.push_back(temp);
                temp = "";
                flag = 0;
            }
        }
        else
        {
            if (cmd[i] != '=')
            {
                temp += cmd[i];
            }
            else
            {
                arguments.push_back(temp);
                temp = "";
            }
        }
    }
    arguments.push_back(temp);
    std::string key = arguments[1];
    std::string value = arguments[2];
    std::pair<std::string, std::string> retVal;
    retVal.first = key;
    retVal.second = value;
    return retVal;
}
