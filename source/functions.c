#include "server.h"

void ctrlChandler(int /*@unused@*/dummy)
{
	CATEXITVAR
	// close open port
	if (close(listenfd) == -1)
	{
		perror("ERROR closing listenfd");
	}
	screenInfo(category, 3, "you typed CTRL-C, message appears 2 times, exit ", 0);
	exit(EXIT_SUCCESS);
}

void wait_for_child(int /*@unused@*/sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

int getInteger(char *input, int *numInteger, int lo, int hi)
{
	unsigned long int number = 0;
	char *pointToEnd = NULL;

	number = strtoul(input, &pointToEnd, 0);
	if((int)number < lo || (int)number > hi || *pointToEnd != '\0')
	{
		return 1;
	}
	else
	{
		*numInteger = (int)number;
		return 0;
	}
}

void screenLog(const char name[11], long unsigned int counter, int pid, int ppid, int topid)
{
	char infoString[LINELEN];

	if (snprintf(infoString, sizeof(infoString),"%s\tNO[%6lu]\tPID[%6d]\tPPID[%6d]\tTOPID[%6d]\n", name, counter, pid, ppid, topid) == (int)(strlen(infoString)))
	{
		if (write(STDOUT_FILENO, infoString, strlen(infoString)) == -1)
		{
			perror("ERROR writing log to screen");
		}
	}
}

void screenInfo(const char *category, int goodbad, char *string, int number)
{
	if (goodbad == 0)
	{
		printf("%s\tSECURITY!\t%s: %d\n", category, string, number);
	}
	else if (goodbad == 1)
	{
		printf("%s\tWARNING! \t%s: %d\n", category, string, number);
	}
	else if (goodbad == 2)
	{
		printf("%s\tINFO!    \t%s\n", category, string);
	}
	else if (goodbad == 3)
	{
		printf("%s\t%s\n", category, string);
	}
	else if (goodbad == 4)
	{
		printf("%s\tINFO!    \t%s: %d\n", category, string, number);
	}
}

int readIncoming(char *textInBuffer, TEXTINCOMING **text)
{
	int length = 0, lines = 0, i = 0, j = 0, k = 0, temp = 0;
	char **inText, *lineToken = NULL, *saveptr = NULL;

	length = strlen(textInBuffer);
	for (i = 0; i < length; i++)
	{
		if(textInBuffer[i] == '\n')
		{
			lines++;
		}
	}
	lines--;
	if (lines == 0)
	{
		return 0;
	}

	inText = (char **) calloc((size_t)(lines), (sizeof(char *)));
	if (inText == NULL)
	{
		perror("ERROR calloc inText");
		return 0;
	}
	for (i = 0; i < lines; i++)
	{
		inText[i] = (char *) calloc((size_t)(LINELEN), (sizeof(char)));
		if(inText[i] == NULL)
		{
			perror("ERROR calloc inText[i]");
			free(inText);
			return 0;
		}
	}

	lineToken = strtok_r(textInBuffer, "\n", &saveptr);
	while (k < lines)
	{
		temp = 0;
		if (strncpy(inText[k], lineToken, strlen(lineToken)) != inText[k])
		{
			perror("ERROR strncpy strtok_r textInBuffer");
			for (i = 0; i < lines; i++)
			{
				free(inText[k]);
			}
			free(inText);
			return 0;
		}
		temp = strlen(inText[k]);
		for (j = 0; j < (int)temp; j++)
		{
			if (inText[k][j] == '\r' || inText[k][j] == '\n')
			{
				inText[k][j] = '\0';
			}
		}
		inText[k][temp] = '\0';

		TEXTINCOMING *pAct;
		pAct = (TEXTINCOMING *) malloc (1 * sizeof(TEXTINCOMING));
		if (pAct == NULL)
		{
			perror("ERROR allocating memory for incoming list");
			return 0;
		}
		if (strncpy(pAct->incomeVar, inText[k], temp) != pAct->incomeVar)
		{
			perror("ERROR strncpy to incoming list");
			return 0;
		}
		pAct->incomeVar[temp] = '\0';
		if(*text == NULL)
		{
			pAct->pNext = NULL;
			*text = pAct;
		}
		else
		{
			pAct->pNext = *text;
			*text = pAct;
		}
		lineToken = strtok_r(NULL, "\n", &saveptr);
		k++;
	}

	for (i = 0; i < lines; i++)
	{
		free(inText[i]);
	}
	free(inText);

	return lines;
}

void freeIncoming(TEXTINCOMING **text)
{
	TEXTINCOMING *pAct;

	if (*text == NULL)
	{
		return;
	}
	while(1)
	{
		pAct = *text;
		if(pAct->pNext == NULL)
		{
			free(pAct);
			*text= NULL;
			break;
		}
		else
		{
			*text = pAct->pNext;
			free(pAct);
		}
	}
}

void showIncoming(TEXTINCOMING **text)
{
	TEXTINCOMING *pAct;
	CATINCOMINGVAR

	if (text == NULL)
	{
		return;
	}
	pAct = *text;
	while(1)
	{
		screenInfo(category, 3, pAct->incomeVar, 0);
		if (pAct->pNext == NULL)
		{
			break;
		}
		else
		{
			pAct = pAct->pNext;
		}
	}
}

int findIncoming(TEXTINCOMING *text, const char *look, int lines, char **output)
{
	TEXTINCOMING *pAct;

	pAct = text;
	while(1)
	{
		if(strncmp(look, pAct->incomeVar, strlen(look)) == 0)
		{
			if (strncpy(*output, pAct->incomeVar, strlen(pAct->incomeVar)) != *output)
			{
				perror("ERROR strncpy find incoming");
				return 0;
			}
			else
			{
				return 1;
			}
		}
		pAct = pAct->pNext;
		if (pAct->pNext == NULL)
		{
			if(strncmp(look, pAct->incomeVar, strlen(look)) == 0)
			{
				if (strncpy(*output, pAct->incomeVar, strlen(pAct->incomeVar)) != *output)
				{
					perror("ERROR strncpy find incoming list");
					return 0;
				}
				else
				{
					return 1;
				}
			}
			else
			{
				return 0;
			}
		}
	}
}

int readAllowed(ALLOWEDPATH **path)
{
	int level = 0;
	ALLOWEDPATH *pAct;

	// add manual / path
	pAct = (ALLOWEDPATH *) malloc (1 * sizeof(ALLOWEDPATH));
	if (pAct == NULL)
	{
		perror("ERROR allocating memory list allowed");
		return 0;
	}
	if (strncpy(pAct->pathVar, SPECIALPATH1, SPECIALPATH1LENGTH) != pAct->pathVar)
	{
		perror("ERROR strncpy to allowed list");
		return 0;
	}
	pAct->pathVar[1] = '\0';
	if(*path == NULL)
	{
		pAct->pNext = NULL;
		*path = pAct;
	}
	else
	{
		pAct->pNext = *path;
		*path = pAct;
	}

	recursiveWalk(&(*path), "..", level);
	return 1;
}

int recursiveWalk(ALLOWEDPATH **path, const char *pathName, int level)
{
	DIR *dir;
	DIRENT *entry;
	int temp = 0;
	FILEEXTSTRING;

	if (!(dir = opendir(pathName)))
	{
		perror("ERROR open directory");
		return 0;
	}
	if (!(entry = readdir(dir)))
	{
		perror("ERROR read directory");
		return 0;
	}
	do
	{
		char fullpath[1024];
		if (snprintf(fullpath, sizeof(fullpath)-1, "%s/%s", pathName, entry->d_name) == 0)
		{
			continue;
		}
		if (entry->d_type == DT_DIR)
		{
			if (strncmp(entry->d_name, ".", 1) == 0 || strncmp(entry->d_name, "..", 2) == 0 || strncmp(entry->d_name, "source", 6) == 0)
			{
				continue;
			}
			recursiveWalk(&(*path), fullpath, level + 1);
		}
		else
		{
			int countOK = 0;
			for (int i = 1; i < FILEXTCOUNT; i++)
			{
				if (strncmp(&(entry->d_name[strlen(entry->d_name)-3]), ext[i], 3) == 0)
				{
					countOK = 1;
				}
			}
			if (strncmp(&(entry->d_name[strlen(entry->d_name)-4]), ext[0], 4) == 0)
			{
				countOK = 1;
			}
			if (countOK == 0)
			{
				continue;
			}
			temp = strlen(&(fullpath[2]));
			ALLOWEDPATH *pAct;
			pAct = (ALLOWEDPATH *) malloc (1 * sizeof(ALLOWEDPATH));
			if (pAct == NULL)
			{
				perror("ERROR allocating memory list allowed");
				return 0;
			}
			if (strncpy(pAct->pathVar, &(fullpath[2]), temp) != pAct->pathVar)
			{
				perror("ERROR strncpy to allowed list");
				return 0;
			}
			pAct->pathVar[temp] = '\0';
			if(*path == NULL)
			{
				pAct->pNext = NULL;
				*path = pAct;
			}
			else
			{
				pAct->pNext = *path;
				*path = pAct;
			}
		}
	}
	while ((entry = readdir(dir)));
	closedir(dir);
	return 1;
}

void freeAllowed(ALLOWEDPATH **path)
{
	ALLOWEDPATH *pAct;

	if (*path == NULL)
	{
		return;
	}
	while(1)
	{
		pAct = *path;
		if(pAct->pNext == NULL)
		{
			free(pAct);
			*path= NULL;
			break;
		}
		else
		{
			*path = pAct->pNext;
			free(pAct);
		}
	}
}

void showAllowed(ALLOWEDPATH **path)
{
	ALLOWEDPATH *pAct;
	CATALLOWEDVAR

	if (path == NULL)
	{
		return;
	}
	pAct = *path;
	while(1)
	{
		screenInfo(category, 3, pAct->pathVar, 0);
		if (pAct->pNext == NULL)
		{
			break;
		}
		else
		{
			pAct = pAct->pNext;
		}
	}
}

int findAllowed(ALLOWEDPATH *path, const char *look, int lines)
{
	ALLOWEDPATH *pAct;
	int count = 0, temp1 = 0, temp2 = 0, temp3 = 0;

	pAct = path;

	temp1 = strlen(pAct->pathVar);
	temp2 = strlen(look);

	if (temp1 > temp2)
	{
		temp3 = temp1;
	}
	else
	{
		temp3 = temp2;
	}
	while(1)
	{
		if(strncmp(pAct->pathVar, look, temp3) == 0)
		{
			return 1;
		}
		pAct = pAct->pNext;
		count++;
		if (pAct->pNext == NULL)
		{
			if(strncmp(pAct->pathVar, look, temp3) == 0)
			{
				return 1;
			}
			else
			{
				if (look[0] == '/' && temp2 == 1)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
	}
}

int getRelativePath(char *input, char **output)
{
	char *start = NULL;
	size_t end = 0;

	start = strchr(input, '/');
	if(start == NULL)
	{
		perror("ERROR strchr / relative path");
		return 0;
	}
	end = strlen(start) - URLENDING;
	if(start[0] == '/' && end == 1)
	{
		if (strncpy(*output, SPECIALPATH2, SPECIALPATH2LENGTH) != *output)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else if (strncpy(*output, start, (size_t)(end)) != *output)
	{
		return 0;
	}
	else
	{
		if(strncmp(*output, SPECIALPATH3, SPECIALPATH3LENGTH) == 0)
		{
			if (strncpy(*output, SPECIALPATH2, SPECIALPATH2LENGTH) != *output)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
		else
		{
			return 1;
		}
	}
}

void help()
{

}
