#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
/************************************/
/*  CSE-344 HOMEWORK-III            */
/*  Author: Islam Goktan SELCUK     */
/*  Student No: 141044071           */
/*  Usage: ./main                   */
/************************************/

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

void cat();

int main() {
	cat();
	
	return 0;
}

void cat() {
	char temp[2];
    size_t bytes = fread(temp,  sizeof(char), 1, stdin);
    while(bytes > 0) {
    	fwrite(temp, sizeof(char), 1, stdout);	
    	bytes = fread(temp, sizeof(char), 1, stdin);
    }
}
