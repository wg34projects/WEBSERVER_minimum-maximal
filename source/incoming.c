#include "server.h"

LOGFILE decodeIncoming(char *textInBuffer, int connfd)
{
	// vars
	LOGFILE logLine;
	int linesIncoming = 0, linesAllowed = 0, maxURLCheck = 0, portNo = 0;
	char ipAddr[15];
	RECEIVEDINFO *clientinfo = NULL;
	ALLOWEDPATH *pHead = NULL;
	TEXTINCOMING *pTop = NULL;
	SOCKADDRIN *conn;
	SOCKSTORAGE clientAdress;
	TM *timeStamp, timeTemp;
	time_t timeEpoch;
	socklen_t length;
	const char *agent = "User-Agent:";
	const char *host = "Host:";
	const char *method[] = { "GET", "HEAD" };
	const char *category = "__security";

	// prepare variables
	logLine.exit = 0;
	logLine.error = 0;
	memset(logLine.logLineFile, 0, sizeof(logLine.logLineFile));
	memset(logLine.timezone, 0, sizeof(logLine.timezone));

	// prepare IP stamp
	length = (socklen_t)sizeof(clientAdress);
	if (getpeername(connfd, (SOCKADDR*)&clientAdress, &length) == -1)
	{
		perror("ERROR getpeername");
		logLine.exit++;
	}
	conn = (SOCKADDRIN*)&clientAdress;
	portNo = ntohs(conn->sin_port);
	if (inet_ntop(AF_INET, &conn->sin_addr, ipAddr, INET_ADDRSTRLEN) == NULL) 
	{
		perror("error inet_ntop");
		printf("while converting IP [%s] to string - ", ipAddr);
		if (errno == EAFNOSUPPORT)
		{
			perror("EAFNOSUPPORT = not a valid adress family");
		}
		if (errno == ENOSPC)
		{
			perror("ENOSPC = converted string would exceed the given size");
		}
		logLine.exit++;
	}

	// prepare timestamp
	timeEpoch = time(NULL);
	timeStamp = localtime_r(&timeEpoch, &timeTemp);
	if (timeStamp == NULL)
	{
		perror("ERROR localtime");
		logLine.error++;
	}
	if (strftime(logLine.timezone, sizeof(logLine.timezone), "%a, %d %b %y %T %z", timeStamp) == 0)
	{
		perror("ERROR strftime returned 0");
		logLine.error++;
	}
	for (int i = 0; i < (int)strlen(logLine.timezone); i++)
	{
		if (logLine.timezone[i] == '\n')
		{
			logLine.timezone[i] = '\0';
		}
	}

	// add incoming data to linked list
	linesIncoming = readIncoming(textInBuffer, &pTop);
	if (linesIncoming == 0)
	{
		logLine.error++;
	}

	// callocs
	clientinfo = (RECEIVEDINFO *) calloc (1, sizeof(RECEIVEDINFO));
	if(clientinfo == NULL)
	{
		perror("ERROR calloc clientinfo");
		logLine.error++;
	}
	clientinfo->method = (char *) calloc (LINELEN, sizeof(char));
	if (clientinfo->method == NULL)
	{
		perror("ERROR calloc clientinfo->method");
		logLine.error++;
	}
	clientinfo->relativePath = (char *) calloc (LINELEN, sizeof(char));
	if (clientinfo->relativePath == NULL)
	{
		perror("ERROR calloc clientinfo->relativePath");
		logLine.error++;
	}
	clientinfo->homeAdress = (char *) calloc (LINELEN, sizeof(char));
	if (clientinfo->homeAdress == NULL)
	{
		perror("ERROR calloc clientinfo->homeAdress");
		logLine.error++;
	}
	clientinfo->agent = (char *) calloc (LINELEN, sizeof(char));
	if (clientinfo->agent == NULL)
	{
		perror("ERROR calloc clientinfo->agent");
		logLine.error++;
	}

#if DEBUGINCLEAR

	// put info to STDOUT incoming request
	showIncoming(&pTop);

#endif

	// find valid method and valid url length and stop
	if (findIncoming(pTop, method[0], linesIncoming, &clientinfo->method) == 1)
	{
		logLine.method = 1;	// GET
	}
	else if (findIncoming(pTop, method[1], linesIncoming, &clientinfo->method) == 1)
	{
		logLine.method = 2;	// HEAD
	}
	else
	{
		logLine.method = 0;
	}
	if (logLine.method == 0)
	{
		screenInfo(category, 0, "no valid method, response and exit", 0);
		//later response like should be
		logLine.exit++;
	}
	else
	{
		screenInfo(category, 2, "valid method found", 0);
	}

	maxURLCheck = strlen(clientinfo->method);

	if (maxURLCheck > MAXURL+URLNORMAL)
	{
		screenInfo(category, 0, "received URL exceeds maximum length of", MAXURL);
		//later page like should be
		logLine.exit++;
	}
	else
	{
		screenInfo(category, 2, "received URL length in range", 0);
	}

	// find valid host
	if (findIncoming(pTop, host, linesIncoming, &clientinfo->homeAdress) == 0)
	{
		screenInfo(category, 0, "no host found, exit", 0);
		logLine.exit++;
	}
	else
	{
		screenInfo(category, 2, "host found", 0);
	}

	if (getRelativePath(&clientinfo->method, &clientinfo->relativePath) == 0)
	{
		screenInfo(category, 0, "no relative path extractable", 0);
		logLine.exit++;
	}
	else
	{
		screenInfo(category, 2, "relative path could be extracted", 0);
	}

	// find allowed relative paths
	linesAllowed = readAllowed(&pHead);
	if (linesAllowed == 0)
	{
		perror("ERROR reading allowed paths");
		logLine.error++;
	}
	showAllowed(&pHead);

	if (findAllowed(pHead, clientinfo->relativePath, linesAllowed) == 0)
	{
		screenInfo(category, 1, "no allowed relative path", 0);
		// later correct page
		logLine.exit++;
	}
	else
	{
		screenInfo(category, 2, "allowed relative path found", 1);
	}

	// find valid user agent
	if (findIncoming(pTop, agent, linesIncoming, &clientinfo->agent) == 0)
	{
		screenInfo(category, 1, "no agent found but no risk detected", 0);
	}
	else
	{
		screenInfo(category, 2, "valid agent received", 0);
	}

	// logline
	if (snprintf(logLine.logLineFile, PIPE_BUF, "URL %s%s | %ju s since Epoch | local time: %s | %s | peer IP %s:%d",
			clientinfo->homeAdress, clientinfo->relativePath, (uintmax_t)timeEpoch, logLine.timezone, clientinfo->agent, ipAddr, portNo) != strlen(logLine.logLineFile))
	{
		perror("ERROR snprintf logLine.logLineFile");
		logLine.error++;
	}

	// clean
	free(clientinfo->method);
	free(clientinfo->relativePath);
	free(clientinfo->homeAdress);
	free(clientinfo->agent);
	free(clientinfo);
	freeIncoming(&pTop);
	freeAllowed(&pHead);
	return logLine;
}
