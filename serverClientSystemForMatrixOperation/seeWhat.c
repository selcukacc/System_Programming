#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

#define FIFO_PERMS (S_IRWXU | S_IWGRP | S_IWOTH)

static volatile sig_atomic_t sigFlag = 0;

static void ctrlcHandler(int signo) {
	sigFlag = 1;
}

int main(int argc, char *argv[]) {
	
	struct sigaction act;
	int len;	
	char buf2[400];
	int **arr;
	int requestfd;
	int readfd;
	int process;
	int i, j, k;
	int n;
	char ch;
	
	if(argc != 2) {
		fprintf(stderr, "Usage: ./seeWhat <mainpipe>\n");
		return 1;
	}
	
	act.sa_handler = ctrlcHandler;
	act.sa_flags = 0;
	if( (sigemptyset(&act.sa_mask) == -1) || 
		(sigaction(SIGINT, &act, NULL)) ) {
		perror("Failed to set ctrl-c handler");
		return 1;		
	}
	
	requestfd = open(argv[1], O_WRONLY);
	if(requestfd == -1) {
		fprintf(stderr, "Failed to open pipe.\n");
		return 1;
	}
	readfd = open("FIFO2", O_RDONLY);
	if(readfd == -1) {
		fprintf(stderr, "Failed to open pipe.\n");
		return 1;
	}
	process = getpid();
		
	while(!sigFlag) {
		write(requestfd, &process, sizeof(int));	
		read(readfd, buf2, 400);
		fprintf(stderr, "\n");
		len = strlen(buf2);
		n = sqrt(len);
		if(n >= 2) {		
			arr = (int **) calloc(n, sizeof(int *));
			for(i = 0; i < n; i++)
			{
				arr[i] = (int *) calloc(n, sizeof(int));
			}
			k = 0;
			/* Fill matrix */
			for(i = 0; i < n; i++)
			{
				for(j = 0; j < n; j++)
				{
					ch = buf2[k];
					arr[i][j] = ch - '0';
					k++;
				}
			}
			for(i = 0; i < n; i++)
			{
				for(j = 0; j < n; j++)
				{
					printf("%d ",arr[i][j]);
				}
				printf("\n");
			}
		}	
		if(strncmp(buf2, "-", 1) == 0)
			sigFlag = 1;
	}
	process = 1;	
	write(requestfd, &process, sizeof(int));


	close(requestfd);
	close(readfd);
	return 0;	
}
