#ifndef TGAUTILS_H_
#define TGAUTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>

#define true    1
#define false   0 

typedef struct {
	unsigned char* imageData;
	unsigned int bpp;
	unsigned int width;
	unsigned int height;
	bool RLE_flag;
	unsigned int imageSize;
	unsigned int color_num;
} TGA_IMG;


void LoadTGA(char *filename, TGA_IMG *texture);
void LoadUncompressedTGA(char *filename, FILE *fTGA, TGA_IMG *texture);
void LoadCompressedTGA(char *filename, FILE *fTGA, TGA_IMG *texture);
void SET_TGA_INFOMATION(TGA_IMG *texture, FILE *fTGA);

#endif /*TGAUTILS_H_*/
