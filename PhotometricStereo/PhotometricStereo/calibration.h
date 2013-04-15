#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "tgautils.h"

using namespace std;
/*==========================================================================================================
												Variable Declaration
===========================================================================================================*/
typedef struct Info
{
	float x;
	float y;
	float radius;
}get_info;

/*==========================================================================================================
												Functions Declaration
===========================================================================================================*/
IplImage* GetImageFromTGA(char* fileName);
string convertInt(int number);
CvMat* CalibrateLight(char* directory, int numLights);

float Max(IplImage* image);
float Max(vector<float> x);
float Max(vector<float> x);
Info getInfo(IplImage* image);

/*==========================================================================================================
												Functions Definations
===========================================================================================================*/

float Max(IplImage* image)
{
	float max = -1;
	for(int i =0; i< image->height; i++)
	{
		for (int j =0; j< image->width; j++)
		{
			if (uchar(image->imageData[i * image->widthStep +j]) > max)
			{
					max = uchar(image->imageData[i * image->widthStep +j]);
			}
		}
	}
	return max;
}

float Max(vector<float> xy)
{
	float maximum = 0.0;
	for(int i =0; i < xy.size(); i++)
	{
		if(xy[i] >= maximum)
			maximum = xy[i];
	}
	return maximum;
}

float Min(vector<float> xy)
{
	float minimum = 1000.0;
		for(int i =0; i < xy.size(); i++)
	{
		if(xy[i] <= minimum)
			minimum = xy[i];
	}
	return minimum;
}

Info getInfo(IplImage* image)
{
	Info get_info;
	float maximum;
	float min_x, max_x, min_y, max_y;
	vector<float> x;
	vector<float> y;
	maximum = Max(image);

	for(int i =0; i< image->height; i++) {
		for (int j =0; j< image->width; j++) {
			if (uchar(image->imageData[i * image->widthStep +j]) == maximum) {
				x.push_back((float)j);
				y.push_back((float)i);
			}
		}
	}
	max_x = Max(x);
	min_x = Min(x);
	max_y = Max(y);
	min_y = Min(y);

	get_info.x = (float)((max_x + min_x)/2.0);
	get_info.y = (float)((max_y + min_y)/2.0);
	get_info.radius = (float)((max_x - min_x)/2.0); 

	return get_info;
}

IplImage* GetImageFromTGA(char* fileName)
{
	TGA_IMG* tga_img = (TGA_IMG*)malloc(1 * sizeof (TGA_IMG));
	LoadTGA(fileName, tga_img);

	CvSize window_size;
	window_size.width = tga_img->width;
	window_size.height= tga_img->height;

	IplImage* image = cvCreateImage(window_size, IPL_DEPTH_8U, 3);
	memcpy(image->imageData, tga_img->imageData, tga_img->width*tga_img->height*3);
	
	if (tga_img->RLE_flag == true) 
	{
		cvCvtColor(image, image, CV_RGB2BGR);
	}

	cvFlip(image, NULL, 0);	
	free(tga_img->imageData);
	free(tga_img);
	CvMemStorage* storage = cvCreateMemStorage(0);

	return image;
}


CvMat* CalibrateLight(char* directory, int numLights)
{
	
	char maskfile[100];
	strcpy(maskfile, directory);
	strcat(maskfile, ".mask.tga");
	IplImage *mask, *chrome;
	mask = GetImageFromTGA(maskfile);
	IplImage* graymask = cvCreateImage( cvGetSize(mask), IPL_DEPTH_8U, 1 );
	cvCvtColor( mask, graymask, CV_BGR2GRAY );
	cvReleaseImage(&mask);
	Info maskcircle =  getInfo(graymask);
	char buff[10];
	CvMat *lightSource = cvCreateMat(numLights, 3, CV_32FC1);
	ofstream output("light.txt");


	for(int i = 0; i < numLights; i++)
	{			
		char chromefile[150];
		strcpy(chromefile, directory);
		strcat(chromefile,".");
		strcat(chromefile, itoa(i,buff,10));
		strcat(chromefile,".tga");
		chrome = GetImageFromTGA(chromefile);
		IplImage* graychrome = cvCreateImage( cvGetSize(chrome), IPL_DEPTH_8U, 1 );
		cvCvtColor( chrome, graychrome, CV_BGR2GRAY );
		cvReleaseImage(&chrome);
		Info chromecircle =  getInfo(graychrome);
		float N[3];
		N[0] = (chromecircle.x - maskcircle.x);
		N[1] = -(chromecircle.y - maskcircle.y);
		N[2] = sqrt(maskcircle.radius * maskcircle.radius - N[0]*N[0] - N[1]*N[1]);
		for(int i=0; i<3; i++)
		{
			N[i] = N[i]/maskcircle.radius;
		}
		// L = 2*(N.R)*N - R
		CvMat normal = cvMat(1, 3, CV_32FC1, N);
		float R[3] = {0,0,1};
		CvMat reflection = cvMat(1, 3, CV_32FC1, R);
		double NR = cvDotProduct(&normal, &reflection);
		CvMat *light = cvCreateMat(1, 3, CV_32FC1);
		CvMat *scale = cvCreateMat(1, 3, CV_32FC1);

		cvScale(&normal, scale, NR*2, 0);
		cvSub(scale, &reflection, light);
		cvSet2D(lightSource,i, 0, cvScalar(light->data.fl[0]));
        cvSet2D(lightSource,i, 1, cvScalar(light->data.fl[1]));
        cvSet2D(lightSource,i, 2, cvScalar(light->data.fl[2]));
		output << light->data.fl[0];
		output << " , ";
		output << light->data.fl[1];
		output << " , ";
		output << light->data.fl[2];
		output << " \n ";	
	}
	output.close();
return lightSource;
}

#endif