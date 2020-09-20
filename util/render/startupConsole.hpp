#ifndef STARTUPCONSOLE_H
#define STARTUPCONSOLE_H
#include <string>

using namespace std;

enum messageType{
    ERROR, WARNING, SUCCESS, NONE, STATUS, FATALERROR
};
struct message{
    messageType type;
    string message;
};

void updateStartupConsole();
void printFormatedString(message message);
void printMessage(string messageString, const char* str,  messageType type);
void printMessage(string messageString, const char* str, const char* str2, messageType type);
void printMessage(string messageString, messageType type);
void updateMessage(string messageString);
void updateMessageType(messageType type);
void waitToCloseError();
#endif
