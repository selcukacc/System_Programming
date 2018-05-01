/***********************************/
/*  CSE-244 HOMEWORK-I             */
/*  Author: Islam Goktan SELCUK    */
/*  Student No: 141044071          */
/*  Date: 01.03.2017               */
/*  Usage: ./list string filename  */
/***********************************/

/*---------------------------------*/
/*            Includes             */
/*---------------------------------*/
#include <stdio.h>
#include <string.h>

/*---------------------------------*/
/*      Function Prototypes        */
/*---------------------------------*/

/* Aranan string'in ilk harfini bulup, string'in kalanini baska bir */
/* fonksiyon ile kontrol eder. */
void findingWord(FILE *inp, char *str); 

/* Aranan string'in ilk harfinden baslayarak kalan karakterlerinin dosyada */
/* olup olmadigini kontrol eder. Eger string'i bulursa konumunu print eder. */
/* Dosyadan okudugu son karakteri de return eder. */
char searchOneWord(FILE *inp, char *str, char ch, int *status, int *found,
						int *row, int *column);

/* Dosyadan okunan karakterler bosluk vb ise o karakterleri atlar ve okuugu */
/* son karakteri return eder. */
char jumper(FILE *inp, int *status, char ch, int *row, int *column);

/* Dosyadan okunan her karakterden sonra bu fonksiyon kullanilarak bulunulan */
/* konum yenilenir. */
void positionCounter(char ch, int *row, int *column, int status);

/*---------------------------------*/
/*         Main Function           */
/*---------------------------------*/
int main(int argc, char *argv[]) 
{

	FILE *inp;	
	/* Girilen arguman sayisinin kontrolu */
	/* Eger hata varsa, -1 return edilir. */
	if(argc != 3) {
		printf("Wrong parameters. Usage: list string filename\n");
		return (-1);
	}
	/* dosyanin acilmasi ve dosyanin kontrol edilmesi */
	inp = fopen(argv[2], "r");	
	if(inp == NULL) {
		printf("Cannot open file %s for input.\n", argv[2]);
		return (-1);
	}	
	/* Arama islemini yapan fonksiyon */	
	findingWord(inp, argv[1]);	
	/* dosya kapatilir*/
	fclose(inp);
	
	return (0);
}

/*---------------------------------*/
/*     Function Implementations    */
/*---------------------------------*/

/* Aranan string'in ilk harfini bulup, string'in kalanini baska bir */
/* fonksiyon ile kontrol eder. */
void findingWord(FILE *inp, 
                 char *str) 
{
	int status;    /* dosyanin sonunu kontrol eder */
	char ch;       /* txt dosyasindan okunan karakter */
	char ch2;      /* aranan kelimenin son harfini tutar */
	int row = 1;   /* fscanf fonksiyonunun bulundugu satiri tutar */
	int column = 1;/* sutunu tutar */
	int found = 0; /* aranan string'in bulunma sayisi */

	status = fscanf(inp, "%c", &ch);
	/* dosyanin sonuna kadar arama yap */
	while(status != EOF) {
		/* eger string'in ilk karakteri bulunduysa, diger karakterlerini ara */
		while(ch == str[0] && status != EOF) {
		/* aranan string bulunursa, son karakteri return edilir, bulunamazsa */
		/* fonksiyon aramayi birakir ve kaldigi karakteri return eder */
			ch2 = searchOneWord(inp, str, ch, &status, &found, &row, &column);							
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
	printf("%d adet %s bulundu.\n", found, str);	
}

/* Aranan string'in ilk harfinden baslayarak kalan karakterlerinin dosyada */
/* olup olmadigini kontrol eder. Eger string'i bulursa konumunu print eder. */
/* Dosyadan okudugu son karakteri de return eder. */
char searchOneWord(FILE *inp,   /* input dosyasi */
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
		printf("[%d, %d] konumunda ilk karakter bulundu.\n", 
				firstRow, firstColumn);
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
