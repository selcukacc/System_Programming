/***********************************/
/*  CSE-344 HOMEWORK-I             */
/*  Author: Islam Goktan SELCUK    */
/*  Student No: 141044071          */
/*  Usage: ./exe filename          */
/***********************************/

#include "stdio.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// tiff dosyasinin bilgilerini tutmak icin kullandim.
struct imageInfo {
	int width;	
	int height;
	int stripOffset; // pixellerin okunmaya baslandigi konum
	int byteOrder;	// byte dizilimi Motorola-Intel
	int pixelNumber; // pixel derinligi
};

// buffer ile okunan hex degerleri decimal a donusturur. (4 byte icin)
int readLongFromBuffer(char *buffer, int byteOrder, int first);
// buffer ile okunan hex degerleri decimal a donusturur. (2 byte icin)
int readFromBuffer(char *buffer, int byteOrder, int first);
// tiff dosyasinin bilgilerini okur ve struct'a kaydeder.
void readImageInfo(char* fileName, struct imageInfo* imageinfo);
// tiff dosyasindaki pixelleri satir satir okur
void readImageFile(char* fileName, struct imageInfo imageinfo);
// imageInfo struct'ına kaydedilen tiff bilgilerini ekrana basar.
void showImageInfo(struct imageInfo imageinfo);
// resim dosyasinin tek satirini okuyup, print eder.
int readLine(FILE *f, struct imageInfo imageInfo);

void hexToBin(char hexa, char* result);

int main(int argc, char *argv[]) {

	if(argc != 2) {
		printf("Wrong parameters. Usage: exe filename\n");
		return (-1);
	}

	struct imageInfo sample;
	readImageInfo(argv[1], &sample);
	readImageFile(argv[1], sample);
	return 0;
}

int readLine(FILE *f, struct imageInfo imageInfo) {
	char *buffer;
	char hex[8];
	char binary[20];
	char temp[20];
	char temp2[20];
	int i, j, bytes;

	// Resmin genisligine bakarak kac byte okunacagina karar verilir.
	int totalByte = imageInfo.width / 8 + (imageInfo.width % 8);
	// Eger son byte'in tamami genislige dahil degilme, dahil olan kismi hesasplanir.
	int lastByteToRead = (imageInfo.width % 8);

	buffer = (char *)malloc(imageInfo.width * sizeof(char));
	
	// her satir icin totalByte kadar byte okunur
	bytes = fread(buffer, 1, totalByte, f);
	buffer[imageInfo.width - 1] = '\0';

	for(i = 0; i < totalByte; i++) {
		// her byte once hex formatinda string'e kaydedilir.
		sprintf(hex, "%.8x\0", buffer[i]);
		// string'deki son iki hex karakteri binary'e donusturulur.
		hexToBin(hex[6], temp);
		hexToBin(hex[7], temp2);
		// ikisi birlestirilir.
		sprintf(binary, "%s%s\0", temp, temp2);
		// eger resim genisliginden fazla karakter alinmissa string sonu belirtilir. 
		if(i == totalByte - 1)
			binary[lastByteToRead] = '\0';
		// binary deger ekrana yazdirilir.
   		printf("%s", binary);
	}
	
	printf("\n");
	free(buffer);
	return 0;
}

void readImageFile(char* fileName, struct imageInfo imageInfo) {
  
   char  buffer[100];

   FILE  *f;
   int i;

   f = fopen(fileName, "rb");
   // dosya kontrolu yapar
	if(f != NULL){
		// pixellerin oldugu konuma gider
   		fseek(f, imageInfo.stripOffset, SEEK_SET);
   		// resim doyasının tum satirlarini okur
		for(i=0; i<imageInfo.height; i++)
			readLine(f, imageInfo);

		fclose(f);	
   }
   else
      printf("File could not be opened.\n");
}

void readImageInfo(char* fileName, struct imageInfo* myImage) {
	char buffer[80];
	FILE *f;
	int bytes, i;
	int numberOfEntries;
	int fieldType = 0;

	f = fopen(fileName, "rb");
	
	if(f != NULL) {
		// Dosyaya ait temel bilgileri okur
		bytes = fread(buffer, 1, 8, f);
		// Tiff turunu okuyarak struct'a kaydeder
		if(buffer[0] == 0x49)
			myImage->byteOrder = 1;
		else
			myImage->byteOrder = 0;
		// tiff bilgilerini tutan offset'i okur. (bit derinligi, width, length gibi)
		myImage->stripOffset = readLongFromBuffer(buffer, myImage->byteOrder, 4);

		// yukarida okunan offset'e gider
		fseek(f, myImage->stripOffset, SEEK_SET);
		// toplam okunacak entry sayini okur.
		bytes = fread(buffer, 1, 2, f);
		numberOfEntries = readFromBuffer(buffer, myImage->byteOrder, 0);

		// tiff dosyasina ait bilgiler struct'a kaydedilir.
		for(i = 0; i < numberOfEntries; i++) {
			bytes = fread(buffer, 1, 12, f);
			// tag tipini okuyarak bilgileri ceker
			int tag = readFromBuffer(buffer, myImage->byteOrder, 0);
			//findTagType(buffer, myImage->byteOrder);
			
			//resim genisligi okunur
			if(tag == 256) {
				// degeri tutan hex'in buyuklugune bakilir
				fieldType = readFromBuffer(buffer, myImage->byteOrder, 2);
				// kücük ise readFromBuffer fonksiyonu kullanılır
				if(fieldType == 3) 
					myImage->width = readFromBuffer(buffer, myImage->byteOrder, 8);
				// buyukse readLongFromBuffer kullanılır.
				else
					myImage->width  = readLongFromBuffer(buffer, myImage->byteOrder, 8);
			} 
			// resim uzunlugunu okur
			else if(tag == 257) {
				fieldType = readFromBuffer(buffer, myImage->byteOrder, 2);
				if(fieldType == 3)
					myImage->height = readFromBuffer(buffer, myImage->byteOrder, 8);
				else
					myImage->height  = readLongFromBuffer(buffer, myImage->byteOrder, 8);
				//printf("height: %d\n", myImage->height);
			}
			// piksel derinligini okur
			else if(tag == 258) {
				fieldType = readFromBuffer(buffer, myImage->byteOrder, 2);
				if(fieldType == 3)
					myImage->pixelNumber = readFromBuffer(buffer, myImage->byteOrder, 8);
				else
					myImage->pixelNumber = readLongFromBuffer(buffer, myImage->byteOrder, 8);
				//printf("bps: %d\n", myImage->pixelNumber);
			}
			// pixellerin bulundugu offset'i okur
			else if(tag == 273) {
				fieldType = readFromBuffer(buffer, myImage->byteOrder, 2);
				if(fieldType == 3) 
					myImage->stripOffset = readFromBuffer(buffer, myImage->byteOrder, 8);
					
				else 
					myImage->stripOffset = readLongFromBuffer(buffer, myImage->byteOrder, 8);				
			}

		}

	}
	else
		printf("File could not be opened.\n");

	// tiff dosyasina ait bilgileri ekrana basar.
	showImageInfo(*myImage);
	fclose(f);
}

int readLongFromBuffer(char *buffer, int byteOrder, int first) {
	int value = -1;
	int i = 0;
	int j = 0;
	char temp[8];
	int len;

	if(byteOrder == 0) {
		// buffer'da tutulan hex degerlerini bir char array'e atar
		sprintf(temp, "%.2x%.2x%.2x%.2x", buffer[first],buffer[first + 1],
			buffer[first + 2],buffer[first + 3]);
	}
	else if(byteOrder == 1)
		sprintf(temp, "%.2x%.2x%.2x%.2x", buffer[first + 3],buffer[first + 2],
			buffer[first + 1],buffer[first]);

	// char array'e atilan hex degerini integer'a donusturur.
	sscanf(temp, "%x", &value);
	return value;
}

int readFromBuffer(char *buffer, int byteOrder, int first) {
	int value = -1;
	char temp[40];

	// buffer'da tutulan hex degerlerini bir char array'e atar
	if(byteOrder == 0) {
		sprintf(temp, "%.2x%.2x", buffer[first],buffer[first + 1]);
	}
	else if(byteOrder == 1)
		sprintf(temp, "%.2x%.2x", buffer[first+1],buffer[first]);
	// char array'e atilan hex degerini integer'a donusturur.
    sscanf(temp, "%x", &value);
	return value;
}

void showImageInfo(struct imageInfo imageinfo) {
	printf("Image width: %d\n", imageinfo.width);
	printf("Image height: %d\n", imageinfo.height);
	printf("Bits per pixel: %d\n", imageinfo.pixelNumber);
	printf("Strip Offset: %d\n", imageinfo.stripOffset);

}

void hexToBin(char hexa, char* result) {

    switch (hexa) {
        case '0':
            strcpy(result, "0000"); break;
        case '1':
            strcpy(result, "0001"); break;
        case '2':
            strcpy(result, "0010"); break;
        case '3':
            strcpy(result, "0011"); break;
        case '4':
            strcpy(result, "0100"); break;
        case '5':
            strcpy(result, "0101"); break;
        case '6':
            strcpy(result, "0110"); break;
        case '7':
            strcpy(result, "0111"); break;
        case '8':
            strcpy(result, "1000"); break;
        case '9':
            strcpy(result, "1001"); break;
        case 'A':
            strcpy(result, "1010"); break;
        case 'B':
            strcpy(result, "1011"); break;
        case 'C':
            strcpy(result, "1100"); break;
        case 'D':
            strcpy(result, "1101"); break;
        case 'E':
            strcpy(result, "1110"); break;
        case 'F':
            strcpy(result, "1111"); break;
        case 'a':
            strcpy(result, "1010"); break;
        case 'b':
            strcpy(result, "1011"); break;
        case 'c':
            strcpy(result, "1100"); break;
        case 'd':
            strcpy(result, "1101"); break;
        case 'e':
            strcpy(result, "1110"); break;
        case 'f':
            strcpy(result, "1111"); break;
        default:
        	strcpy(result, "err"); break;
    }
}