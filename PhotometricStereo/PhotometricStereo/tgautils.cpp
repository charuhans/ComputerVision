#include "tgautils.h"


void LoadTGA(char *filename, TGA_IMG *texture) {

	FILE *fTGA;
	fTGA = fopen(filename, "rb");
	if (fTGA ==NULL) {
		printf("can't open file");
		exit(0);
	}


	unsigned char tgaheader[12];
	if (fread(&tgaheader, sizeof(tgaheader), 1, fTGA) == 0)
	{
		printf("can't read header");
		exit(0);
	}

	unsigned char uTGAcompare[12] = {0,0, 2,0,0,0,0,0,0,0,0,0};
	unsigned char cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};

	if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)
	{

	//	printf("this file is uncompressed");
		LoadUncompressedTGA(filename, fTGA,texture);

	}
	else if(memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)
	{

	//	printf("this file is compressed");
		LoadCompressedTGA(filename, fTGA,texture);

	}
	else
	{
	//	printf("wrong!");
	}

	fclose(fTGA);fTGA=NULL;
}

void LoadUncompressedTGA(char *filename, FILE *fTGA, TGA_IMG *texture) {

	texture->RLE_flag = false;

	SET_TGA_INFOMATION(texture, fTGA); 

	if (fread(texture->imageData, 1, texture->imageSize, fTGA)
			!= texture->imageSize) {
		printf("Error could not read image data");
		fclose(fTGA);
		fTGA = NULL;
		free(texture->imageData);
		texture->imageData =NULL;
		exit(0);
	}

}

void LoadCompressedTGA(char *filename, FILE *fTGA, TGA_IMG *texture) {

	texture->RLE_flag = true;

	SET_TGA_INFOMATION(texture, fTGA); 


	unsigned int pixelcount = texture->width * texture->height;
	unsigned int currentpixel = 0;
	unsigned int currentbyte = 0;
	unsigned char* colorbuffer = (unsigned char*)malloc(texture->color_num * sizeof(unsigned char));//new unsigned char[texture->color_num]; 


	do {
		/** header **/
		unsigned char chunkheader = 0;
		if (fread(&chunkheader, sizeof(unsigned char), 1, fTGA) == 0) {
			printf("wrong chunk");
			fclose(fTGA);
			fTGA = NULL;
			free(colorbuffer);
			colorbuffer = NULL;
			free(texture->imageData);
			texture->imageData =NULL;
			exit(0);
		}

		if (chunkheader < 128) 
		{

			short counter;
			chunkheader++;

			for (counter = 0; counter < chunkheader; counter++) //(chunkheader)
			{

				if (fread(colorbuffer, 1, texture->color_num, fTGA)
						!= texture->color_num) {
					printf("Error could not read image data");
					fclose(fTGA);
					fTGA = NULL;
					free(colorbuffer);
					colorbuffer = NULL;
					free(texture->imageData);
					texture->imageData =NULL;
					exit(0);
				}

				texture->imageData[currentbyte] = colorbuffer[2]; //red
				texture->imageData[currentbyte + 1] = colorbuffer[1]; //green
				texture->imageData[currentbyte + 2] = colorbuffer[0]; //blue
				if (texture->color_num == 4) {
					texture->imageData[currentbyte + 3] = colorbuffer[3]; //alpha
				}

				currentbyte += texture->color_num;
				currentpixel++;
				if (currentpixel > pixelcount) 
				{
					printf("Error too many pixels read");
					fclose(fTGA);
					fTGA = NULL;
					free(colorbuffer);
					colorbuffer = NULL;
					free(texture->imageData);
					texture->imageData =NULL;
					exit(0);
				}
			}

		} else 
		{

			short counter;
			chunkheader -= 127;
			if (fread(colorbuffer, 1, texture->color_num, fTGA)
					!= texture->color_num) {
				printf("Error could not read from file");
				fclose(fTGA);
				fTGA = NULL;
				free(colorbuffer);
				colorbuffer = NULL;
				free(texture->imageData);
				texture->imageData =NULL;
				exit(0);
			}

			for (counter = 0; counter < chunkheader; counter++) {
				texture->imageData[currentbyte ] = colorbuffer[2];
				texture->imageData[currentbyte + 1 ] = colorbuffer[1];
				texture->imageData[currentbyte + 2 ] = colorbuffer[0];
				if (texture->color_num == 4) {
					texture->imageData[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += texture->color_num;
				currentpixel++;
				if (currentpixel > pixelcount) {
					printf("Error too many pixels read");
					fclose(fTGA);
					fTGA = NULL;
					free(colorbuffer);
					colorbuffer = NULL;
					free(texture->imageData);
					texture->imageData =NULL;
					exit(0);
				}

			}


		}


	} while (currentpixel < pixelcount); 

	free(colorbuffer);
	colorbuffer = NULL;

}

void SET_TGA_INFOMATION(TGA_IMG *texture, FILE *fTGA) {

	unsigned char tmp_headear[6];
	if (fread(tmp_headear, sizeof(tmp_headear), 1, fTGA) == 0)
			{
				printf("can't read next header");
				exit(0);
			}

			texture->width = tmp_headear[1] * 256 + tmp_headear[0];
			texture->height = tmp_headear[3] * 256 + tmp_headear[2];
			texture->bpp = tmp_headear[4]; //24 or 32 bit

			if((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp !=32)))
			{
				printf("Wrong Data");
				fclose(fTGA);fTGA = NULL;
				exit(0);
			}

			texture->color_num = (texture->bpp / 8);
			texture->imageSize = (texture->color_num * texture->width * texture->height);
			texture->imageData = (unsigned char*)malloc(texture->imageSize * sizeof(unsigned char));
			if(texture->imageData == NULL)
			{
				printf("sorry can't get memory");
				fclose(fTGA);fTGA = NULL;
				free(texture->imageData); texture->imageData =NULL;
				exit(0);
			}
		}