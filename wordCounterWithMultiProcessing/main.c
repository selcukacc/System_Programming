/************************************/
/*  CSE-244 HOMEWORK-II             */
/*  Author: Islam Goktan SELCUK     */
/*  Student No: 141044071           */
/*  Date: 09.03.2017                */
/*  Usage: ./listdir string dirname */
/************************************/

/*---------------------------------*/
/*            Includes             */
/*---------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CHAR 255

/*---------------------------------*/
/*      Function Prototypes        */
/*---------------------------------*/

/* Bakilan dosyanin txt turunde olup olmadigini kontol eder. */
int isTxt(char *txtName);
/* Bakilan dosyanin dosyanin directory olup olmadigini kontrol eder. */
int isDirectory(char *path);
/* Child process'ler icin bekleme fonksiyonu */
pid_t r_wait(int *stat_loc);
/* Kaydin tutuldugu log dosyasini okuyarak kac kelime bulundugunu hesaplar */
int readLog(void);

/*--- BIRINCI ODEVIN FONKSIYONLARI ---*/

/* Aranan string'in ilk harfini bulup, string'in kalanini baska bir */
/* fonksiyon ile kontrol eder. */
int findingWord(FILE *inp, FILE *outp, char *fileName, char *str); 

/* Aranan string'in ilk harfinden baslayarak kalan karakterlerinin dosyada */
/* olup olmadigini kontrol eder. Eger string'i bulursa konumunu print eder. */
/* Dosyadan okudugu son karakteri de return eder. */
char searchOneWord(FILE *inp, FILE *outp, char *fileName, char *str, char ch, 
					int *status, int *found, int *row, int *column);

/* Dosyadan okunan karakterler bosluk vb ise o karakterleri atlar ve okuugu */
/* son karakteri return eder. */
char jumper(FILE *inp, int *status, char ch, int *row, int *column);

/* Dosyadan okunan her karakterden sonra bu fonksiyon kullanilarak bulunulan */
/* konum yenilenir. */
void positionCounter(char ch, int *row, int *column, int status);


int main(int argc, char *argv[]) {
	DIR *dirp;                    
	struct dirent *direntp;	
	FILE *inp;	/* kelimelerin arandigi dosyalar icin */
	FILE *outp; /* log  dosyasi icin */
	pid_t child;/* fork fonksiyonundan sonra child process'i ayirabilmek icin*/
	/* en son islemleri tek processle yürütmek icin */
	pid_t mainProcess = getpid();
	char str[MAX_CHAR];	
	char newAddress[MAX_CHAR];
	int wordCount; /* toplam kelime sayisi */

	
	if(argc != 3) {
		fprintf(stderr, "Wrong parameters. Usage: ./listdir string dirname\n");
		return 1;
	}
	
	outp = fopen("log.txt", "w");	
/* gecerli directory str dizisine aktarilir ve istenilen directory */
/* adresi olusturulur. */
	getcwd(str, MAX_CHAR);
	strcat(str, "/");
	strcat(str, argv[2]);
	
	if((dirp = opendir(argv[2])) == NULL) {
		perror("Failed to open directory.");
		return 1;
	}
				
	direntp = readdir(dirp);	
	while(direntp != NULL) {
/* eğer directory yoksa sonraki dosya için adres eski haline getirilir. */
		strcpy(newAddress, str); 
		strcat(newAddress, "/");
		strcat(newAddress, direntp->d_name);	
		/* Eger bir directory bulunursa child olusturulur */
		if( isDirectory(newAddress) ) {
			if( (child = fork()) == -1) {
				perror("Failed to fork.\n");
				return 1;
			}
			/*islemin child ile devam edebilmesi icin*/
			if(child == 0) {
				if( (dirp = opendir(newAddress)) == NULL ) {
					perror("Failed to open directory.");
					return 1;
				}
			/* Eger directory bulunduysa yeni arama yeri o directory olur.*/
				strcpy(str, newAddress); 
			}	
			/* child'i bekler */
			while(r_wait(NULL) > 0);		
		}
		/* txt dosyasi icin ayrica child olusturulur */
		else if(isTxt(direntp->d_name)) {
			if( (child = fork()) == -1) {
				perror("Failed to fork.\n");
				return 1;
			}
			if(child == 0) {
				/* txt dosyasi acilir ve kelimeler aranir */
				inp = fopen(newAddress, "r");
				if(inp != NULL)
					findingWord(inp, outp, direntp->d_name, argv[1]);				
				
				fclose(inp);
				fclose(outp);
				/* txt den sorumlu child icin directory kapatilir */
				while((closedir(dirp) == -1) && EINTR == errno) ;
				/* child arama islemi bitirildiginde sonlandirilir */
				return 0;
			}
		}
		while(r_wait(NULL) > 0);
		direntp = readdir(dirp);		
	}
	while(r_wait(NULL) > 0);
	
	while((closedir(dirp) == -1) && EINTR == errno) ;
	/* log dosyasindan faydalanilarak kelime sayisi hesaplanir */
	if( ( child = getpid() ) == mainProcess ) {
		wordCount = readLog();
		fprintf(outp, "\n\n%d %s were found in total.\n", wordCount, argv[1]);
	}
	fclose(outp);	
	return 0;
}
/* Kaydin tutuldugu log dosyasini okuyarak kac kelime bulundugunu hesaplar */
int readLog(void) {
	FILE *inp;
	int status;
	char ch;
	int i;
	
	inp = fopen("log.txt", "r");
	i = 0;
	status = fscanf(inp, "%c", &ch);
	while(status != EOF) {
		if(ch == '\n')
			i++;	
	status = fscanf(inp, "%c", &ch);
	}
	
	return (i/2);
}

/* Bakilan dosyanin txt turunde olup olmadigini kontol eder. */
int isTxt(char *txtName) {
	int len = strlen(txtName);
	
	if(len <= 3)
		return 0;

	if(strncmp("txt", &txtName[len - 3], 3) == 0)
		return 1;
	else
		return 0;
}

/* Bakilan dosyanin dosyanin directory olup olmadigini kontrol eder. */	
int isDirectory(char *path) {
	struct stat statbuf;
	
	int len = strlen(path);
	
	if( stat(path, &statbuf) == -1)
		return 0;
	else if(strcmp(&path[len-2], "..") == 0 || strcmp(&path[len-1], ".") == 0)
		return 0;
	else
		return S_ISDIR(statbuf.st_mode); 
}

pid_t r_wait(int *stat_loc) {

	int retval;
	
	while ( ( (retval = wait(stat_loc)) == -1 ) && errno == EINTR) ;
	
	return retval;
}


/*-----------------------------------------------------------------*/
 /*               (Birinci Odevin Uygulamalari)                   */
  /*-------------------------------------------------------------*/

/* Aranan string'in ilk harfini bulup, string'in kalanini baska bir */
/* fonksiyon ile kontrol eder. Aranan string'in bulunma sayisini return eder.*/
int findingWord(FILE *inp, FILE *outp, char *fileName, char *str) 
{
	int status;    /* dosyanin sonunu kontrol eder */
	char ch;       /* txt dosyasindan okunan karakter */
	char ch2;      /* aranan kelimenin son harfini tutar */
	int row = 1;   /* fscanf fonksiyonunun bulundugu satiri tutar */
	int column = 1;/* sutunu tutar */
	int found = 0; /* aranan string'in bulunma sayisi */
	

	status = fscanf(inp, "%c", &ch);
	if(ch == '\n')
		positionCounter(ch, &row, &column, status);
	/* dosyanin sonuna kadar arama yap */
	while(status != EOF) {
		/* eger string'in ilk karakteri bulunduysa, diger karakterlerini ara */
		while(ch == str[0] && status != EOF) {
		/* aranan string bulunursa, son karakteri return edilir, bulunamazsa */
		/* fonksiyon aramayi birakir ve kaldigi karakteri return eder */
			ch2 = searchOneWord(inp, outp, fileName, 
								str, ch, &status, &found, &row, &column);							
			/* eger aranan metinde fonksiyonun gonderdigi karakter string'in */
			/* ilk karakteriyse dongu devam eder */
			if(ch2 == str[0]) {
				ch = ch2;
			}
		}
		
		status = fscanf(inp, "%c", &ch);
		/* her fscanf fonksiyonundan sonra bulunulan konum yenilenir */
		positionCounter(ch, &row, &column, status);
	}
	
	return (found);
}

/* Aranan string'in ilk harfinden baslayarak kalan karakterlerinin dosyada */
/* olup olmadigini kontrol eder. Eger string'i bulursa konumunu print eder. */
/* Dosyadan okudugu son karakteri de return eder. */
char searchOneWord(FILE *inp,   /* input dosyasi */
                   FILE *outp,  /* log dosyasi */
                   char *fileName, /* arama yapilan dosyanin ismi*/
                   char *str,   /* aranan string */
                   char ch,     /* dosyadan okunan karakter */
                   int *status, /* dosyanin sonunu kontrol eder */
                   int *found,  /* aranan kelimenin sayisini tutar */
                   int *row,    /* bulunulan satiri tutar */
                   int *column) /* bulunulan sutunu tutar */
{

	int i;   /* string'in karakterleri icin sayac */
	int len; /* string'in uzunlugu */
	int firstRow;   /* string ilk harfinin bulundugu satir */
	int firstColumn;/* ilk harfin bulundugu sutun */
	
	len = strlen(str);
/* Dosyada ilk harfin bulundugu konum degiskenlere atanir. Eger kelimenin */
/* tamami bulunursa atanan degerler print edilir. */
	firstRow = *row;
	firstColumn = *column;	
	/* string'in ilk harfinden itibaren diger harfleri de kontrol edilir */
	/* bulunan her harf icin i degiskeni bir artirilir */	
	for(i = 0; str[i] == ch && i < len-1 && *status != EOF; i++) {
		*status = fscanf(inp, "%c", &ch);		
		positionCounter(ch, row, column, *status);
		/* eger dosyada bosluk vb karakter varsa atlanir */
		if(ch == '\n' || ch == ' ' || ch == '\t') {
			ch = jumper(inp, status, ch, row, column);
		}
	}
/* son karaktere ve i'nin buyuklugune bakilarak string bulunup bulunmadigi */
/* kontrol edilir */
	if(str[len-1] == ch && i == len-1) {
		fprintf(outp, "%s: [%d, %d] %s first character is found.\n\n", 
				fileName, firstRow, firstColumn, str);
		*found = *found + 1;	
	}
	/* dosyada kalinan son karakter return edilir */	
	return (ch);
}

/* Dosyadan okunan karakterler bosluk vb ise o karakterleri atlar ve okuugu */
/* son karakteri return eder. */
char jumper(FILE *inp,   
            int *status, 
            char ch,     
            int *row,    
            int *column) 
{

	while( (ch == '\n' || ch == ' ' || ch == '\t') && *status != EOF ) {
		*status = fscanf(inp, "%c", &ch);
		/* atlanan her karakterden sonra dosyada bulunulan konum yenilenir */
		positionCounter(ch, row, column, *status);
	}
	/* kalinan son karakter return edilir */
	return (ch);
}

/* Dosyadan okunan her karakterden sonra bu fonksiyon kullanilarak bulunulan */
/* konum yenilenir. */
void positionCounter(char ch,     /* dosyadan okunan son karakter */ 
                     int *row,    
                     int *column, 
                     int status)  
{
	if(status != EOF) {
	/* Eger satir atlanmissa sutun sifira esitlenir */
		if(ch == '\n') {
			*row = *row + 1;
			*column = 0;
		}	
		else 
			*column = *column + 1;
	}
}

