/*
 * @file bmphide.c
 * @autor David Durman 2009
 */ 

#include <sys/types.h>
#include <stdarg.h>	//va_list, va_start, va_end
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "bmphide.h"

#define NULLBYTE 0x0

int NLSBS;	// number of LSBs used to hide a text


/**
 * Print error to stderr end exit.
 */
int BMPHideError(const char *fmt, ...){
  va_list args;
  fprintf(stderr, "BMPHide ERROR: ");

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");	
  return BMPHIDEFail;
}


/*--------------------------------------------------.
  |  Functions and types for BMP manipulation        |
  `--------------------------------------------------*/

// RGB
typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} t_RGB;


// BMP Header
typedef struct {
  u_int16_t          bfType;           /* BM - file type - 2byte*/
  u_int32_t          bfSize;           /* file size - 4byte*/
  u_int16_t          bfReserved1;      /* reserved */
  u_int16_t          bfReserved2;      /* reserved */
  u_int32_t          bfOffBits;        /* image data offset */
} BMPFileHeader;

// BMP format
typedef struct {
  u_int32_t          biSize;           /* info header size */
  u_int32_t          biWidth;          /* image width */
  u_int32_t          biHeight;         /* image height */
  u_int16_t          biPlanes;         /* planes */
  u_int16_t          biBitCount;       /* bits per pixel */
  u_int32_t          biCompression;    /* compression */
  u_int32_t          biSizeImage;      /* image data size */
  u_int32_t          biXPelsPerMeter;  /* X pixels per meter */
  u_int32_t          biYPelsPerMeter;  /* Y pixels per meter */
  u_int32_t          biClrUsed;        /* number of colors used */
  u_int32_t          biClrImportant;   /* number of important colors */
} BMPInfoHeader;

typedef struct {
  int size;
  int index;
  int bitIndex;
  unsigned char* data;
} tBMPHIDEText;


BMPFileHeader BMPHeader;
BMPInfoHeader BMPInfo;

unsigned char BMPData[MAX_BMP_DATA];
u_int64_t BMPDataLength = 0;


void BMPHidePrintInfo(){
  if (VERBOSE){
    printf("INFO:\n");
    printf("\tbfSize = %d\n", BMPHeader.bfSize);
    printf("\tbiWidth: %d\n\tbiHeight: %d\n", BMPInfo.biWidth, BMPInfo.biHeight);
  }
}

/**
 * Write data to BMP.
 */
u_int64_t BMPhideSaveBMP(FILE* file){
  u_int64_t cbytes = 0;	// number of writen bytes

  fwrite(&(BMPHeader.bfType), 2, 1, file);
  fwrite(&(BMPHeader.bfSize), 4, 1, file);
  fwrite(&(BMPHeader.bfReserved1), 2, 1, file);
  fwrite(&(BMPHeader.bfReserved2), 2, 1, file);
  fwrite(&(BMPHeader.bfOffBits), 4, 1, file);
  cbytes += 14;

  fwrite(&BMPInfo, sizeof(BMPInfo), 1, file);
  cbytes += sizeof(BMPInfo);

  int i = 0;
  for (; i < BMPDataLength; i++){
    fputc(BMPData[i], file);
  }

  return cbytes;
}

u_int64_t BMPHideReadBMP(FILE* inputFile){
  u_int64_t cbytes = 0;	// number of read bytes

  if (fread(&BMPHeader.bfType, 2, 1, inputFile) != 1)
    return BMPHideError("Can not read BMP type info.");
  if (fread(&BMPHeader.bfSize, 4, 1, inputFile) != 1)
    return BMPHideError("Can not read BMP size info.");
  if (fread(&BMPHeader.bfReserved1, 2, 1, inputFile) != 1)
    return BMPHideError("Can not read BMP reserved1 info.");
  if (fread(&BMPHeader.bfReserved2, 2, 1, inputFile) != 1)
    return BMPHideError("Can not read BMP reserved2 info.");
  if (fread(&BMPHeader.bfOffBits, 4, 1, inputFile) != 1)
    return BMPHideError("Can not read BMP off bits info.");

  if (fread(&BMPInfo, 40, 1, inputFile) != 1)
    return BMPHideError("Can not read BMP info.");

  BMPHidePrintInfo();

  int c;
  while ((c = fgetc(inputFile)) != EOF){
    BMPData[BMPDataLength++] = (unsigned char) c;
    cbytes++;
  }
  return cbytes;
}

int BMPHideGetLSB(unsigned char c){
  return c & 0x1;
}

int BMPHideGetNthLSB(unsigned char c, int nth){
  return (c & (1 << nth)) >> nth;
}

int BMPHideGetTextBit(tBMPHIDEText* text){
  if (text->bitIndex < 0){
    text->index++;
    text->bitIndex = 7;
  }
  return (text->data[text->index] >> text->bitIndex--) & 0x1;  
}

// returns set byte or -1 if not set
int BMPHideSetTextBit(tBMPHIDEText* text, int bit){
  int ret = -1;
  if (text->bitIndex < 0){
    ret = text->data[text->index];
    text->index++;
    text->bitIndex = 7;
  }
  if (bit)
    text->data[text->index] |= (0x1 << text->bitIndex--);
  else
    text->data[text->index] &= ~(0x1 << text->bitIndex--);
  return ret;
}

void BMPHideHideText(tBMPHIDEText* text){
  int iBmp = 0;
  int iTxt = 0;
  int iNull = 0;
  int bit;
  int iLsb;
  int i;

  // first, save number of LSBs used (each into a separate byte)
  for (i = 0; i < 8; i++){
    bit = (NLSBS >> i) & 0x1;
    if (bit)
      BMPData[iBmp] |= (0x1);
    else
      BMPData[iBmp] &= ~(0x1);
    iBmp++;
  }

  do {
    for (iLsb = 0; iLsb < NLSBS; iLsb++){
      bit = BMPHideGetTextBit(text);
      if (bit)
	BMPData[iBmp] |= (0x1 << iLsb);
      else
	BMPData[iBmp] &= ~(0x1 << iLsb);
      iTxt++;
    }
    iBmp++;
  } while (iTxt < text->size * 8);

  // append null character
  iLsb = 0;
  while (1){
    for (iLsb = 0; iLsb < NLSBS; iLsb++){
      BMPData[iBmp] &= ~(0x1 << iLsb);
      iNull++;
      if (iNull == 8)
	goto endwhile;
    }
    iBmp++;
  } 
 endwhile:
  ;
}


/**
 * Convert binary string to decimal number.
 */
int BMPHideBin2Dec(char* str){
  int ret = 0;
  int strl = strlen(str);
  int i = strl - 1;
  for (; i >= 0; i--){
    ret += (str[i] - '0') * ( 1 << (strl - 1 - i) );
  }
  return ret;
}


int bmphide(tBMPHIDE* record, unsigned char* t, FILE* inputFile, FILE* outputFile){
  tBMPHIDEText* text = (tBMPHIDEText*) malloc(sizeof(tBMPHIDEText));
  if (text == NULL)
    BMPHideError("Not enough memory.");
  text->size = strlen(t);
  text->index = 0;
  text->bitIndex = 7;
  text->data = t;

  BMPHideReadBMP(inputFile);

  int pictureBytes = BMPInfo.biWidth * BMPInfo.biHeight * 3;
  int textBytes = text->size;
  int textBits = textBytes * 8;
  NLSBS = (textBits / pictureBytes) + 1;
  
  if (VERBOSE)
    printf("LSBS used: %d\n", NLSBS);
  if (NLSBS > 7){
    return BMPHideError("Text is too long.");
  }

  

  BMPHideHideText(text);
  BMPhideSaveBMP(outputFile);

  if (VERBOSE){
    printf("TEXT: %s\n", text->data);
    printf("TEXT IN BINARY FORMAT: \n");
    text->index = 0;
    text->bitIndex = 7;
    int i;
    for (i = 0; i < text->size * 8; i++)
      printf("%d", BMPHideGetTextBit(text));
    printf("\n");
  }

  free(text);
  return BMPHIDEOk;
}

tBMPHIDEText* bmpshow(tBMPHIDE* record, FILE* inputFile){
  tBMPHIDEText* text = (tBMPHIDEText*) malloc(sizeof(tBMPHIDEText));
  if (text == NULL){
    BMPHideError("Not enough memory.");
    exit(1);
  }
  text->size = 0;
  text->index = 0;
  text->bitIndex = 7;
  text->data = (unsigned char*) malloc(sizeof(MAX_TEXT_SIZE));
  if (text->data == NULL)
    BMPHideError("Not enough memory.");

  BMPHideReadBMP(inputFile);
  
  int iBmp = 0;
  int bit;
  int byte;
  int i;

  // first, get number of LSBs used
  for (i = 0; i < 8; i++){
    bit = BMPData[iBmp++] & 0x1;
    if (bit)
      NLSBS |= (0x1 << i);
    else
      NLSBS &= ~(0x1 << i);
  }
  if (VERBOSE)
    printf("NLSBS: %d\n", NLSBS);

  for (; iBmp < BMPDataLength; iBmp++){
    if (VERBOSE)
      if (iBmp % 8 == 0) 
	printf(" ");
    for (i = 0; i < NLSBS; i++){
      bit = BMPHideGetNthLSB(BMPData[iBmp], i);
      if (VERBOSE)
	printf("%d", bit);      
      byte = BMPHideSetTextBit(text, bit);
      if (byte == NULLBYTE)
	goto endfor;
    }
  }
 endfor:
  text->data[text->index] = '\0';

  // vypisy
  if (VERBOSE){
    printf("\nTEXT\n\tlength: %d\n\tdata: %s\n", strlen(text->data), text->data);
    printf("%d\n", 0 << 1);
  }
  printf("%s", text->data);

  //  free(text->data);
  free(text);
  return NULL;
}

