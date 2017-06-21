#include "server.h"

TEXTOUTGOING codeOutgoing(const char *code, char *date)
{
	TEXTOUTGOING outgoingData;

	if (strncpy(outgoingData.code, code, strlen(code)) != outgoingData.code)
	{
		perror("ERROR strncpy outgoingData.code");
	}


/*struct textOutgoing*/
/*{*/
/*	char code[LINELEN];*/
/*	char date[LINELEN];*/
/*	char server[LINELEN];*/
/*	char contentlength[LINELEN];*/
/*	char connection[LINELEN];*/
/*	char allowed[LINELEN];*/
/*	char content[LINELEN];*/
/*	char load[TEXTLEN];*/
/*};*/


	return outgoingData;

}

