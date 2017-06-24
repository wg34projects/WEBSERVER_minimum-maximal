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

#define TEXTLEN 5000		// max length send/receive
#define TEXTLENSEND 3000000

#define LINELEN 500			// max line length

#define MAXURL 100			// max url length
#define URLENDING 9			// HTTP/1.1 plus one space = 9
#define IP4LENGTH 15

#define DEBUGPIPE 0		// info pipe log to file
#define DEBUGINRAW 0		// raw incoming string
#define DEBUGINLINES 0 		// show incoming lines from client
#define DEBUGALLOWEDLINES 0
#define DEBUGRELATIVEPATH 0
#define RASPBERRYPI 1

#define FILEXTCOUNT 8
#define FILEEXTSTRING 		const char *ext[] = { "html", "htm", "png", "ico", "jpg", "gif", "txt", "mp4" };
#define FILEEXTSENDTYPE		const char *type[] = { "text/html; charset=utf-8", "text/htm; charset=utf-8", "image/png", "image/ico", "image/jpg", "image/gif", "text/plain", "video/mp4"};
#define CODECOUNT 5
#define FILEEXTSWITCHCASE	switch (logLine.type)\
							{\
							case 0:\
							{\
								typeID = 0;\
								break;\
							}\
							case 1:\
							{\
								typeID = 0;\
								break;\
							}\
							case 2:\
							{\
								typeID = 1;\
								break;\
							}\
							case 3:\
							{\
								typeID = 2;\
								break;\
							}\
							case 4:\
							{\
								typeID = 3;\
								break;\
							}\
							case 5:\
							{\
								typeID = 4;\
								break;\
							}\
							case 6:\
							{\
								typeID = 5;\
								break;\
							}\
							case 7:\
							{\
								typeID = 6;\
								break;\
							}\
							case 8:\
							{\
								typeID = 7;\
								break;\
							}\
							default:\
							{\
								break;\
							}\
							}\

#define CODESTRING 			const char *code[] = { "HTTP/1.1 200 OK\r\n", "HTTP/1.1 404 Not Found\r\n", "HTTP/1.1 405 Method Not Allowed\r\n", "HTTP/1.1 414 Request-URL Too Long\r\n", "HTTP/1.1 500 Internal Server Error\r\n" };
#define HTTP404STRING		"/pages/system/error404.html"
#define HTTP404STRINGLEN	32
#define HTTP500STRING		"/pages/system/error500.html"
#define HTTP500STRINGLEN	32
#define HTTP414STRING		"/pages/system/error414.html"
#define HTTP414STRINGLEN	32

#define SERVERSTRING		"Server: the-min-max-webserver helmut.resch@gmail.com v0.9\r\n"
#define CONNECTIONSTRING	"Connection: close\r\n"
#define METHODSALLOWED		"Allow: GET, HEAD\r\n"

#define CALLOCTEMPSTRING	tempString = (char *) calloc (LINELEN, sizeof(char));\
							if (tempString == NULL)\
							{\
								perror("ERROR calloc tempstring");\
								logLine.error++;\
							}	

#define FREETEMPSTRING		free(tempString);

#define CLOSELISTENPORT		if (close(listenfd) == -1)\
							{\
								perror("ERROR closing listenfd");\
							}

#define CLOSEWORKINGPORT	if (close(connfd) == -1)\
							{\
								perror("ERROR closing connected socket");\
							}

#define USERAGENTVAR 		const char *agent = "User-Agent:";
#define HOSTVAR 			const char *host = "Host:";
#define METHODVAR 			const char *method[] = { "GET", "HEAD" };
#define CATSECURITYVAR		const char *category = "__security";
#define CATALLOWEDVAR		const char *category = "___allowed";
#define CATINCOMINGVAR		const char *category = "__incoming";
#define CATOUTGOINGVAR		const char *category = "__outgoing";
#define CATEXITVAR			const char *category = "______exit";
#define CATFULLVAR			const char *category[] = { "____system", "______root", "____logger", "____server" , "___handler" , "____closer" };

#define TIMESTAMPFORMAT 	"%a, %d %b %y %T %z"

#define PATHEXECUTEABLE		"..%s"	// need to be changed if executeable is not one level "higher"

#define SPECIALPATH1		"/"
#define SPECIALPATH1LENGTH	1
#define SPECIALPATH2		"/index.html"
#define SPECIALPATH2LENGTH	11
#define SPECIALPATH3		"/index.htm"
#define SPECIALPATH3LENGTH	10

typedef struct allowedPath ALLOWEDPATH;
typedef struct textIncoming TEXTINCOMING;
typedef struct textOutgoing TEXTOUTGOING;
typedef struct logFile LOGFILE;
typedef struct sockaddr_in SOCKADDRIN;
typedef struct sigaction SIGACTION;
typedef struct tm TM;
typedef struct sockaddr_storage SOCKSTORAGE;
typedef struct sockaddr SOCKADDR;
typedef struct dirent DIRENT;

struct logFile
{
	char ipAddr[IP4LENGTH];
	int portNo;
	int error;
	int exit;
	int method;
	int host;
	int maxurl;
	int found;
	int agentinfo;
	int type;
	time_t timeEpoch;
	char timezone[LINELEN];
	char homeAdress[LINELEN];
	char methodString[LINELEN];
	char relativePath[LINELEN];
	char agent[LINELEN];
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
	int error;
	int exit;
	int lengthTotal;
	char relativePathCorrected[LINELEN];
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
int getRelativePath(char *input, char **output);

TEXTOUTGOING codeOutgoingHeader(LOGFILE logLine);

// global variable to be available for all functions and signal handler

int listenfd;

#endif
