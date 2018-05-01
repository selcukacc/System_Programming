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
#include <sys/wait.h>


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\\
// wc komutu implement edilmedi.\\
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\\


// cd komutunun fonksiyonu
int cd(char* cdCommand, char* token);
// programin bulundugu directory'i degistirir.
void changeDir(char* lsPath, char* cdCommand);
// help komutunun fonksiyonu
void help();
// yazilacak dosyanin uygunlugunu kontrol eder.
int isFile(char* fileName);
// ismini token parametresinden aldigi dosyayi okur.
void readFile(char* token);

int main() {
	//exec icin
	char* arg_list[] = {
		NULL
	};

	int i = 0; // toplam girilen komut sayisini tutar
	char lsPath[1024]; // exec parametresine gonderilen ls programinin adresi
	char catPath[1024]; // cat programinin adresi
	char curPath[1024]; // gecerli adres

	getcwd(curPath, 1024);
	/* Program bulundugu path'den ilerledigi icin sadece program isimleri
		atandi. (exec fonksiyonu icin) 
	*/
	sprintf(lsPath, "./lsCom");
	sprintf(catPath, "./catCom");
	char cdCommand[1024];	// cd icin gonderilen komut
	char allCommands[120][80]; // shell'e yazilan tum komutlar
	int totalCommand = 0; // shell'e yazilan tum komutlarin sayisi
	char c[80] = {};
	char *command = c; // alinan gecerli komut
	size_t size = 80;
	while(1) {
		// kullanicidan komut alinir
		printf("\n-->%s$ ", curPath);
		getline(&command, &size,stdin);
		if(strlen(command) > 1)
			command[strlen(command) - 1] = '\0';

		// history komutu girilmedigi surece komutlar kaydedilir.
		if(strcmp(command,"history") != 0) {
			strcpy(allCommands[i], command);
			totalCommand = ++i;		
		}
		else 
			totalCommand--;
		
		char t[80];
		char *token = t;
		token = strtok(command, " ");

		// ls komutu girildiginde yapilacaklar
		if(token != NULL && strcmp("ls", token) == 0) {
			int pfd[2];
			pipe(pfd);
			//exec ile ls programi calistirilir ve sonuc pipe ile standart output'a
			// yollanir
			if(fork() == 0) {
				close(pfd[0]);
				dup2(pfd[1], STDOUT_FILENO);
				execvp(lsPath, arg_list);	
				return 0;
			}

			// ls'in devaminda girilen komut varsa devam edilir.
			token = strtok(NULL, " ");
			if(token != NULL && (strcmp("|", token) == 0 || strcmp(">", token) == 0)) {
				token = strtok(NULL, " ");
				if(token != NULL && strcmp("cat", token) == 0) {			
					token = strtok(NULL, " ");
					if(token != NULL && strcmp(">", token) == 0) {
						token = strtok(NULL, " ");
						// cat'den sonra > komutu girildiginde cikti pipe ile ls'den 
						// alinarak belirtilen dosyaya kaydedilir.
						if(token != NULL && fork() == 0) {
							FILE *output = fopen(token, "w+");
							close(pfd[1]);
							dup2(pfd[0], STDIN_FILENO);
							close(pfd[0]);

							char temp[2];
							size_t bytes = fread(temp,  sizeof(char), 1, stdin);
							while(bytes > 0) {
								fwrite(temp, sizeof(char), 1, output);	
								bytes = fread(temp, sizeof(char), 1, stdin);
							}
							fclose(output);
							return 0;
						}
					}
					else // eger ls'den sonra komut girilmediyse cıktı cat ile ekrana yazdirilir.
						if(fork() == 0) {
							close(pfd[1]);
							dup2(pfd[0], STDIN_FILENO);
							close(pfd[0]);
							execvp(catPath, arg_list);
							return 0;
						}
				}
				// ls'in ciktisi dosyaya yazdirilir.
				else if(token != NULL && isFile(token)) {
					if(fork() == 0) {
						FILE *output = fopen(token, "w+");
						close(pfd[1]);
						dup2(pfd[0], STDIN_FILENO);
						close(pfd[0]);

						char temp[2];
						size_t bytes = fread(temp,  sizeof(char), 1, stdin);
						while(bytes > 0) {
							fwrite(temp, sizeof(char), 1, output);	
							bytes = fread(temp, sizeof(char), 1, stdin);
						}
						fclose(output);
						return 0;
					}

				}
			}
			else if(token == NULL) {
				if(fork() == 0) {
					close(pfd[1]);
					dup2(pfd[0], STDIN_FILENO);
					close(pfd[0]);
					execvp(catPath, arg_list);
					return 0;
				}
			}
			// pipe dosyalari kapatilir ve cocuklarin bitirilmesi beklenir.
			close(pfd[0]);
			close(pfd[1]);
			wait(NULL);
			wait(NULL);
		}		
		else if(token != NULL && strcmp("cat", token) == 0) {
			token = strtok(NULL, " ");
			// parametre almayan cat komutu cagirilir.
			if(token == NULL) {
				if(fork() == 0) {
					execvp(catPath, arg_list);	
					exit(0);
				}
				wait(NULL);
			}
			// < komutu ile cat programi calistirilir.
			else if(strcmp("<", token) == 0) {
				token = strtok(NULL, " ");
				if(token != NULL && isFile(token)) {
					
					if(fork() == 0) {
						readFile(token);
						return 0;
					}
					wait(NULL);
				}
			}

		}
		// wc komutunu yazamadigim icin sonsuz bir dongude 
		// input alarak taklit etmeye calistim.
		else if(strcmp("wc", token) == 0) {
			char temp2[1024];
			while(1)
				scanf("%s", temp2);
		}
		// Girilen anlamli yada anlamsiz tum komutlar ekrana bastirilir.
		else if(strcmp(token,"history") == 0) {
			int j = 0;
			while(totalCommand >= 0) {
				printf("%d %s\n", j+1, allCommands[j]);
				totalCommand--;
				j++;
			}
		}
		// cd komutu ile dosyalar arasinda gecisi saglar.
		else if(strcmp(token, "cd") == 0){
			if(token != NULL) {
				token = strtok(NULL, " ");
				if(token != NULL) {
					if(cd(command, token) == 0) {
						getcwd(curPath, 1024);
						changeDir(lsPath, command);
						changeDir(catPath, command);
					} 
				}
			}

		}
		else if(strcmp("pwd", command) == 0) {
			char curDirectory[1024];
			getcwd(curDirectory, 1024);
			printf("%s\n", curDirectory);
		}
		else if(strcmp("help", command) == 0) {
			help();
		}
		else if(strcmp("exit", command) == 0)
			return 0;
		else
			printf("-->Invalid command! Try Again.\n");
	}

	
}

void readFile(char* token) {
	FILE *input = fopen(token, "r");
	char temp[2];
	size_t bytes = fread(temp,  sizeof(char), 1, input);
	while(bytes > 0) {
		fwrite(temp, sizeof(char), 1, stdout);	
		bytes = fread(temp, sizeof(char), 1, input);
	}

	fclose(input);
}

int isFile(char* fileName) {
	int index = strlen(fileName); 
	char extension[5];
	if(index < 5 || fileName[index-4] != '.')
		return 0;
	else 
		strcpy(extension, &fileName[index-3]);

	if(strcmp(extension, "dat") == 0 ||
		strcmp(extension, "txt") == 0 ||
		strcmp(extension, "log") == 0)
		return 1;
	else 
		return 0;
}

void changeDir(char* lsPath, char* cdCommand) {
	char temp[1024];
	getcwd(temp, 80);
	sprintf(temp, "./%s%s", cdCommand, &lsPath[1]);
	strcpy(lsPath, temp);
}

int cd(char* cdCommand, char* token) {
	char path[80] = {};	// cd'den sonra girilen parametreyi tutar.
	strcpy(path, token);
	int found = 0; // eger verilen parametre bulunuyorsa isle yapilir.
	char * curDirectory = malloc(1024);
	getcwd(curDirectory, 1024); // gecerli directory alinir
	DIR *dirp = opendir(curDirectory); // bulunulan directory acilir.
	struct dirent *direntp; 
	direntp = readdir(dirp);
	while(direntp != NULL) {
		// bulunulan directory'deki tum dosyalar karsilastirilir.
		if(strcmp(direntp->d_name, path) == 0) {
			found = 1;
		}
		direntp = readdir(dirp);
	}
	// eger istenilen yol bulunmussa oraya girilir.
	if(found) {
		if(strcmp("..", path) == 0) {
			int index = strlen(curDirectory) - 1;
			while(curDirectory[index] != '/' && index >= 0)
				index--;
			strcpy(cdCommand, &curDirectory[index+1]);
		}
		else // eger ust directory'e gidilecekse .. ekleneir.
			strcpy(cdCommand, "..");
		// alt directory icin dosya ismi eklenir.
		strcat(curDirectory, "/");
		strcat(curDirectory, path);
		chdir(curDirectory);
	}
	else {
		printf("Wrong directory name.\n");
	}

	free(curDirectory);
	closedir(dirp);
	if(found)
		return 0;
	else
		return -1;
}

void help() {
	printf("--ls; which will list file name of all files in the present working directory.\n");
	printf("--pwd: which will print the present working directory.\n");
	printf("--cd: which will change the present working directory to the location provided as argument.\n");
	printf("--help: which will print the list of supported commands.\n");
	printf("--wc: which will print on standard output the number of lines in the file provided to it as argument.\n");
	printf("--exit: which will exit the shell.\n");
	printf("--history: which will show history of commands.\n");
}
