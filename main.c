/**
 * @author David Durman, 2009
 * @file main.c
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "bmphide.h"

int main(int argc, char **argv)
{
  char *ifile = NULL;	// input picture
  char *ofile = NULL;	// output altered picture
  char *lfile = NULL;	// log file
  char *tfile = NULL;	// text to hide file
  char* txt = NULL;
  int lFlag = 0;
  int hFlag = 0;
  int c;

  FILE* inputFile = NULL;
  FILE* outputFile = NULL;
  FILE* logFile = NULL;
  FILE* textFile = NULL;

  opterr = 0;

  while ((c = getopt(argc, argv, "i:o:s:l:cxh")) != -1){
    switch (c){
    case 'i':
      ifile = optarg;
      break;
    case 'o':
      ofile = optarg;
      break;
    case 'l':
      lfile = optarg;
      lFlag = 1;
      break;
    case 's':	// secret file
      tfile = optarg;
      break;
    case 'h':
      hFlag = 1;
      break;
    case 't':
      txt = optarg;
      break;
    case '?':
    default:
      return BMPHideError("unknown option");
      break;
    }
  }

  if (hFlag == 1){
    printf("USAGE: bmphide -h | [-i input_file] [-o output_file] [-l log_file] [-s secret_file] \n");
    return BMPHIDEOk;
  }

  // input file
  if (ifile == NULL)
    return BMPHideError("An input file expected.");
  else
    inputFile = fopen(ifile, "rb");
  if (inputFile == NULL)
    return BMPHideError("can not find an input file");

  // output file
  if (ofile == NULL)
    outputFile = stdout;
  else
    outputFile = fopen(ofile, "wb");
  if (outputFile == NULL)
    return BMPHideError("can not open an output file");


  // text file
  if (tfile == NULL)
    textFile = stdin;
  else
    textFile = fopen(tfile, "r");
  if (textFile == NULL)
    return BMPHideError("can not open a text file");


  // log file
  if (lFlag == 1){
    if (lfile == NULL)
      return BMPHideError("can not open a log file");
    else
      logFile = fopen(lfile, "w");
  }

  size_t len = 0;
  if (tfile != NULL){
    fseek(textFile, 0, SEEK_END);
    len = ftell(textFile);
    fseek(textFile, 0, SEEK_SET);
    txt = (char *) malloc(len);
    len = fread(txt, len, 1, textFile);
  }

  if (txt != NULL)
    bmphide(NULL, txt, inputFile, outputFile);
  else
    bmpshow(NULL, inputFile);

  // close files
  if (inputFile != NULL) fclose(inputFile);
  if (outputFile != NULL) fclose(outputFile);
  if (textFile != NULL) fclose(textFile);

  if (txt)
    free(txt);

  exit(0);
}


	
