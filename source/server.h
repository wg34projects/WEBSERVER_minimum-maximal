#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <netdb.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

#define TEXTLEN 100000		// max length send/receive

#define LINELEN 500			// max line length

#define MAXURL 100			// max url length
#define URLNORMAL 14		// GET /nowMAXURLlength HTTP/1.1 are MAXURL + 14
#define URLENDING 9			// HTTP/1.1 plus one space = 9

#define DEBUGPIPE 1			// info pipe
#define DEBUGINRAW 0		// raw incoming string
#define DEBUGINCLEAR 1 		// cleared incoming string

#define FILEXTCOUNT 5
#define FILEEXTSTRING { "html", "htm", "png", "ico", "jpg" } // this 5 to be kept

#define CODECOUNT 6
#define CODESTRING { "HTTP/1.1 200 OK", "HTTP/1.1 400 Bad Request", "HTTP/1.1 404 Not Found", "HTTP/1.1 405 Method Not Allowed", "HTTP/1.1 414 Request-URL Too Long", "HTTP/1.1 500 Internal Server Error" }


typedef struct allowedPath ALLOWEDPATH;
typedef struct textIncoming TEXTINCOMING;
typedef struct textOutgoing TEXTOUTGOING;
typedef struct logFile LOGFILE;
typedef struct sockaddr_in SOCKADDRIN;
typedef struct receivedInfo RECEIVEDINFO;
typedef struct sigaction SIGACTION;
typedef struct tm TM;
typedef struct sockaddr_storage SOCKSTORAGE;
typedef struct sockaddr SOCKADDR;
typedef struct dirent DIRENT;

struct logFile
{
	char logLineFile[PIPE_BUF];
	char timezone[LINELEN];
	int error;
	int exit;
	int method;
};

struct receivedInfo
{
	char *homeAdress;
	char *method;
	char *relativePath;
	char *agent;
};

struct allowedPath
{
	char pathVar[LINELEN];
	ALLOWEDPATH *pNext;
};

struct textIncoming
{
	char incomeVar[LINELEN];
	TEXTINCOMING *pNext;
};

struct textOutgoing
{
	char code[LINELEN];
	char date[LINELEN];
	char server[LINELEN];
	char contentlength[LINELEN];
	char connection[LINELEN];
	char allowed[LINELEN];
	char content[LINELEN];
	char load[TEXTLEN];
};

// function declarations

void ctrlChandler(int dummy);
void wait_for_child(int sig);

void screenLog(const char name[11], long unsigned int counter, int pid, int ppid, int topid);
void screenInfo(const char *category, int goodbad, char *string, int number);
int getInteger(char *input, int *numInteger, int lo, int hi);
void help();

LOGFILE decodeIncoming(char *textInBuffer, int connfd);

int readAllowed(ALLOWEDPATH **path);
void showAllowed(ALLOWEDPATH **path);
int findAllowed(ALLOWEDPATH *path, const char *look, int lines);
void freeAllowed(ALLOWEDPATH **path);
int recursiveWalk(ALLOWEDPATH **path, const char *pathName, int level);

int readIncoming(char *textInBuffer, TEXTINCOMING **text);
void showIncoming(TEXTINCOMING **text);
int findIncoming(TEXTINCOMING *text, const char *look, int lines, char **output);
void freeIncoming(TEXTINCOMING **text);
int getRelativePath(char **input, char **output);

TEXTOUTGOING codeOutgoing(const char *code, char *date);

// global variable to be available for all functions and signal handler

int listenfd;

#endif
