#ifndef NORMAL2_H
#define NORMAL2_H

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <string>
#include "tgautils.h"

using namespace std;


/*==========================================================================================================
												Functions Definations
===========================================================================================================*/


CvMat* readLight(char* directory, int numLights, const char* data)
{
	CvMat *lightDirection = cvCreateMat(numLights, 3, CV_32FC1);
	if(!(strcmp(data, "pear")&& strcmp(data, "diffPear")))
	{
		cvSet2D(lightDirection, 0, 0, cvScalar(-0.3425));
		cvSet2D(lightDirection, 0, 1, cvScalar(-0.2633));
		cvSet2D(lightDirection, 0, 2, cvScalar(0.9019));
		cvSet2D(lightDirection, 1, 0, cvScalar(-0.3505));
		cvSet2D(lightDirection, 1, 1, cvScalar(0.2989));
		cvSet2D(lightDirection, 1, 2, cvScalar(0.8876));
		cvSet2D(lightDirection, 2, 0, cvScalar(0.2720));
		cvSet2D(lightDirection, 2, 1, cvScalar(0.3372));
		cvSet2D(lightDirection, 2, 2, cvScalar(0.9013));
		cvSet2D(lightDirection, 3, 0, cvScalar(0.2294));
		cvSet2D(lightDirection, 3, 1, cvScalar(-0.1068));
		cvSet2D(lightDirection, 3, 2, cvScalar(0.9675));
	}
	else
	{
		cvSet2D(lightDirection, 0, 0, cvScalar(-0.3836));
		cvSet2D(lightDirection, 0, 1, cvScalar(-0.2366));
		cvSet2D(lightDirection, 0, 2, cvScalar(0.8927));
		cvSet2D(lightDirection, 1, 0, cvScalar(-0.3728));
		cvSet2D(lightDirection, 1, 1, cvScalar(0.3039));
		cvSet2D(lightDirection, 1, 2, cvScalar(0.8767));
		cvSet2D(lightDirection, 2, 0, cvScalar(0.2508));
		cvSet2D(lightDirection, 2, 1, cvScalar(0.3475));
		cvSet2D(lightDirection, 2, 2, cvScalar(0.9035));
		cvSet2D(lightDirection, 3, 0, cvScalar(0.2038));
		cvSet2D(lightDirection, 3, 1, cvScalar(-0.0963));
		cvSet2D(lightDirection, 3, 2, cvScalar(0.9743));
	}

	return lightDirection;
}

CvMat* surfaceNormal1(char directory[150], int numLights , int threshold, const char* data)
{
	char maskfile[100],resultAlbedo[150],rnormalRGB[150],rnormalR[150],rnormalG[150],rnormalB[150],buff[10],imagefile[150];
	CvMat* lightDirection, *normal;
	strcpy(maskfile, directory);
	strcat(maskfile, "im1.jpg");
	IplImage *image, *rgb;
	image = cvLoadImage(maskfile);
	IplImage* graymask = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
	IplImage* mask = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
	cvCvtColor( image, graymask, CV_BGR2GRAY );
	cvThreshold(graymask, mask, threshold, 255, CV_THRESH_BINARY);
	lightDirection =  readLight(directory,numLights,data);
	int height = graymask->height;
	int width = graymask->width;	
	int widthStep = graymask->widthStep;
	//int numLights = 12;
	CvMat* ImgMatrix = cvCreateMat(numLights, numLights, CV_32FC1);
    CvMat* imageMatrix = cvCreateMat(numLights, 1, CV_32FC1);
	CvMat* imageMatrixNew = cvCloneMat(imageMatrix);

	CvMat* lightDirectionNew = cvCloneMat(lightDirection);
	CvMat* gMatrix = cvCreateMat(3, 1, CV_32FC1); 
	CvMat* albedo =  cvCreateMat(height, width, CV_8UC1);	
    normal = cvCreateMat(height, width, CV_32FC3);
	IplImage* normalRGB = cvCreateImage(cvSize(width, height), 8, 3);	
	IplImage* normalR = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage* normalG = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage* normalB = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage* grayImage[4];
	cvZero(ImgMatrix);
    cvZero(albedo);
    cvZero(normal);
	cvZero(normalRGB);
	cvZero(normalR);
	cvZero(normalG);
	cvZero(normalB);
	for(int i = 0; i < numLights; i++)
	{			
		strcpy(imagefile, directory);
		strcat(imagefile,"im");
		strcat(imagefile, itoa(i+1,buff,10));
		strcat(imagefile,".jpg");
		rgb = cvLoadImage(imagefile);
		grayImage[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1 );
		cvCvtColor(rgb, grayImage[i], CV_BGR2GRAY );
		cvReleaseImage(&rgb);
	}

	for (int y = 0; y < height; ++y) 
	{
		for (int x = 0; x < width; ++x) 
		{

			if (cvGetReal2D(mask,y,x))
			{
				for(int i = 0; i < numLights; i++)
				{
					double temp = cvGetReal2D(grayImage[i], y, x);
					cvSet2D(imageMatrix, i, 0, cvScalar(temp));
					cvSet2D(ImgMatrix, i, i, cvScalar(temp));
				}
				/* ===============Minimize the objective function, (I*I - I*L*g)=====================*/
				/*	I  = I*I	*/
				cvMatMul(ImgMatrix, imageMatrix, imageMatrixNew);
				/*	I  = I*L	*/
				cvMatMul(ImgMatrix, lightDirection, lightDirectionNew);
				/* (I*I - I*L*g) using linear least square */
				cvSolve(lightDirectionNew, imageMatrixNew, gMatrix, CV_SVD);
				/*	R  = norm(g)	*/
				double gNorm = cvNorm(gMatrix, NULL);
				/*	Albedo	*/
				cvSet2D(albedo, y, x, cvScalar(gNorm));
				/*	N  = g/R	*/
				cvNormalize(gMatrix, gMatrix);	            
				CvScalar gScaler = cvScalar(1.0*cvGetReal1D(gMatrix,0),1.0*cvGetReal1D(gMatrix,1),1.0*cvGetReal1D(gMatrix,2));
				cvSet2D(normal, y, x, gScaler);	                                
				//if (cvGetReal2D(graymask,y,x))
				//{
				normalRGB->imageData[y*normalRGB->widthStep+x*normalRGB->nChannels+0] = abs(255*gScaler.val[0]);
				normalRGB->imageData[y*normalRGB->widthStep+x*normalRGB->nChannels+1] = abs(255*gScaler.val[1]);
				normalRGB->imageData[y*normalRGB->widthStep+x*normalRGB->nChannels+2] = abs(255*gScaler.val[2]);
				cvSet2D(normalB, y, x, cvScalar(abs(255*gScaler.val[0])));
				cvSet2D(normalG, y, x, cvScalar(abs(255*gScaler.val[1])));
				cvSet2D(normalR, y, x, cvScalar(abs(255*gScaler.val[2])));
				//}
			}
        }
    }
	cvNamedWindow("Surface Normal", CV_WINDOW_AUTOSIZE);
	cvShowImage("Surface Normal", normalRGB);
	cvNamedWindow("redNormal", 0);
	cvShowImage("redNormal", normalR);
	cvNamedWindow("greenNormal", 0);
	cvShowImage("greenNormal", normalG);
	cvNamedWindow("blueNormal", 0);
	cvShowImage("blueNormal", normalB);
	cvNamedWindow("albedo", CV_WINDOW_AUTOSIZE);
	cvShowImage("albedo", albedo);
    /*strcpy(resultAlbedo, directory);
	strcat(resultAlbedo,"albedo");
	strcat(resultAlbedo,".jpg");
	cvSaveImage(resultAlbedo,albedo);
	strcpy(rnormalRGB, directory);
	strcat(rnormalRGB,"normalRGB");
	strcat(rnormalRGB,".jpg");
	cvSaveImage(rnormalRGB,normalRGB);
	strcpy(rnormalR, directory);
	strcat(rnormalR,"normalR");
	strcat(rnormalR,".jpg");
	cvSaveImage(rnormalR,normalR);
	strcpy(rnormalG, directory);
	strcat(rnormalG,"normalG");
	strcat(rnormalG,".jpg");
	cvSaveImage(rnormalG,normalG);
	strcpy(rnormalB, directory);
	strcat(rnormalB,"normalB");
	strcat(rnormalB,".jpg");
	cvSaveImage(rnormalB,normalB);*/
	return normal;
}

void colorAlbedo(char* directory, int numLights, CvMat* normal, const char* data)
{
	
	IplImage* colorImage[4], *mask;
	CvMat* lightDirection;
	char maskfile[100], resultAlbedo[150], buff[10], imagefile[150];
	strcpy(maskfile, directory);
	strcat(maskfile, "im1.jpg");
	IplImage *image, *rgb;
	image = cvLoadImage(maskfile);
	IplImage* graymask = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
	cvCvtColor( image, graymask, CV_BGR2GRAY );
	lightDirection =  readLight(directory,numLights,data);
	for(int i = 0; i < numLights; i++)
	{			
		strcpy(imagefile, directory);
		strcat(imagefile,"im");
		strcat(imagefile, itoa(i+1,buff,10));
		strcat(imagefile,".jpg");
		colorImage[i] = cvLoadImage(imagefile);
		//cvReleaseImage(&rgb);
	}
	int height = colorImage[0]->height;
	int width = colorImage[0]->width;	
	int widthStep = colorImage[0]->widthStep;
	cvNamedWindow("Original Input", CV_WINDOW_AUTOSIZE);
	cvShowImage("Original Input", colorImage[0]);
	cvWaitKey(0);
	IplImage * colorAlbedo = cvCreateImage(cvSize(width, height), 8, 3);
	cvZero(colorAlbedo);
	/* albedo  by minimizing (I*I - k*L*gt) using linear least square */
	float num, denom, add;
	num = 0; denom = 0;
	CvMat* matAdd, *normalT, *normalTemp;
	normalT = cvCreateMat(1,3,CV_32FC1);
	matAdd = cvCreateMat(1,1,CV_32FC1);
	normalTemp = cvCreateMat(3,1,CV_32FC1);
	CvScalar temp_norm_scalar;
	CvMat * lightSource = cvCreateMat(1, 3, CV_32FC1);
	for(int x =0; x< height; x++)
		for(int y=0; y< width; y++)
		{
			for (int n = 0; n < colorAlbedo->nChannels; n++)
			{				
				//if (cvGetReal2D(graymask,y,x) > 0)
				if((uchar)graymask->imageData[x*graymask->widthStep+y] !=0)
				{
					for(int i = 0; i < numLights; i++)
					{
						CvScalar normalScalar = cvGet2D(normal, x, y);
						normalTemp->data.fl[0] = normalScalar.val[0];
						normalTemp->data.fl[1] = normalScalar.val[1];
						normalTemp->data.fl[2] = normalScalar.val[2];

						CvMat * lightSource = cvCreateMat(1, 3, CV_32FC1);						
						cvmSet(lightSource,0, 0, cvmGet(lightDirection,i,0));
						cvmSet(lightSource,0, 1, cvmGet(lightDirection,i,1));
						cvmSet(lightSource,0, 2, cvmGet(lightDirection,i,2));
						/* sum(L*g) */
						cvMatMul(lightSource, normalTemp, matAdd);
						/* sumAlbedo = sum( I*L*g) */
						num += matAdd->data.fl[0] * (uchar)colorImage[i]->imageData[x*widthStep+y*colorImage[i]->nChannels+n];
						/* denom = (sum(L*g))^2 */
						denom += matAdd->data.fl[0] * matAdd->data.fl[0];
					}
					/*============================ k = num/den ==================================*/
					colorAlbedo->imageData[x*colorAlbedo->widthStep+y*colorAlbedo->nChannels+n] = abs(num/denom);
					num = 0;
					denom = 0;
				}
			}
		}
	cvNamedWindow("ColorAlbedo", CV_WINDOW_AUTOSIZE);
	cvShowImage("ColorAlbedo", colorAlbedo);
	/*strcpy(resultAlbedo, directory);
	strcat(resultAlbedo,"colorAlbedo");
	strcat(resultAlbedo,".jpg");
	cvSaveImage(resultAlbedo,colorAlbedo);*/
}

#endif