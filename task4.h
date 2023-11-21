#include <string>
void initialiseTerminal(void);
std::string getEnvVariable(std::string);
void clearScreen(void);
void initialise(void);
void exitShell(int);
void parseInputString(std::string);
void handleRecordingOutput(std::string, std::string);
void handleRecordingNoOutput(std::string, std::string);