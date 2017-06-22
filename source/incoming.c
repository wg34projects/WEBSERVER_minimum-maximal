#include "server.h"

LOGFILE decodeIncoming(char *textInBuffer, int connfd)
{
	LOGFILE logLine;
	ALLOWEDPATH *pHead = NULL;
	TEXTINCOMING *pTop = NULL;
	SOCKADDRIN *conn;
	SOCKSTORAGE clientAdress;
	TM *timeStamp, timeTemp;
	socklen_t length;
	int linesIncoming = 0, linesAllowed = 0;
	char *tempString;
	USERAGENTVAR
	HOSTVAR
	METHODVAR
	CATSECURITYVAR
	FILEEXTSTRING

	// init
	memset(logLine.timezone, 0, sizeof(logLine.timezone));
	memset(logLine.ipAddr, 0, sizeof(logLine.ipAddr));
	memset(logLine.methodString, 0, sizeof(logLine.methodString));
	memset(logLine.relativePath, 0, sizeof(logLine.relativePath));
	memset(logLine.homeAdress, 0, sizeof(logLine.homeAdress));
	memset(logLine.agent, 0, sizeof(logLine.agent));
	logLine.exit = 0;
	logLine.error = 0;
	logLine.method = 0;
	logLine.portNo = 0;
	logLine.host = 0;
	logLine.maxurl = 0;
	logLine.timeEpoch = 0;
	logLine.found = 0;
	logLine.agentinfo = 0;
	logLine.type = 0;

	// IP info
	length = (socklen_t)sizeof(clientAdress);
	if (getpeername(connfd, (SOCKADDR*)&clientAdress, &length) == -1)
	{
		perror("ERROR getpeername");
		logLine.exit++;		// exit trigger
	}
	conn = (SOCKADDRIN*)&clientAdress;
	logLine.portNo = ntohs(conn->sin_port);
	if (inet_ntop(AF_INET, &conn->sin_addr, logLine.ipAddr, INET_ADDRSTRLEN) == NULL)
	{
		perror("error inet_ntop");
		if (errno == EAFNOSUPPORT)
		{
			perror("EAFNOSUPPORT = not a valid adress family");
		}
		if (errno == ENOSPC)
		{
			perror("ENOSPC = converted string would exceed the given size");
		}
		logLine.exit++;		// exit trigger
	}

	// timestamp
	logLine.timeEpoch = time(NULL);
	timeStamp = localtime_r(&logLine.timeEpoch, &timeTemp);
	if (timeStamp == NULL)
	{
		perror("ERROR localtime");
		logLine.error++;		// error trigger
	}
	if (strftime(logLine.timezone, sizeof(logLine.timezone), TIMESTAMPFORMAT, timeStamp) == 0)
	{
		perror("ERROR strftime returned 0");
		logLine.error++;		// error trigger
	}
	for (int i = 0; i < (int)strlen(logLine.timezone); i++)
	{
		if (logLine.timezone[i] == '\n')
		{
			logLine.timezone[i] = '\0';
		}
	}

	// incoming data to linked list
	linesIncoming = readIncoming(textInBuffer, &pTop);
	if (linesIncoming == 0)
	{
		logLine.error++;		// error trigger
	}
	screenInfo(category, 4, "lines received from client", linesIncoming);
#if DEBUGINLINES
	showIncoming(&pTop);
#endif

	// method
	CALLOCTEMPSTRING
	if (findIncoming(pTop, method[0], linesIncoming, &tempString) == 1)
	{
		logLine.method = 1;
		screenInfo(category, 2, "valid method GET received", 1);
	}
	else if (findIncoming(pTop, method[1], linesIncoming, &tempString) == 1)
	{
		logLine.method = 2;
		screenInfo(category, 2, "valid method HEAD received", 2);
	}
	else
	{
		logLine.method = 0;
		screenInfo(category, 0, "no valid method GET or HEAD received", 0);
		logLine.error++;		// error trigger
	}
	if (strncpy(logLine.methodString, tempString, strlen(tempString)) != logLine.methodString)
	{
		perror("ERROR error strncpy method");
		logLine.error++;		// error trigger
	}
	FREETEMPSTRING

	// host, homeadress, relative path
	CALLOCTEMPSTRING
	if (findIncoming(pTop, host, linesIncoming, &tempString) == 0)
	{
		logLine.host = 0;
		screenInfo(category, 0, "no host info received", 0);
		logLine.exit++;		// exit trigger
	}
	else
	{
		logLine.host = 1;
		screenInfo(category, 2, "host info received", 0);
	}
	if (strncpy(logLine.homeAdress, tempString+6, strlen(tempString)) != logLine.homeAdress)
	{
		perror("ERROR error strncpy homeAdress");
		logLine.error++;		// error trigger
	}
	FREETEMPSTRING
	CALLOCTEMPSTRING
	if (getRelativePath(logLine.methodString, &tempString) == 0)
	{
		screenInfo(category, 0, "no relative path determinable", 0);
		logLine.error++;		// error trigger
	}
	else
	{
		screenInfo(category, 2, "relative path determinable", 0);
	}
	if (strncpy(logLine.relativePath, tempString, strlen(tempString)) != logLine.relativePath)
	{
		perror("ERROR error strncpy relativePath");
		logLine.error++;		// error trigger
	}
	FREETEMPSTRING

#if DEBUGRELATIVEPATH
	screenInfo(category, 3, "valid method GET received", 1);
#endif

	// max URL length
	if (strlen(logLine.relativePath) > MAXURL)
	{
		logLine.maxurl = 0;
		screenInfo(category, 0, "received URL exceeds maximum length of", MAXURL);
		if (strncpy(logLine.relativePath, tempString, strlen(tempString)) != logLine.relativePath)
		{
			perror("ERROR error strncpy relativePath");
			logLine.error++;		// error trigger
		}
	}
	else
	{
		logLine.maxurl = 1;
		screenInfo(category, 2, "received URL length in range", 0);
	}

	// find allowed relative paths
	linesAllowed = readAllowed(&pHead);
	if (linesAllowed == 0)
	{
		perror("ERROR reading allowed paths");
		logLine.error++;		// error trigger
	}
#if DEBUGALLOWEDLINES
	showAllowed(&pHead);
#endif

	if (findAllowed(pHead, logLine.relativePath, linesAllowed) == 0)
	{
		logLine.found = 0;
		screenInfo(category, 1, "requested file not in list of allowed", 0);
		logLine.error++;		// error trigger
	}
	else
	{
		logLine.found = 1;
		screenInfo(category, 2, "requested file in list of allowed", 1);

		for (int i = 1; i < FILEXTCOUNT; i++)
		{
			if (strncmp(&(logLine.relativePath[strlen(logLine.relativePath)-3]), ext[i], 3) == 0)
			{
				logLine.type = i + 1;
			}
		}
		if (strncmp(&(logLine.relativePath[strlen(logLine.relativePath)-4]), ext[0], 4) == 0)
		{
			logLine.type = 1;
		}
	}

	// user agent
	CALLOCTEMPSTRING
	if (findIncoming(pTop, agent, linesIncoming, &tempString) == 0)
	{
		logLine.agentinfo = 0;
		screenInfo(category, 1, "no agent found but no risk detected", 0);
	}
	else
	{
		logLine.agentinfo = 0;
		screenInfo(category, 2, "valid agent received", 0);
	}
	if (strncpy(logLine.agent, tempString, strlen(tempString)) != logLine.agent)
	{
		perror("ERROR error strncpy agent");
		logLine.error++;		// error trigger
	}
	FREETEMPSTRING

	// free linked lists
	freeAllowed(&pHead);
	freeIncoming(&pTop);
	return logLine;
}
