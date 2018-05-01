#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
/* References: */
/* Matrix Deteminant Function: */
/* http://www.programmingforums.org/thread5882.html */

#define FIFO_PERMS (S_IRWXU | S_IWGRP | S_IWOTH)

static volatile sig_atomic_t sigFlag = 0;

static void ctrlcHandler(int signo) {
	sigFlag = 1;
}

void createMatris(char *matris, int n, int *int_matris);

int det(int *arr, int m);

int main(int argc, char *argv[]) {
	struct timespec sleeptime;
	struct timespec start;
	struct sigaction ctrlcAct;
	int requestfd, tempfd;
	int rval, wval;
	char *matris;
	int *int_matris;
	pid_t child;
	int n;
	int milisec;
	int id;
	long matrixTime;
	FILE *outp;
	int matrixDet;
	int milisec2;
		
	outp = fopen("logServer.txt", "w");
	fprintf(outp, "%10s %10s %20s\n", "Miliseconds", "Pid", "Determinant");
	ctrlcAct.sa_handler = ctrlcHandler;
	ctrlcAct.sa_flags = 0;
	if( (sigemptyset(&ctrlcAct.sa_mask) == -1) || 
		(sigaction(SIGINT, &ctrlcAct, NULL)) ) {
		perror("Failed to set ctrl-c handler");
		return 1;		
	}
	
	if(argc != 4) {
		fprintf(stderr, "Usage: ./timeServer <milisecond> <n> <mainpipe>");
		return 1;
	}
	
	milisec = atoi(argv[1]);
	n = atoi(argv[2]);

	matris = malloc( (4 * n * n + 1) * sizeof(char *));
	int_matris = malloc( (4 * n * n) * sizeof(int *));
	
	milisec2 = milisec % 1000;
	sleeptime.tv_sec = milisec / 1000;	
	sleeptime.tv_nsec = milisec2 * 1000000;
	
	if( mkfifo(argv[3], FIFO_PERMS) == -1 ) {
		/*fprintf(stderr, "Failed to create pipe.\n");*/
		/*return 1;*/
	}	
	if( mkfifo("FIFO2", FIFO_PERMS) == -1 ) {
		/*fprintf(stderr, "Failed to create pipe.\n");*/
		/*return 1;*/
	}
	while( (requestfd = open(argv[3], O_RDONLY)) == -1 && (errno == EINTR) );
	if(requestfd == -1) {
		fprintf(stderr, "Failed to open pipe.\n");
		return 1;
	}	
	while( (tempfd = open("FIFO2", O_WRONLY)) == -1 && (errno == EINTR) );
	if(tempfd == -1) {
		fprintf(stderr, "Failed to open pipe.\n");
		return 1;
	}

	
	while(sigFlag == 0) {
		nanosleep(&sleeptime, NULL);

		while( (rval = read(requestfd, &id, sizeof(int))) == -1 && (errno == EINTR) );
		if(rval == -1) {
			fprintf(stderr, "Failed to read from pipe.\n");
			return 1;
		}
		if( id == 1 ) {
			sigFlag = 1;
			id = -1;
			while((wval = write(tempfd, &id, sizeof(int))) == -1 
						&& (errno == EINTR));
		}
		else {
			if( (child = fork()) == -1 ) {
				perror("Failed to fork");
				return 1;
			}
			if(child == 0) {
				clock_gettime(CLOCK_REALTIME, &start);
				matrixTime = (start.tv_sec * 1000) + (start.tv_nsec / 1000000);
				createMatris(matris, n, int_matris);
				matrixDet = det(int_matris, n);
				if(matrixDet != 0) {
					fprintf(outp, "%10ld %10d %10d\n", matrixTime, id, matrixDet);
					while((wval = write(tempfd, matris, (4 * n * n + 1))) == -1 
							&& (errno == EINTR)); 
				}
				return 0;
			}	
		}
	}	
	
	if(unlink(argv[3]) == -1)
		perror("Failed to remove mainfifo");
	if(unlink("FIFO2") == -1)
		perror("Failed to remove fifo.");
	
	free(matris);
	free(int_matris);
	return 0;
}


void createMatris(char *matris, int n, int *int_matris) {
	int i;
	int number;
	char buf[255];
	srand(time(NULL));
	
	for(i = 0; i < 4*n*n; i++) {
		number = rand() % 10;	
		sprintf(buf, "%d", number);
		strcat(matris, buf);
		int_matris[i] = number;
	}
/*	
	printf("\n");
	for(i = 0; i < 4*n*n; i++) {
		printf("%d ", int_matris[i]);
		if( (i+1) % (2*n) == 0 )
			printf("\n");
	}	
*/
}

int det(int *arr, int m)
{
	double **a;	/* Coefficient matrix */
	int i, j, k;		/* Matrix subscripts */
	double factor;	/* Greatest common factor */
	double temp;	/* Temporary variable */
	int counti;	/* Counts number of I operations */
	int number;
	
	counti = 0;	/* Initialize the I count */


	/* Allocate a one-dimensional array of pointers, one pointer per row */
	a = (double **) calloc(m, sizeof(double *));

	/* Allocate one c-element array of integers for each row */
	for(i = 0; i < m; i++)
	{
		a[i] = (double *) calloc(m, sizeof(double));
	}
	k = 0;
	/* Fill matrix */
	for(i = 0; i < m; i++)
	{
		for(j = 0; j < m; j++)
		{
			number = arr[k];
			a[i][j] = (double)number;
			k++;
		}
	}
	k = 0;
	/* Transform matrix into upper triangular */
	for(i = 0; i < m - 1; i++)
	{
		/* Elementary Row Operation I */
		if(a[i][i] == 0)
		{
			for(k = i; k < m; k++)
			{
				if(a[k][i] != 0)
				{
					for(j = 0; j < m; j++)
					{
						temp = a[i][j];
						a[i][j] = a[k][j];
						a[k][j] = temp;
					}
				k = m;
				}
			}
			counti++;
		}
		/* Elementary Row Operation III */
		if(a[i][i] != 0)
		{
			for(k = i + 1; k < m; k++)
			{
				factor = -1.0 * a[k][i] /  a[i][i];
				for(j = i; j < m; j++)
				{
					a[k][j] = a[k][j] + (factor * a[i][j]);
				}
			}
		}
	}

	temp = 1.0;

	/* Calculate determinant */
	for(i = 0; i < m; i++)
	{
		temp *= a[i][i];
	}
	/* Modify determinant */
	if(counti % 2 != 0)
	{
		temp *= -1.0;
	}
		
	for(i = 0; i < m; i++)
	{
		free(a[i]);
	}
	free(a);
	number = (int)temp;
		
	return (number);
}
