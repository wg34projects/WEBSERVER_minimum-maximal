#include "server.h"

int main (int argc, char *argv[])
{
	// some vars
	int connfd = 0, optval = 0, pipeCPchar[2], overhandedPort = 0, i = 0, pidSave = 0, rvGetOpt = 0, inputCount = 0;
	char textInBuffer[TEXTLEN], textFromConnection[TEXTLEN];
	const char *category[] = { "____system", "______root", "____logger", "____server" , "___handler" , "____closer" };
	unsigned long int serverInfoCounter = 0;	//	overall counter for info
	bool endless = true;
	SOCKADDRIN servaddr, cliaddr;
	socklen_t len;
	LOGFILE logLine;
	TEXTOUTGOING textOutBuffer;
	SIGACTION sa;
	pid_t forkPID1, forkPID2;

	char textOutBufferheader[] = "HTTP/1.1 200 OK\r\nDate: 21.06.17\r\nServer: my_new_min_max_www_server\r\nConnection: close\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";
	char textOutBufferload[] = "<!DOCTYPE html><html><head><title>my new funky webserver</title></head><body><h1>it is just a first test of my new webserver concept, come back soon...</h1></body></html>\r\n\r\n";

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
			exit(EXIT_FAILURE);
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
		printf("\n");

		// set pipe one time and keep open, close only on error
		if (close(pipeCPchar[1]) == -1)
		{
			perror("ERROR closing pipe 1");
			exit(EXIT_FAILURE);
		}

		// endless loop for logger
		while(endless)
		{
			if (read(pipeCPchar[0], textFromConnection, PIPE_BUF) == -1)
			{
				perror("ERROR reading pipe 0");
				break;	// clean exit as mentioned above
			}
			for (i = 0; i < (int)(strlen(textFromConnection)); i++)
			{
				if (textFromConnection[i] == '\n')
				{
					textFromConnection[i] = '\0';
				}
			}

#if DEBUGPIPE

			// screeninfo
			screenInfo(category[2], 3, textFromConnection, 0);
			printf("\n");
#endif

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

				// screeninfo
				screenInfo(category[4], 3, textInBuffer, 0));
#endif

				// decode input	and send output and receive message to log
				logLine = decodeIncoming(textInBuffer, connfd);

				// safety warning after RISK warning and exit
				if (logLine.exit != 0)
				{
					// safety exit
					screenInfo(category[1], 0, "exit trigger occured total", logLine.exit);
					exit(EXIT_FAILURE);
				}
				else
				{
					// info about status and proceed
					screenInfo(category[1], 2, "no exit trigger occured", 0);
				}
				
				// if safe but other errors warning and exit
				if (logLine.error != 0)
				{
					// safety exit
					screenInfo(category[1], 0, "error trigger occured total", logLine.error);
					exit(EXIT_FAILURE);
				}
				else
				{
					// info about status and proceed
					screenInfo(category[1], 2, "no error trigger occured", logLine.error);
				}

				// collect data

				//textOutBuffer = codeOutgoing();
				//codeOutgoing(const char *code, const char *server)
				
				// send header
				if (logLine.method == 1)
				{
					// GET
					if (send(connfd, textOutBufferheader, strlen(textOutBufferheader), MSG_NOSIGNAL) == -1)
					{
						perror("error send HTML header");
						exit(EXIT_FAILURE);
					}
				}
				else
				{
					// HEAD
				}

				// send load like foreseen
				if (logLine.method == 1)
				{
					if (send(connfd, logLine.logLineFile, strlen(logLine.logLineFile), MSG_NOSIGNAL) == -1)
					{
						perror("error send HTML load");
						exit(EXIT_FAILURE);
					}
				}

				// set pipe
				if (close(pipeCPchar[0]) == -1)
				{
					perror("ERROR closing pipe 0");
					exit(EXIT_FAILURE);
				}

				// write to pipe
				if (write(pipeCPchar[1], logLine.logLineFile, PIPE_BUF) != (ssize_t)(PIPE_BUF))
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
