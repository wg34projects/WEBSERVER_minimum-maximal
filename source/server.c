#include "server.h"

int main (int argc, char *argv[])
{
	int connfd = 0, optval = 0, pipeCPchar[2], overhandedPort = 0, i = 0, pidSave = 0, rvGetOpt = 0, inputCount = 0, errorcountlog = 0;
	char textInBuffer[TEXTLEN], textFromConnection[TEXTLEN], logLineFile[PIPE_BUF], sendBuffer[TEXTLENSEND], logFileName[27];
	unsigned long int serverInfoCounter = 0;	//	overall counter for info
	bool endless = true;
	pid_t forkPID1, forkPID2;
	socklen_t len;
	SIGACTION sa;
	SOCKADDRIN servaddr, cliaddr;
	LOGFILE logLine;
	FILE *logFile;
	TEXTOUTGOING textOutBuffer;
	CATFULLVAR

	// init
	memset(textInBuffer, 0, sizeof(textInBuffer));
	memset(textFromConnection, 0 , sizeof(textFromConnection));
	memset(logLineFile, 0 , sizeof(logLineFile));
	memset(sendBuffer, 0 , sizeof(sendBuffer));
	memset(logFileName, 0 , sizeof(logFileName));

	// screen output and check if enough arguments were provided from command line
	printf("\n");
	screenInfo(category[0], 3, "minimum maximum HTTP/1.1 server project helmut.resch@gmail.com", 0);
	printf("\n");

	while((rvGetOpt = getopt(argc, argv, "p:h")) != -1)
	{
		switch(rvGetOpt)
		{
		case 'p':
		{
			inputCount++;
			if (getInteger(optarg, &overhandedPort, 0, 65535) != 0)
			{
				screenInfo(category[0], 1, "error port number conversion and/or range 0 - 65.535", 0);
				help();
				exit(EXIT_FAILURE);
			}
			break;
		}
		case 'h':
		{
			inputCount++;
			help();
			exit(EXIT_SUCCESS);
		}
		default:
		{
			help();
			exit(EXIT_SUCCESS);
		}
		}
	}
	if (inputCount == 0)
	{
		screenInfo(category[0], 1, "not sufficient options, port 0 - 65.535 must be given", 0);
		help();
		exit(EXIT_FAILURE);
	}
	if (inputCount > 1)
	{
		screenInfo(category[0], 1, "too many command line options given", 0);
		help();
		exit(EXIT_FAILURE);
	}

	// socket to listen
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1)
	{
		perror("ERROR listen socket");
		exit(EXIT_FAILURE);
	}

	// setsockopt free previously used sockets
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)(sizeof(optval))) == -1)
	{
		perror("ERROR setsockopt");
		exit(EXIT_FAILURE);
	}

	// fill the structure required to handle bind()
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;

	// htonl and htons convert values between host and network byte order
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons((uint16_t)(overhandedPort));

	// bind
	if (bind(listenfd, (struct sockaddr *) &servaddr, (socklen_t)(sizeof(servaddr))) == -1)
	{
		perror("ERROR bind");
		exit(EXIT_FAILURE);
	}

	// listen
	if (listen(listenfd, TEXTLEN) == -1)
	{
		perror("ERROR listen");
		exit(EXIT_FAILURE);
	}

	// install CTRL-C handler
	if (signal(SIGINT, ctrlChandler) == SIG_ERR)
	{
		perror("ERROR while setting signal handler CTRL-C");
		exit(EXIT_FAILURE);
	}

	// zombie and sigchld handler
	sa.sa_handler = wait_for_child;
	if (sigemptyset(&sa.sa_mask) == -1)
	{
		perror("ERROR sigemptyset");
		return 1;
	}
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("ERROR sigaction");
		return 1;
	}

	// setup pipe
	if (pipe(pipeCPchar) == -1)
	{
		perror ("ERROR pipe setup");
		exit(EXIT_FAILURE);
	}

	// info about main server process
	pidSave = getpid();
	screenLog(category[1], serverInfoCounter, 0, 0, pidSave);

	// fork log process = child, parent = server
	forkPID1 = (pid_t)(fork());
	if (forkPID1 == 0)
	{
		// child code PID1 log
		screenLog(category[2], serverInfoCounter, (int)(getpid()), (int)(getppid()), pidSave);

		// set pipe one time and keep open, close only on error
		if (close(pipeCPchar[1]) == -1)
		{
			perror("ERROR closing pipe 1");
			errorcountlog++;
		}

		// endless loop for logger
		while(endless)
		{
			if (read(pipeCPchar[0], textFromConnection, PIPE_BUF) == -1)
			{
				perror("ERROR reading pipe 0");
				errorcountlog++;
			}
			for (i = 0; i < (int)(strlen(textFromConnection)); i++)
			{
				if (textFromConnection[i] == '\n')
				{
					textFromConnection[i] = '\0';
				}
			}
#if DEBUGPIPE
			screenInfo(category[2], 3, textFromConnection, 0);
#endif
			// logfile disk

			if (snprintf(logFileName, sizeof(logFileName), "../logging/min-max-www.txt") != (strlen(logFileName)))
			{
				perror("ERROR sprintf log file name");
				errorcountlog++;
			}

			logFile = fopen(logFileName, "a");

			if (logFile == NULL)
			{
				perror("ERROR opening log file, check permissions");
				errorcountlog++;
			}

			if (fprintf(logFile, "%s\n", textFromConnection) < 1)
			{
				perror("ERROR writing to logfile");
				errorcountlog++;
			}

#if RASPBERRYPI

			if (fflush(logFile) == EOF)
			{
				perror("ERROR flush to SD raspberry");
				errorcountlog++;
			}
#endif

			if (fclose(logFile) != 0)
			{
				printf("ERROR closing log file, check permissions");
				errorcountlog++;
			}

			if (errorcountlog == 0)
			{
				screenInfo(category[1], 2, "log written", 0);
			}
			else
			{
				screenInfo(category[1], 1, "errors during logging, check filesystem", 0);
			}
			errorcountlog = 0;
		}

		// close pipe only on error above
		if (close(pipeCPchar[0]) == -1)
		{
			perror("ERROR closing pipe 0");
			exit(EXIT_FAILURE);
		}

		// if logging loop dies, end server
		exit(EXIT_FAILURE);
	}
	else if (forkPID1 == -1)
	{
		// fork error PID1
		perror("ERROR fork PID1");
		exit(EXIT_FAILURE);
	}
	else
	{
		// parent code PID1 server
		screenLog(category[3], serverInfoCounter, (int)(getpid()), (int)(getppid()), pidSave);

		// endless loop for server
		while(endless)
		{
			serverInfoCounter ++;	// just a counter

			// accept
			len = (socklen_t)(sizeof(cliaddr));
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
			if (connfd == -1)
			{
				perror("ERROR accept");
				exit(EXIT_FAILURE);
			}

			// new child for every accept
			forkPID2 = (pid_t)(fork());
			if (forkPID2 == 0)
			{
				// child code PID2 handling request
				screenLog(category[4], serverInfoCounter, (int)(getpid()), (int)(getppid()), pidSave);

				// close listen port
				if (close(listenfd) == -1)
				{
					perror("ERROR close listen port");
					exit(EXIT_FAILURE);
				}

				// read infos
				if (recv(connfd, textInBuffer, TEXTLEN, 0) == -1)
				{
					perror("ERROR receive from client");
					exit(EXIT_FAILURE);
				}

#if DEBUGINRAW
				screenInfo(category[4], 3, textInBuffer, 0));
#endif

				// decode input	and send output and receive message to log = incoming.c
				logLine = decodeIncoming(textInBuffer, connfd);

				// prepare string for pipe
				if (snprintf(logLineFile, PIPE_BUF, "URL %s%s | %ju s since Epoch | local time: %s | %s | peer IP %s:%d",
							 logLine.homeAdress, logLine.relativePath, logLine.timeEpoch, logLine.timezone, logLine.agent, logLine.ipAddr, logLine.portNo) != strlen(logLineFile))
				{
					perror("ERROR snprintf logLineFile");
					logLine.error++;
				}
				// set pipe
				if (close(pipeCPchar[0]) == -1)
				{
					perror("ERROR closing pipe 0");
					exit(EXIT_FAILURE);
				}

				// write to pipe
				if (write(pipeCPchar[1], logLineFile, PIPE_BUF) != (ssize_t)(PIPE_BUF))
				{
					perror("ERROR writing pipe 1");
					exit (EXIT_FAILURE);
				}

				// close pipe
				if (close(pipeCPchar[1]) == -1)
				{
					perror("ERROR closing pipe 1");
					exit(EXIT_FAILURE);
				}

				screenInfo(category[1], 2, logLine.ipAddr, 0);
				screenInfo(category[1], 2, logLine.timezone, 0);
				screenInfo(category[1], 2, logLine.relativePath, 0);

				// exit check without answer
				if (logLine.exit != 0)
			{
				screenInfo(category[1], 0, "exit trigger total", logLine.exit);
					screenInfo(category[1], 4, "no IP addr. or no HOST received", 0);
					screenInfo(category[1], 4, "closing connection without response", 0);
					exit(EXIT_FAILURE);
				}
				else
				{
					screenInfo(category[1], 2, "no exit trigger", 0);
				}

				// error status information
				if (logLine.error != 0)
			{
				screenInfo(category[1], 1, "error trigger total", logLine.error);
					screenInfo(category[1], 4, "responding accordingly", 0);
				}
				else
				{
					screenInfo(category[1], 2, "no error trigger", logLine.error);
				}

				// organize outgoing informations, strings, etc.
				textOutBuffer = codeOutgoingHeader(logLine);

				// send header data
				if (logLine.method == 0)
			{
				// header for HTTP/1.1 405 Method Not Allowed
				if (snprintf(sendBuffer, sizeof(sendBuffer), "%s%s\r\n", textOutBuffer.code, textOutBuffer.allowed) != strlen(sendBuffer))
					{
						perror("ERROR snprintf sendbuffer");
					}
				}
				else
				{
					// header data for HTTP/1.1 200 OK, HTTP/1.1 404 Not Found, HTTP/1.1 414 Request-URL Too Long, HTTP/1.1 500 Internal Server Error
					if (snprintf(sendBuffer, sizeof(sendBuffer), "%s%s%s%s%s%s\r\n", textOutBuffer.code, textOutBuffer.date, textOutBuffer.server, textOutBuffer.contentlength, textOutBuffer.connection, textOutBuffer.content) != strlen(sendBuffer))
					{
						perror("ERROR snprintf sendbuffer");
					}

				}

				// send header
				if (send(connfd, sendBuffer, strlen(sendBuffer), MSG_NOSIGNAL) == -1)
				{
					perror("ERROR send HTML header");
					exit(EXIT_FAILURE);
				}

				// now the real content after GET Method

				int retSend = 0;
				int filetosend = 0;
				memset(sendBuffer, 0 , sizeof(sendBuffer));
				filetosend = open(textOutBuffer.relativePathCorrected, O_RDONLY);
				while ((retSend = read(filetosend, sendBuffer, textOutBuffer.lengthTotal)) > 0)
				{
					if (send(connfd, sendBuffer, retSend, MSG_NOSIGNAL) == -1)
					{
						perror("ERROR send data");
						exit(EXIT_FAILURE);
					}
				}
				if (close(filetosend) == -1)
				{
					perror("ERROR close file after send");
					exit(EXIT_FAILURE);
				}

				// connection handled without problems, SIGCHLD by parent
				exit(EXIT_SUCCESS);
			}
			else if (forkPID1 == -1)
			{
				// fork error PID1
				perror("ERROR fork PID2");
				exit(EXIT_FAILURE);
			}
			else
			{
				// parent code PID2 SIGCHLD
				screenLog(category[5], serverInfoCounter, (int)(getpid()), (int)(getppid()), pidSave);
				if (waitpid((__pid_t)(forkPID2), NULL, WNOHANG) != 0)
				{
					perror("ERROR wait pid2");
					exit(EXIT_FAILURE);
				}
				if (close(connfd) == -1)
				{
					perror("ERROR closing connected socket");
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}
