#include <termios.h>
#include <unordered_map>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <sys/wait.h>
#include "task1.h"
#include "task2.h"
#include "task3.h"

using namespace std;

unordered_map<string, string> envVars;
unordered_map<string, string> localEnvVars;
unordered_map<string, string> aliasUnorderedMap;
struct termios oldTermios, newTermios;
vector<string> commands;
struct winsize winSize;
string histFilePath = "history.txt";
his_trie root;
Trie trie;
int lastRow;
int globalCount = 0;
bool isRecording = false;
char cwdBuffer[200];
string launchDir;
string recordFilePath = "record.txt";

// Place cursor based on x and y coordinates
void placeCursor(int x, int y)
{
    cout << "\033[" << x << ";" << y << "H";
    fflush(stdout);
}

void clearScreen(void)
{
    printf("\033[2J\033[1;1H");
    placeCursor(lastRow, 0);
}

void clearLine()
{
    cout << "\033[2K";
}

void print(string s)
{
    cout << endl
         << s << endl;
}

vector<string> splitCommand(string s, string delimiter)
{
    vector<string> result;

    int start, end = -1 * delimiter.size();
    do
    {
        start = end + delimiter.size();
        end = s.find(delimiter, start);
        result.push_back(s.substr(start, end - start));
    } while (end != -1);

    return result;
}

// Reading all env variables stored in .bashrc on startup
void readEnvVariables(void)
{
    fstream file;
    file.open(".bashrc", ios::in);
    if (file.is_open())
    {
        string line, delimiter = "=", key, value;
        int idx;
        while (getline(file, line))
        {
            idx = line.find(delimiter);
            key = line.substr(0, idx);
            value = line.substr(idx + 1);
            envVars[key] = value;
        }
        file.close();
    }
}

string getEnvVariable(string key)
{
    if (envVars.find(key) != envVars.end())
    {
        return envVars[key];
    }
    return "Environment variable not found!";
}

void switchToCanonicalMode()
{
    tcgetattr(0, &oldTermios);
    newTermios = oldTermios;
    // The actual magic happens here
    newTermios.c_lflag &= ~(ICANON | ECHO);
    newTermios.c_cc[VMIN] = 1;
    newTermios.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newTermios);
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winSize);
    lastRow = winSize.ws_row;
    placeCursor(lastRow - 1, 0);
    return;
}

void exitShell(int sigNum)
{
    exit(0);
}

void handleRedirection(string command, string redirectionType)
{
    vector<string> splittedCommand;

    string src = "", destination;

    splittedCommand = splitCommand(command, " ");

    for (int i = 0; i < splittedCommand.size() - 2; i++)
    {
        src += splittedCommand[i] + " ";
    }
    destination = splittedCommand[splittedCommand.size() - 1];

    const char *arg[] = {src.c_str(), destination.c_str()};
    int pid = fork();
    int fd1 = open(destination.c_str(), redirectionType == ">" ? O_TRUNC | O_CREAT | O_WRONLY : O_APPEND | O_CREAT | O_WRONLY, 0644);
    if (!pid)
    {
        dup2(fd1, 1);
        execl("/bin/sh", "sh", "-c", src.c_str(), NULL);
    }
    else
    {
        wait(&pid);
        close(fd1);
        cout << endl;
        return;
    }
}

void handleBackgroundExec() {}

void changeDirectory(vector<string> &command)
{
    if (command.size() < 2)
    {
        cout << "Destination not provided" << endl;
        return;
    }
    string destination = command[1];

    if (destination == "~")
    {
        // Switch to the home directory
        destination = getEnvVariable("HOME");
    }

    // Changing directory
    cout << endl;
    if (chdir(destination.c_str()) < 0)
    {
        return;
    }
}

void exportVariable(vector<string> &command)
{
    if (command[1] == "-p")
    {
        // List all the environment variables, both the local ones and the global ones
        map<string, string> m;
        for (auto it : envVars)
        {
            m[it.first] = it.second;
        }
        for (auto it : localEnvVars)
        {
            m[it.first] = it.second;
        }

        // Printing all the variables
        for (auto it : m)
        {
            cout << endl
                 << it.first << "=" << it.second;
        }
        cout << endl;
        return;
    }

    if (command[1] == "-n")
    {
        string keyToDelete = command[2];
        if (localEnvVars.find(keyToDelete) != localEnvVars.end())
        {
            localEnvVars.erase(keyToDelete);
            return;
        }

        if (envVars.find(keyToDelete) != envVars.end())
        {
            envVars.erase(keyToDelete);
            return;
        }
    }
    // Storing the passed environment variable
    vector<string> variable = splitCommand(command[1], "=");
    string key = variable[0];
    string value = variable[1];

    localEnvVars[key] = value;
    cout << endl;
    return;
}

void setEnvironment(string key, string value)
{
    cout << endl;
    if (key == "")
    {
        print("Key cannot be empty");
        return;
    }
    // Writing into bashrc
    ofstream file(".bashrc", ios::app);
    file << key << "=" << value << endl;
    file.close();
    // Updating our map to get this env variable
    readEnvVariables();
}

void unsetEnvironment(string key)
{
    cout << endl;
    if (key == "")
    {
        print("Key cannot be empty");
        return;
    }
    string line;
    ifstream inp(".bashrc");

    if (!inp.is_open())
    {
        print(".bashrc failed to open");
        return;
    }
    ofstream op("temp");
    while (getline(inp, line))
    {
        vector<string> splittedLine = splitCommand(line, "=");
        if (splittedLine[0] != key)
        {
            cout << "Inside" << endl;
            op << line << endl;
        }
    }
    inp.close();
    op.close();

    remove(".bashrc");
    rename("temp", ".bashrc");

    // Clearing the entry from map we have maintained
    envVars.erase(key);
}

void handleBasicCommands(string args)
{
    cout << endl;
    int pid = fork();
    if (!pid)
    {
        execl("/bin/sh", "sh", "-c", args.c_str(), NULL);
    }
    else
    {
        wait(&pid);
        return;
    }
}

void handleRecordingOutput(string recordFile, string output)
{
    if (isRecording)
    {
        ofstream fout;
        fout.open(recordFile.c_str(), std::ios::app);
        fout << output;
        fout << "\n";
        fout.close();
    }
}

void handleRecordingNoOutput(string recordFile, string command)
{
    if (isRecording)
    {
        ofstream fout;
        fout.open(recordFile.c_str(), std::ios::app);
        fout << command << "\n";
        fout.close();
    }
}

void handleRecording(string recordFile, string command)
{
    if (isRecording)
    {
        handleRecordingNoOutput(recordFile, command);
        vector<string> splittedCommand = splitCommand(command, " ");
        string firstParam = splittedCommand[0];
        if (firstParam != "touch" && firstParam != "nano" && firstParam != "mkdir" && firstParam != "cd")
        {
            int pid = fork();
            int fd1 = open(recordFile.c_str(), O_APPEND | O_WRONLY | O_CREAT, 0644);
            if (!pid)
            {
                dup2(fd1, 1);
                execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
            }
            else
            {
                wait(&pid);
                close(fd1);
                cout << endl;
                return;
            }
        }
    }
}

void parseInputString(string command)
{
    if (command.find("~") != string::npos)
    {
        int idx = command.find("~");
        string leftPart = command.substr(0, idx);
        string rightPart = command.substr(idx + 1);
        command = leftPart + getEnvVariable("HOME") + rightPart;
    }
    vector<string> splittedCommand = splitCommand(command, " ");
    if (command.find('>') != string::npos)
    {
        // Handle both >> and >
        if (command.find(">>") != string::npos)
        {
            handleRedirection(command, ">>");
        }
        else
        {
            handleRedirection(command, ">");
        }
        handleRecordingNoOutput(recordFilePath, command);
    }
    else if (command.find('|') != string::npos)
    {
        // Handle | piping
        pipeCmd(command);
        handleRecording(recordFilePath, command);
    }
    else if (command.back() == '&')
    {
        // Handle background command execution
        // handleBackgroundExec();
        handleBasicCommands(command);
    }
    else if (splittedCommand[0] == "echo")
    {
        if (splittedCommand.size() == 1)
        {
            // If no parameter is provided to echo the simply print a newline
            return;
        }
        else if (splittedCommand[1] == "$$")
        {
            // $$ is the PID of the current process
            handleRecordingNoOutput(recordFilePath, command);
            string processId = to_string(getpid());
            print(processId);
            handleRecordingOutput(recordFilePath, processId);
        }
        else if (splittedCommand[1] == "$?")
        {
            // $? is the return code of the last executed command.
            handleBasicCommands(command);
            handleRecording(recordFilePath, command);
        }
        else if (splittedCommand[1][0] == '$')
        {
            // Print the environment variable
            handleRecordingNoOutput(recordFilePath, command);
            string key = splittedCommand[1].substr(1);
            for (auto it : envVars)
            {
                if (it.first == key)
                {
                    print(it.second);
                    handleRecordingOutput(recordFilePath, it.second);
                    return;
                }
            }
            for (auto it : localEnvVars)
            {
                if (it.first == key)
                {
                    print(it.second);
                    handleRecordingOutput(recordFilePath, it.second);
                    return;
                }
            }
            return;
        }
        // else if() {Implement redirection in echo}
        else
        {
            handleBasicCommands(command);
            if (isRecording)
            {
                handleRecording(recordFilePath, command);
            }
        }
    }
    else if (splittedCommand[0] == "jobs")
    {
        // I've not implemented the jobs command
        handleBasicCommands(command);
    }
    else if (splittedCommand[0] == "quit")
    {
        // Can save the history.. Look into it
        handleRecordingNoOutput(recordFilePath, command);
        save_history(root, histFilePath);
        exit(EXIT_SUCCESS);
    }
    else if (splittedCommand[0] == "history")
    {
        handleRecordingNoOutput(recordFilePath, command);
        int limit = stoi(getEnvVariable("HISTSIZE"));
        print_history(root, limit, recordFilePath);
    }
    else if (splittedCommand[0] == "clear")
    {
        handleRecordingNoOutput(recordFilePath, command);
        clearScreen();
    }
    else if (splittedCommand[0] == "export")
    {
        handleRecordingNoOutput(recordFilePath, command);
        exportVariable(splittedCommand);
    }
    else if (splittedCommand[0] == "alarm")
    {
        // Rishabh has to implement this
        handleRecordingNoOutput(recordFilePath, command);
        alarmMessage(splittedCommand, stoi(splittedCommand[1]), recordFilePath);
    }
    else if (splittedCommand[0] == "open")
    {
        // Rishabh has to implement this
        handleRecordingNoOutput(recordFilePath, command);
        string filePath = splittedCommand[1];
        fileOpen(filePath.c_str());
    }
    else if (splittedCommand[0] == "alias")
    {
        // Prashant has to implement this
        handleRecordingNoOutput(recordFilePath, command);
        pair<string, string> alias = aliasHandle(command);
        aliasUnorderedMap[alias.first] = alias.second;
    }
    else if (splittedCommand[0] == "fg")
    {
        // Ujjwal has to do this
    }
    else if (splittedCommand[0] == "setenv")
    {
        // Format would be like
        // setenv COLLEGE IIIT
        handleRecordingNoOutput(recordFilePath, command);
        setEnvironment(splittedCommand[1], splittedCommand[2]);
    }
    else if (splittedCommand[0] == "unsetenv")
    {
        // Format would be like
        // unsetenv COLLEGE
        handleRecordingNoOutput(recordFilePath, command);
        unsetEnvironment(splittedCommand[1]);
    }
    else if (splittedCommand[0] == "record")
    {
        handleRecordingNoOutput(recordFilePath, command);
        if (splittedCommand[1] == "start")
        {
            isRecording = true;
        }
        else if (splittedCommand[1] == "stop")
        {
            isRecording = false;
        }
    }
    else
    {
        if (aliasUnorderedMap.find(splittedCommand[0]) != aliasUnorderedMap.end())
        {
            string modifiedCommand = "";
            splittedCommand[0] = aliasUnorderedMap[splittedCommand[0]];
            parseInputString(splittedCommand[0]);
        }
        else
        {
            // cat, ls, mkdir, touch, nano, cd, pwd, whoami
            if (splittedCommand[0] == "cd")
            {
                if (chdir(splittedCommand[1].c_str()) != 0)
                {
                    cout << endl
                         << "Invalid path" << endl;
                }
            }
            else
            {
                handleBasicCommands(command);
                if (isRecording)
                {
                    handleRecording(recordFilePath, command);
                }
            }
        }
    }
}

void takeInput()
{
    // username@hostname:path >
    string s;
    char ch;
    vector<string> pathDirs = splitCommand(getEnvVariable("PATH"), " ");
    trie.populateTrie(pathDirs);
    // int commandIdx = -1;
    while (true)
    {
        string ps1 = getEnvVariable("PS1");
        string prompt = "";
        string homePath = getEnvVariable("HOME");
        char pathBuff[200];
        string cwd = getcwd(pathBuff, sizeof(pathBuff));
        string promptPath = "";
        if (homePath.size() <= cwd.size() && cwd.substr(0, homePath.size()) == homePath)
        {
            promptPath = "~" + cwd.substr(homePath.size());
        }
        else
        {
            promptPath = cwd;
        }
        char buffer[256];
        gethostname(buffer, sizeof(buffer));
        string host = buffer;
        getlogin_r(buffer, sizeof(buffer));
        string user = buffer;
        string separator = geteuid() ? "@" : "#";
        if (separator == "@")
        {
            prompt = user + separator + host + ":" + promptPath + ps1;
        }
        else
        {
            prompt = "root" + separator + host + ":" + promptPath + ps1;
        }
        cout << prompt;
        s = "";
        while ((ch = cin.get()) && ch != 10)
        {
            cout << ch;
            // If backspace key is pressed
            if (ch == 127)
            {
                if (s.size())
                {
                    s.pop_back();
                    printf("\b \b");
                }
                else
                {
                    // placeCursor(rows, 0);
                }
            }
            else if (ch == 27)
            {
                ch = cin.get();
                ch = cin.get();
                if (ch == 'A')
                {
                    // Up key was pressed
                    // if(commandIdx >= 0) {
                    //     s = commands[commandIdx--];
                    //     cout<<s;
                    // }
                }
                else if (ch == 'B')
                {
                    // Down key was pressed
                    // if(commandIdx < commands.size()) {
                    //     s = commands[commandIdx++];
                    //     cout<<" "<<s;
                    // }
                }
                else if (ch == 'C')
                {
                    // Right key was pressed
                }
                else if (ch == 'D')
                {
                    // Left key was pressed
                }
            }
            else if (ch == '\t')
            {
                // TAB key was pressed
                string lastWord;
                vector<string> autoCompletionResults;
                if (s.find_last_of(' ') != string::npos)
                {
                    lastWord = s.substr(s.find_last_of(' '));
                }
                else
                {
                    lastWord = s;
                }
                // trim(lastWord);
                trie.autocomplete(lastWord, autoCompletionResults);
                // for(int i = 0; i < autoCompletionResults.size(); i++) {
                //     cout<<autoCompletionResults[i]<<" ";
                // }
                if (autoCompletionResults.empty())
                {
                    // Auto completion result is empty.. Nothing stored in tried in context to lastWord
                    cout << endl;
                    break;
                }
                else if (autoCompletionResults.size() == 1)
                {
                    // Only one result in trie in context to lastWord
                    // Just replace the lastWord with the result
                    // trim(s);
                    cout << s;
                    if (s.find(' ') == string::npos)
                    {
                        // There is only one word in the input command
                        s = autoCompletionResults[0];
                    }
                    else
                    {
                        s = s.substr(0, s.find_last_of(' '));
                        s = s + ' ' + autoCompletionResults[0];
                    }
                    clearLine();
                    placeCursor(lastRow, 0);
                    cout << prompt << s;
                }
                else
                {
                    // Many words stored in trie
                    cout << endl;
                    for (int i = 0; i < autoCompletionResults.size(); i++)
                    {
                        cout << autoCompletionResults[i] << " ";
                    }
                    cout << endl;
                    break;
                }
            }
            else
            {
                s += ch;
            }
        }
        if (ch == 10)
        {
            // Enter key was pressed
            if (s != "")
            {
                globalCount++;
                root.insert(&root, s, globalCount);
                parseInputString(s);
            }
            cout << endl;
        }
    }
}

// Initialise the POSIX shell
// All startup functions are called here
void initialise()
{
    clearScreen();
    readEnvVariables();
    switchToCanonicalMode();
    load_history(&root, histFilePath, globalCount);
    getcwd(cwdBuffer, sizeof(cwdBuffer));
    launchDir = string(cwdBuffer);
    recordFilePath = launchDir + "/" + recordFilePath;
    histFilePath = launchDir + "/" + histFilePath;
    takeInput();
}
