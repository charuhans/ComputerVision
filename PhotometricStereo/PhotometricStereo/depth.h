#ifndef DEPTH_H
#define DEPTH_H

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "tgautils.h"
extern "C" 
{
	#include "cs.h";
}
using namespace std;

///*==========================================================================================================
//												Variable Declaration
//===========================================================================================================*/
int getsizePixel(IplImage* image);
typedef struct validPixel
{	int size;
	CvMat* X;
	CvMat* Y;
	void setSizePixel(IplImage* image)
	{
		size = getsizePixel(image);
		X = cvCreateMat(size,1,CV_32FC1);
	    Y = cvCreateMat(size,1,CV_32FC1);
	} 
}valPixel;


///*==========================================================================================================
//												Functions Definations
//===========================================================================================================*/
int getsizePixel(IplImage* image)
{
	int count = 0;
	for(int i =0; i< image->height; i++) {
		for (int j =0; j< image->width; j++) {
			if (uchar(image->imageData[i * image->widthStep +j]) != 0) {
				count++;
			}
		}
	}
	return count;
}
validPixel getvalidPixel(IplImage* image)
{
	validPixel valPixel;
	valPixel.setSizePixel(image);
	int count = 0;
	for(int i =0; i< image->height; i++) {
		for (int j =0; j< image->width; j++) {
			if (uchar(image->imageData[i * image->widthStep +j]) != 0) {
				cvSet2D(valPixel.X, count, 0, cvScalar(j));
				cvSet2D(valPixel.Y, count, 0 , cvScalar(i));
				count++;
			}
		}
	}
	return valPixel;
}


void reconstructSurface(char directory[150], CvMat* normal)
{
	validPixel valPixel;	//struct that holds the pixel info of the binary image
	double *b, *b1, nx, nz, ny;
	char maskfile[100], resultObject[150], resultFile[100];	
	IplImage* mask;
	strcpy(maskfile, directory);
	strcat(maskfile, ".mask.tga");
	mask = GetImageFromTGA(maskfile);
	IplImage* graymask = cvCreateImage( cvGetSize(mask), IPL_DEPTH_8U, 1 );
	cvCvtColor( mask, graymask, CV_BGR2GRAY );
	cvReleaseImage(&mask);	
	int width = graymask->width ;
	int height = graymask->height;
	CvMat* temp = cvCreateMat(height,width, CV_32FC1);
    cvZero(temp);
	CvMat* shape = cvCreateMat(height,width, CV_32FC1);
    cvZero(shape);
	int numPix, pRow, pCol;
	valPixel = getvalidPixel(graymask);
	numPix = valPixel.size;
	int equationSize = 2*height*width;
	cs * M = cs_spalloc (equationSize, numPix ,0, 1, 1);
	
	b = (double *) cs_malloc(equationSize, sizeof(double));
	for (int i = 0; i < equationSize; ++i) 
	{
		b[i] = 0.0;
	}
	b1 = (double *) cs_malloc(equationSize, sizeof(double));
	for (int i = 0; i < equationSize; ++i) 
	{
		b1[i] = 0.0;
	}

	int num_equation = 0;
	int nNonZeros = 0;
	CvScalar n;
	for(int d = 0; d < numPix; d++)
	{
		 pRow = (int)cvGetReal2D(valPixel.X, d, 0);
		 pCol = (int)cvGetReal2D(valPixel.Y, d, 0);
		 n = cvGet2D(normal, pCol, pRow);
		 nx = n.val[0];
		 ny = n.val[1];
		 nz = n.val[2];

		//  right neighbor
		if ((cvGetReal2D(graymask, pCol, pRow+1) != 0))
		{
			cs_entry(M, num_equation, pCol*width + pRow, -1.0 * nz);
			cs_entry(M, num_equation, pCol*width + (pRow + 1), nz);
			b[num_equation] = -nx;
			num_equation++;	

			//testinX->imageData[pCol*testinX->widthStep+pRow] = 255;
		}

		// down neighbor
		if ((cvGetReal2D(graymask, pCol+1, pRow) != 0))
		{
			cs_entry(M, num_equation, pCol*width + pRow, -1.0 * nz);
			cs_entry(M, num_equation, (pCol + 1)*width + pRow, nz);
			b[num_equation] = -ny;
			num_equation++;			

		}
	}
	
	cs * compressedM = cs_compress(M);
	cs * MT = cs_transpose(compressedM, true); 
	cs * M2 = cs_multiply(MT, compressedM);
	cs_gaxpy(MT, b, b1);
	int a = cs_qrsol(2, M2, b1);
	for(int d = 0; d < numPix; d++)
	{
		pRow = (int)cvGetReal2D(valPixel.X, d, 0);
		pCol = (int)cvGetReal2D(valPixel.Y, d, 0);
		cvSet2D(temp, pCol, pRow, cvScalar(b1[pCol*width+pRow]));
	}	

	//cvFlip(temp,temp,1);
	cs_spfree(compressedM);	
	cs_spfree(MT);
	cs_spfree(M2);
	CvMat* object = cvCreateMat(height, width, CV_8UC1);
	strcpy(resultFile, directory);
	strcat(resultFile,"depth.txt");;
	//ofstream out(resultFile);

	/*IplImage* tester = cvCreateImage(cvSize(width,height),IPL_DEPTH_32F, 1);
	for (int y =0; y< height; y++){
		for (int x =0; x < width; x++)
		{
			tester->imageData[y*tester->widthStep+x] = cvGet2D(temp,y,x).val[0];
		}
	}
	cvAddS(tester, cvScalar(100), tester, graymask);
	cvSaveImage("temp.jpg",tester);*/

	ofstream out("test.txt");
	out<< height<< " " <<width<<"\n";
	for (int y =0; y< height; y++){
		for (int x =0; x < width; x++)
		{
			out << cvGet2D(temp,y,x).val[0] + 150 << " ";
		}
		out << "\n";
	}
	out.close();
	//double minVal = 10000.0;
	//for (int x = 0; x < temp->width; ++x) 
	//{
	//	for (int y = 0; y < temp->height; ++y) 
	//	{
	//	if(cvGetReal2D(temp, y, x) < minVal) 
	//		minVal = cvGetReal2D(temp, y, x);
	//	}
	//}

	////cvSubS(temp, cvScalar(minVal), temp, graymask);
   cvConvert(temp, object);
	//cvAddS(object, cvScalar(100), object, graymask);
	cvNamedWindow("Depth Map");
    cvShowImage("Depth Map", object);
	//strcpy(resultObject, directory);
	//strcat(resultObject,"depth");
	//strcat(resultObject,".jpg");
	//cvSaveImage(resultObject,object);
	//cvWaitKey(0);
}
void reconstructSurface1(char directory[150], CvMat* normal, int threshold)
{
	validPixel valPixel;	//struct that holds the pixel info of the binary image
	double *b, *b1, nx, nz, ny;
	char maskfile[100], resultObject[150], resultFile[100];	
	strcpy(maskfile, directory);
	strcat(maskfile, "im1.jpg");
	IplImage *image, *rgb;
	image = cvLoadImage(maskfile);
	IplImage* graymask = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
	IplImage* mask = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
	cvCvtColor( image, graymask, CV_BGR2GRAY );
	cvThreshold(graymask, mask, threshold, 255, CV_THRESH_BINARY);
	//cvReleaseImage(&graymask);
	int width = mask->width ;
	int height = mask->height;
	CvMat* temp = cvCreateMat(height,width, CV_32FC1);
    cvZero(temp);
	int numPix, pRow, pCol;
	valPixel = getvalidPixel(mask);
	numPix = valPixel.size;
	int equationSize = 2*height*width;
	cs * M = cs_spalloc (equationSize, numPix ,0, 1, 1);
	
	b = (double *) cs_malloc(equationSize, sizeof(double));
	for (int i = 0; i < equationSize; ++i) 
	{
		b[i] = 0.0;
	}
	b1 = (double *) cs_malloc(equationSize, sizeof(double));
	for (int i = 0; i < equationSize; ++i) 
	{
		b1[i] = 0.0;
	}

	int num_equation = 0;
	int nNonZeros = 0;
	CvScalar n;
	for(int d = 0; d < numPix; d++)
	{
		 pRow = (int)cvGetReal2D(valPixel.X, d, 0);
		 pCol = (int)cvGetReal2D(valPixel.Y, d, 0);
		 n = cvGet2D(normal, pCol, pRow);
		 nx = n.val[0];
		 ny = n.val[1];
		 nz = n.val[2];

		//  right neighbor
		if (( pRow + 1 < width) &&(cvGetReal2D(mask, pCol, pRow+1) != 0))
		{
			cs_entry(M, num_equation, pCol*width + pRow, -1.0 * nz);
			cs_entry(M, num_equation, pCol*width + (pRow + 1), nz);
			b[num_equation] = -nx;
			num_equation++;	

			//testinX->imageData[pCol*testinX->widthStep+pRow] = 255;
		}

		// down neighbor
		if (( pCol + 1 < height) &&(cvGetReal2D(mask, pCol+1, pRow) != 0))
		{
			cs_entry(M, num_equation, pCol*width + pRow, -1.0 * nz);
			cs_entry(M, num_equation, (pCol + 1)*width + pRow, nz);
			b[num_equation] = -ny;
			num_equation++;			

		}
	}
	
	cs * compressedM = cs_compress(M);
	cs * MT = cs_transpose(compressedM, true); 
	cs * M2 = cs_multiply(MT, compressedM);
	cs_gaxpy(MT, b, b1);
	int a = cs_qrsol(2, M2, b1);
	for(int d = 0; d < numPix; d++)
	{
		pRow = (int)cvGetReal2D(valPixel.X, d, 0);
		pCol = (int)cvGetReal2D(valPixel.Y, d, 0);
		cvSet2D(temp, pCol, pRow, cvScalar((-1.0*b1[pCol*width+pRow])+ 100));
	}	

	//cvFlip(temp,temp,1);
	cs_spfree(compressedM);	
	cs_spfree(MT);
	cs_spfree(M2);
	CvMat* object = cvCreateMat(height, width, CV_8UC1);
	strcpy(resultFile, directory);
	strcat(resultFile,"depth.txt");;
	//ofstream out(resultFile);

	/*IplImage* tester = cvCreateImage(cvSize(width,height),IPL_DEPTH_32F, 1);
	for (int y =0; y< height; y++){
		for (int x =0; x < width; x++)
		{
			tester->imageData[y*tester->widthStep+x] = cvGet2D(temp,y,x).val[0];
		}
	}
	cvAddS(tester, cvScalar(100), tester, graymask);
	cvSaveImage("temp.jpg",tester);*/

	ofstream out("test.txt");
	out<< height<< " " <<width<<"\n";
	for (int y =0; y< height; y++){
		for (int x =0; x < width; x++)
		{
			out << cvGet2D(temp,y,x).val[0] << " ";
		}
		out << "\n";
	}
	out.close();
	//double minVal = 10000.0;
	//for (int x = 0; x < temp->width; ++x) 
	//{
	//	for (int y = 0; y < temp->height; ++y) 
	//	{
	//	if(cvGetReal2D(temp, y, x) < minVal) 
	//		minVal = cvGetReal2D(temp, y, x);
	//	}
	//}
	//CvMat* object = cvCreateMat(height, width, CV_8UC1);
	//cvSubS(temp, cvScalar(minVal), temp, graymask);
	cvAddS(object, cvScalar(100), object, graymask);
   cvConvert(temp, object);
	cvNamedWindow("Depth Map");
    cvShowImage("Depth Map", object);
	//strcpy(resultObject, directory);
	//strcat(resultObject,"depth");
	//strcat(resultObject,".jpg");
	//cvSaveImage(resultObject,object);
	//cvWaitKey(0);
}


#endif