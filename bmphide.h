/*
 * @author David Durman, 2009
 * @file bmphide.h
 */ 

#ifndef __BMPHIDE_H__
#define __BMPHIDE_H__

#include <sys/types.h>
#include <stdio.h>

#define BMPHIDEOk 0
#define BMPHIDEFail -1

#define MAX_TEXT_SIZE 1000000
#define VERBOSE 0
#define MAX_BMP_DATA 2048 * 1024 * 3


/* Record of the conversion */
typedef struct{
	/* BMP size */
	int64_t bmpSize;
	/* GIF size */
	int64_t gifSize;
} tBMPHIDE;


/**
 * @param record Record of the conversion.
 * @param t text to hide
 * @param inputFile BMP
 * @param outputFile BMP with hidden text
 * @return  0 if OK, -1 if error
 */
int bmphide(tBMPHIDE *record, unsigned char* t, FILE* inputFile, FILE *outputFile);


#endif

