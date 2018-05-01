/***********************************/
/*  CSE-344 HOMEWORK-II            */
/*  Author: Islam Goktan SELCUK    */
/*  Student No: 141044071          */
/*  Usage: ./exe                   */
/***********************************/

/*
	Program olusturulan islemlerden biriyle x.dat dosyasına 
rastgele sayilar yazar. Oteki islemleyse bu yazılan sayilar 
aynı dosyadan okunur. Calisma sirasinda iki islem de busy waiting
yapmadan veri transferi saglanir.

	SIGINT(ctrl-c) sinyali ile program iki islemi duzgun sekilde
sonlandirarak kapanir.

*/
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>

// Programa kapanma sinyali gonderildiginde
//bu degiskenden yararlanilarak donguler bitirilir.
int keepWriting = 1;

pid_t child;

void stopWriting(int signo) {
	if(signo == SIGINT)
		keepWriting = 0;
}

// dosyanin satir sayisini dondurur.
int numberOfLines();
// olusturulan rastgele sayilari dosyaya yazar
void writeRandomNumber(FILE* file);
// dosyadan sayilari okur
void printDft(int* values);

int main(int argc, char** argv) {
	
	// dosya olusturulur
	FILE *file = fopen("x.dat", "w+");
	fclose(file);

	srand(time(NULL)); 
	// SIGINT icin signal handler'ın atanmasi
	struct sigaction sact;
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = 0;
	sact.sa_handler = &stopWriting;
	sigaction(SIGINT, &sact, NULL);

	char buffer[16] = {};
	int fileClosed = 0; // child'in dosyayi kapatip kapatmadigini
						// kontrol eder
	int fileClosedP = 0;// parent icin dosya kapali mi kontrolu

	// dosya kilitleme icin gereken struct initialize edilir
	struct flock lock;
	memset(&lock, 0, sizeof(lock));

	// bir tane islem olustururlur
	if( (child = fork()) == 0) {
		// SIGINT sinyali ile dongu sonlandirilmasi icin
		// keepWriting kosulu vardir.
		while(keepWriting) {

			FILE* file = fopen("x.dat", "r+");
			fileClosed = 0;
			// dosya yazilmak uzere mesgul degilse kilitlenir
			lock.l_type = F_WRLCK;
			if(fcntl(fileno(file), F_SETLK, &lock) != -1) {	
				// dosya sonuna gidilir ve sayilar yazilir			
				fseek(file, 0, SEEK_END);
				writeRandomNumber(file);
				// yazma islemi tamamlandiktan sonra kilit kaldirilir
				lock.l_type = F_UNLCK;
				if(fcntl(fileno(file), F_SETLK, &lock) != -1) {
	      			fclose(file);
	      			fileClosed = 1;
	      		}			
			}
			if(fileClosed == 0) {
				fclose(file);
				fileClosed = 1;
			}
		}
		exit(0);
	} else {
		while(keepWriting) {
			// dosyadan okuma icin dosya parent tarafindan acilir
			FILE* file = fopen("x.dat", "r+");
			fileClosedP = 0;
			// dosya kilitlenir
			lock.l_type = F_WRLCK;
			if(fcntl(fileno(file), F_SETLK, &lock) != -1) {			
				// son satiri silmek icin imlecin yeri degistirilir.
				fseek(file, 0, SEEK_END);
				fseek(file, -11, SEEK_CUR);
				char temp[16];
				int values[5];
				int j;
				// sayilar dosyadan alinir
				for(j = 0; j < 4; j++) {
					fscanf(file, "%s", temp);
					values[j] = atoi(temp);	
				}
				fscanf(file, "%s\n", temp);
				values[4] = atoi(temp);
				// sayilar ekrana yazilir
				printDft(values);
				// son satir dosyadan silinir
				fseek(file, -11, SEEK_CUR);
				ftruncate(fileno(file), ftell(file));
				// dosya kilidi kaldirilir.
				lock.l_type = F_UNLCK;
				if(fcntl(fileno(file), F_SETLK, &lock) != -1) {
	            	fclose(file);
	            	fileClosedP = 1;
	      		}
			}
			if(fileClosedP == 0) {
				fclose(file);
				fileClosedP = 1;
			}			
		}
		// parent bitirilmek icin child'i bekler
		wait(NULL);

		exit(0);
	}

	return 0;
}

int numberOfLines() {
	FILE *f = fopen("x.dat", "r");
	int i = 0;
	char buf[2] = {};
	int byte = 0;

	byte = fread(buf, 1, 1, f);
	while(byte > 0) { 
		byte = fread(buf, 1, 1, f);
		if(buf[0] == '\n')
			i++;
	}
	return i-1;
}

void writeRandomNumber(FILE* file) {
	int i;
	int r;
	printf("Process A: i’m producing a random sequence: ");
	for(i = 0; i < 4; i++) {
		r = rand() % 10;
		fprintf(file, "%d ", r);
		printf("%d ", r);
	}
	r = rand() % 10;
	printf("%d\n", r);
	fprintf(file, "%d \n", r);
}

void printDft(int* values) {
	int j;
	printf("Process B: Numbers carried from the process A: ( ");
	for(j = 0; j < 5; j++)
		printf("%d ", values[j]);
	printf(")\n");
	
}