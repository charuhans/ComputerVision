#ifndef NORAML1_H
#define NORAML1_H

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "tgautils.h"

using namespace std;

/*==========================================================================================================
												Functions Declaration
===========================================================================================================*/

CvMat* surfaceNormal( char directory[150], CvMat* lightDirection, int numLights);
void colorAlbedo(char directory[150], CvMat* lightDirection, int numLights, CvMat* normal);
/*==========================================================================================================
												Functions Definations
===========================================================================================================*/


CvMat* surfaceNormal(char directory[150], CvMat* lightDirection, int numLights)
{
	char maskfile[100], buff[10], imagefile[150], savefile[150], resultnormalRGB[150];
	strcpy(maskfile, directory);
	strcat(maskfile, ".mask.tga");
	IplImage *mask, *image;
	mask = GetImageFromTGA(maskfile);
	IplImage* graymask = cvCreateImage( cvGetSize(mask), IPL_DEPTH_8U, 1 );
	cvCvtColor( mask, graymask, CV_BGR2GRAY );
	cvReleaseImage(&mask);		
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
    CvMat* normal = cvCreateMat(height, width, CV_32FC3);
	IplImage* normalRGB = cvCreateImage(cvSize(width, height), 8, 3);
	/*IplImage* normalR = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage* normalG = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage* normalB = cvCreateImage(cvSize(width, height), 8, 1);*/
	IplImage* grayImage[12];
	cvZero(ImgMatrix);
    cvZero(albedo);
	cvZero(normalRGB);
    cvZero(normal);
	for(int i = 0; i < numLights; i++)
	{				
		strcpy(imagefile, directory);
		strcat(imagefile,".");
		strcat(imagefile, itoa(i,buff,10));
		strcat(imagefile,".tga");
		image = GetImageFromTGA(imagefile);
		strcpy(savefile, directory);
		strcat(savefile,"in");
		strcat(savefile, itoa(i,buff,10));
		strcat(savefile,".jpg");
		cvSaveImage(savefile,image);
		grayImage[i] = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1 );
		cvCvtColor(image, grayImage[i], CV_BGR2GRAY );
		cvReleaseImage(&image);
	}

	for (int y = 0; y < height; ++y) 
	{
		for (int x = 0; x < width; ++x) 
		{

			if (cvGetReal2D(graymask,y,x))
			{
				for(int i = 0; i < numLights; i++)
				{
					float temp = cvGetReal2D(grayImage[i], y, x);
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
				/*cvSet2D(normalR, y, x, cvScalar(abs(255*gScaler.val[0])));
				cvSet2D(normalG, y, x, cvScalar(abs(255*gScaler.val[1])));
				cvSet2D(normalB, y, x, cvScalar(abs(255*gScaler.val[2])));*/
				//}
			}
        }
    }
	cvNamedWindow("Surface Normal", CV_WINDOW_AUTOSIZE);
	cvShowImage("Surface Normal", normalRGB);
	/*cvNamedWindow("NormalR", CV_WINDOW_AUTOSIZE);
	cvShowImage("NormalR", normalR);
	cvNamedWindow("NormalG", CV_WINDOW_AUTOSIZE);
	cvShowImage("NormalG", normalG);
	cvNamedWindow("NormalB", CV_WINDOW_AUTOSIZE);
	cvShowImage("NormalB", normalB);*/
	//cvWaitKey(0);
	/*strcpy(resultnormalRGB, directory);
	strcat(resultnormalRGB,"normalRGB");
	strcat(resultnormalRGB,".jpg");
	cvSaveImage(resultnormalRGB,normalRGB);*/
	return normal;
}

void colorAlbedo(char directory[150], CvMat* lightDirection, int numLights, CvMat* normal)
{
	
	IplImage* colorImage[12], *mask;
	char maskfile[100], imagefile[150], buff[10], resultAlbedo[150];
	strcpy(maskfile, directory);
	strcat(maskfile, ".mask.tga");
	mask = GetImageFromTGA(maskfile);
	IplImage* graymask = cvCreateImage( cvGetSize(mask), IPL_DEPTH_8U, 1 );
	cvCvtColor( mask, graymask, CV_BGR2GRAY );
	cvReleaseImage(&mask);
	for(int i = 0; i < numLights; i++)
	{			
		strcpy(imagefile, directory);
		strcat(imagefile,".");
		strcat(imagefile, itoa(i,buff,10));
		strcat(imagefile,".tga");
		colorImage[i] = GetImageFromTGA(imagefile);
	}
	int height = colorImage[0]->height;
	int width = colorImage[0]->width;	
	int widthStep = colorImage[0]->widthStep;
	cvNamedWindow("Original Input", CV_WINDOW_AUTOSIZE);
	cvShowImage("Original Input", colorImage[0]);
	IplImage * colorAlbedo = cvCreateImage(cvSize(width, height), 8, 3);
	/*IplImage * blueAlbedo = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage * greenAlbedo = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage * redAlbedo = cvCreateImage(cvSize(width, height), 8, 1);
	cvZero(redAlbedo);
	cvZero(greenAlbedo);
	cvZero(blueAlbedo);*/
	/* albedo for each channel by minimizing (I*I - k*L*gt) using linear least square */
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
				if (cvGetReal2D(graymask,x,y) > 0)
				//if((uchar)graymask->imageData[x*graymask->widthStep+y] !=0)
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
		/*for(int x =0; x< height; x++)
		{
			for(int y=0; y< width; y++)
			{
				CvScalar albedoScalar = cvGet2D(colorAlbedo, x, y);
				cvSet2D(redAlbedo, x,y,cvScalar(albedoScalar.val[0]));
				cvSet2D(greenAlbedo, x,y,cvScalar(albedoScalar.val[1]));
				cvSet2D(blueAlbedo, x,y,cvScalar(albedoScalar.val[2]));
			}
		}*/

	cvNamedWindow("Albedo", CV_WINDOW_AUTOSIZE);
	cvShowImage("Albedo", colorAlbedo);
	/*strcpy(resultAlbedo, directory);
	strcat(resultAlbedo,"colorAlbedo");
	strcat(resultAlbedo,".jpg");
	cvSaveImage(resultAlbedo,colorAlbedo);*/
	/*cvNamedWindow("redAlbedo", CV_WINDOW_AUTOSIZE);
	cvShowImage("redAlbedo", redAlbedo);
	cvNamedWindow("greenAlbedo", CV_WINDOW_AUTOSIZE);
	cvShowImage("greenAlbedo", greenAlbedo);
	cvNamedWindow("blueAlbedo", CV_WINDOW_AUTOSIZE);
	cvShowImage("blueAlbedo", blueAlbedo);*/
	//cvWaitKey(0);
}

#endif