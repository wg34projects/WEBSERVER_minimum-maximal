#include "server.h"

TEXTOUTGOING codeOutgoingHeader(LOGFILE logLine)
{
	TEXTOUTGOING outgoingData;
	int typeID = 0, filedescriptor = 0;
	CODESTRING
	FILEEXTSENDTYPE

	// init
	memset(outgoingData.code, 0, sizeof(outgoingData.code));
	memset(outgoingData.date, 0, sizeof(outgoingData.date));
	memset(outgoingData.server, 0, sizeof(outgoingData.server));
	memset(outgoingData.contentlength, 0, sizeof(outgoingData.contentlength));
	memset(outgoingData.connection, 0, sizeof(outgoingData.connection));
	memset(outgoingData.allowed, 0, sizeof(outgoingData.allowed));
	memset(outgoingData.content, 0, sizeof(outgoingData.content));
	memset(outgoingData.load, 0, sizeof(outgoingData.load));
	memset(outgoingData.relativePathCorrected, 0, sizeof(outgoingData.relativePathCorrected));
	outgoingData.error = 0;
	outgoingData.exit = 0;
	outgoingData.lengthTotal = 0;

	// method 1 maxurl 1 found 1

	if (strncpy(outgoingData.code, code[0], strlen(code[0])) != outgoingData.code)
	{
		perror("ERROR strncpy outgoingData.code");
		outgoingData.error++;
	}

	if (snprintf(outgoingData.date, sizeof(outgoingData.date), "Date: %s\r\n", logLine.timezone) != strlen(outgoingData.date))
	{
		perror("ERROR snprintf outgoingData.code");
		outgoingData.error++;
	}

	if (snprintf(outgoingData.server, sizeof(outgoingData.server), SERVERSTRING) != strlen(outgoingData.server))
	{
		perror("ERROR snprintf outgoingData.server");
		outgoingData.error++;
	}

	// minimum error = 404 not found
	if (logLine.found == 0)
	{
		if (strncpy(logLine.relativePath, HTTP404STRING, HTTP404STRINGLEN) != logLine.relativePath)
		{
			perror("ERROR error strncpy logLine.relativePathCorrected");
			exit(EXIT_FAILURE);
		}
	}

	// medium error = 500 internal server error
	if (logLine.error > 0 && logLine.maxurl == 1 && logLine.found == 1)
	{
		if (strncpy(logLine.relativePath, HTTP500STRING, HTTP500STRINGLEN) != logLine.relativePath)
		{
			perror("ERROR error strncpy logLine.relativePathCorrected");
			exit(EXIT_FAILURE);
		}
	}

	// hardest error 414 URL too long, it overrides the others
	if (logLine.maxurl == 0)
	{
		if (strncpy(logLine.relativePath, HTTP414STRING, HTTP414STRINGLEN) != logLine.relativePath)
		{
			perror("ERROR error strncpy logLine.relativePathCorrected");
			exit(EXIT_FAILURE);
		}
	}

	if (snprintf(outgoingData.relativePathCorrected, sizeof(outgoingData.relativePathCorrected), PATHEXECUTEABLE, logLine.relativePath) != strlen(outgoingData.relativePathCorrected))
	{
		perror("ERROR snprintf tempPath");
		outgoingData.error++;
	}
	filedescriptor = open(outgoingData.relativePathCorrected, O_RDONLY);
	outgoingData.lengthTotal = lseek(filedescriptor, 0, SEEK_END);
	if (lseek(filedescriptor, 0, SEEK_SET) == (off_t)-1)
	{
		perror("ERROR seek file");
		outgoingData.exit++;
	}
	if (close(filedescriptor) == -1)
	{
		perror("ERROR close file after seek");
		outgoingData.error++;
	}
	if (snprintf(outgoingData.contentlength, sizeof(outgoingData.contentlength), "Content-Length: %d\r\n", outgoingData.lengthTotal) != strlen(outgoingData.contentlength))
	{
		perror("ERROR snprintf outgoingData.contentlength");
		outgoingData.error++;
	}

	if (snprintf(outgoingData.connection, sizeof(outgoingData.connection), CONNECTIONSTRING) != strlen(outgoingData.connection))
	{
		perror("ERROR snprintf outgoingData.server");
		outgoingData.error++;
	}

	if (snprintf(outgoingData.allowed, sizeof(outgoingData.allowed), METHODSALLOWED) != strlen(outgoingData.allowed))
	{
		perror("ERROR snprintf outgoingData.server");
		outgoingData.error++;
	}

	FILEEXTSWITCHCASE

	if (snprintf(outgoingData.content, sizeof(outgoingData.content), "Content-Type: %s\r\n", type[typeID]) != strlen(outgoingData.content))
	{
		perror("ERROR snprintf outgoingData.content");
		outgoingData.error++;
	}

	return outgoingData;
}

