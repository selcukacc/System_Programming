/************************************/
/*  CSE-344 HOMEWORK-III            */
/*  Author: Islam Goktan SELCUK     */
/*  Student No: 141044071           */
/*  Usage: ./main                   */
/************************************/

/*---------------------------------*/
/*            Includes             */
/*---------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

void ls();

int main() {

	ls();
	
	return 0;
}

void ls() {
	// bulunulan directory alinir
	char * curDirectory = malloc(80);
	getcwd(curDirectory, 80);
	DIR *dirp = opendir(curDirectory);
	struct dirent *direntp;
	direntp = readdir(dirp);
	// okunan tum dosyalar ekrana basilir.
	while(direntp != NULL) {
		if(strcmp(direntp->d_name, "..") != 0 && 
			strcmp(direntp->d_name, ".") != 0)
			printf("%s\n", direntp->d_name);
		direntp = readdir(dirp);
	}

	free(curDirectory);
	closedir(dirp);
}
