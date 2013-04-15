#ifndef RGB2SUV_H
#define RGB2SUV_H

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <math.h>
#include "tgautils.h"

using namespace std;


/*==========================================================================================================
												Functions Declaration
===========================================================================================================*/
IplImage* rgb2suv(IplImage* rgb, CvMat* lightSource);
/*==========================================================================================================
												Functions Definations
===========================================================================================================*/

IplImage* rgb2suv(IplImage* rgb, CvMat* lightSource) 
{
	double cosAngle, angle, qx, qy, qz, qw;
	IplImage* suv, *s, *u, *v, *diffuse;
	CvScalar c;
	suv = cvCreateImage(cvGetSize(rgb), IPL_DEPTH_8U, 3);
	s = cvCreateImage(cvGetSize(rgb), IPL_DEPTH_8U, 1);
	u = cvCreateImage(cvGetSize(rgb), IPL_DEPTH_8U, 1);
	v = cvCreateImage(cvGetSize(rgb), IPL_DEPTH_8U, 1);
	diffuse = cvCreateImage(cvGetSize(rgb), IPL_DEPTH_8U, 1);
	//CvMat* rgbMatrix = cvCreateMat(rgb->height, rgb->width, CV_32FC1);
	CvMat* rotation = cvCreateMat(3, 3, CV_32FC1);
	CvMat* cross = cvCreateMat(3, 1, CV_32FC1);
	CvMat* I = cvCreateMat(3, 1, CV_32FC1);
	CvMat* temp = cvCreateMat(3,1,CV_32FC1);
	cvSet2D(I, 0, 0, cvScalar(1));
	cvSet2D(I, 1, 0, cvScalar(0));
	cvSet2D(I, 2, 0, cvScalar(0));
	// rotation*lightSource = I
	cosAngle = cvDotProduct(lightSource,I);
	angle = acos(cosAngle);
	cvCrossProduct(lightSource,I, cross);
	double norm = cvNorm(cross, NULL);

	qx = (cvGet2D(cross, 0, 0)).val[0]/norm  * sin(angle/2);
	qy = (cvGet2D(cross, 1, 0)).val[0]/norm * sin(angle/2);
	qz = (cvGet2D(cross, 2, 0)).val[0]/norm * sin(angle/2);
	qw = cos(angle/2);
	// Fill the rotation matrix
	cvSet2D(rotation, 0, 0, cvScalar(1 - 2*qy*qy - 2*qz*qz));
	cvSet2D(rotation, 0, 1, cvScalar(2*qx*qy - 2*qz*qw));
	cvSet2D(rotation, 0, 2, cvScalar(2*qx*qz + 2*qy*qw));
	cvSet2D(rotation, 1, 0, cvScalar(2*qx*qy + 2*qz*qw));
	cvSet2D(rotation, 1, 1, cvScalar(1 - 2*qx*qx - 2*qz*qz));
	cvSet2D(rotation, 1, 2, cvScalar(2*qy*qz - 2*qx*qw));
	cvSet2D(rotation, 2, 0, cvScalar(2*qx*qz - 2*qy*qw));
	cvSet2D(rotation, 2, 1, cvScalar(2*qy*qz + 2*qx*qw));
	cvSet2D(rotation, 2, 2, cvScalar(1 - 2*qx*qx - 2*qy*qy));

	for (int y = 0; y < rgb ->height; ++y) 
	{
		for (int x = 0; x < rgb->width; ++x)
		{
			c = cvGet2D(rgb, y, x);
			temp->data.fl[0] = c.val[2];
			temp->data.fl[1] = c.val[1];
			temp->data.fl[2] = c.val[0];
			cvMatMul(rotation,temp, temp);
			c.val[0] = temp->data.fl[0];
			c.val[1] = temp->data.fl[1];
			c.val[2] = temp->data.fl[2];
			cvSet2D(suv, y, x, c);
	
			cvSet2D(s, y, x, cvScalar(temp->data.fl[0]));//c.val[0]
			cvSet2D(diffuse, y, x, cvScalar(sqrt(temp->data.fl[1]*temp->data.fl[1] + temp->data.fl[2]*temp->data.fl[2])));
			cvSet2D(u, y, x, cvScalar(sqrt(temp->data.fl[1]*temp->data.fl[1])));
			cvSet2D(v, y, x, cvScalar(sqrt(temp->data.fl[2]*temp->data.fl[2])));
		}
	}

	/*cvNamedWindow("SUV space");
    cvShowImage("SUV space", suv);
	cvWaitKey();
	cvNamedWindow("S space");
    cvShowImage("S space", s);

	cvNamedWindow("U space");
    cvShowImage("U space", u);

	cvNamedWindow("V space");
    cvShowImage("V space", v);

	cvNamedWindow("Diffuse");
    cvShowImage("Diffuse", diffuse);
	cvWaitKey();
	cvSaveImage("s.jpg",s);
	cvWaitKey();*/
	return diffuse;

}

void output(char * directory, double lightSource[3])
{
	IplImage* rgb, *diffuse;
	char buff[10];
	CvMat* colorSrc = cvCreateMat(3, 1, CV_32FC1);
	cvSet2D(colorSrc, 0, 0, cvScalar(lightSource[0]));
	cvSet2D(colorSrc, 1, 0, cvScalar(lightSource[1]));
	cvSet2D(colorSrc, 2, 0, cvScalar(lightSource[2]));
	for(int i = 0; i < 4; i++)
	{
		char imagefile[150], savefile[100];		
		strcpy(imagefile, directory);
		strcat(imagefile,"\im");
		strcat(imagefile, itoa(i + 1,buff,10));
		strcat(imagefile,".jpg");
		rgb = cvLoadImage(imagefile);
		//cvNamedWindow("rgb");
		//cvShowImage("rgb", rgb);
		//cvWaitKey();
		diffuse = rgb2suv(rgb, colorSrc);
		strcpy(savefile, directory);
		strcat(savefile,"imd");
		//strcat(savefile,"s");
		strcat(savefile, itoa(i + 1,buff,10));
		strcat(savefile,".jpg");
		cvSaveImage(savefile,diffuse);
		cvReleaseImage(&diffuse);
	}
}

#endif