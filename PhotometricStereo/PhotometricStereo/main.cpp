#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <cxcore.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include "tgautils.h"
#include "calibration.h"
#include "normal1.h";
#include "depth.h";
#include "rgb2suv.h";
#include "normal2.h";
extern "C" 
{
	#include "cs.h";
}
using namespace std;

int main()
{

		int numLights = 8;
		string data1;
		char* directory = "dataset\\chrome\\chrome";
		char* dir = "dataset\\";
		char fileName[150];
		CvMat* lightDirection = CalibrateLight(directory, numLights);
		double lightSource[3] = {0.6257,0.5678,0.5349};
		cout<< "Dataset Name:"<<endl;
		cout<< "1. rock"<<endl<< "2. owl"<<endl<< "3. horse"<<endl<< "4. gray"<<endl<< "5. cat"<<endl;
		cout<< "6. buddha"<<endl<< "7. pear"<<endl<< "8. sphere"<<endl<< "9. diffSphere"<<endl<< "10. diffPear"<<endl;
		cout<<"Enter a Dataset Name:"<<endl;
		cin>>data1;
		const char* data = data1.c_str();
		if(!(strcmp(data, "rock") && strcmp(data, "owl") && strcmp(data, "horse") && strcmp(data, "gray") && strcmp(data, "cat")&& 
			strcmp(data, "buddha") && strcmp(data, "pear")&& strcmp(data, "sphere")&& strcmp(data, "diffSphere")&& strcmp(data, "diffPear"))!= 0)
		{
			strcpy_s(fileName,dir);
			strcat(fileName,data);
			strcat(fileName,"\\");
		}
		else
			cout<<"Match not found";

		if(!(strcmp(data, "pear")))
		{
			//output(fileName, lightSource); // only for saving s and diffuse
			CvMat* normal = surfaceNormal1(fileName, 4, 3, data);
			colorAlbedo(fileName, 4, normal, data); // if want to show color albedo
			reconstructSurface1(fileName, normal, 6); // 6 is threshold
		}
		else if (!strcmp(data, "diffPear"))
		{
			//output(fileName, lightSource); // only for saving s and diffuse
			CvMat* normal = surfaceNormal1(fileName, 4, 3, data);
			reconstructSurface1(fileName, normal, 4); // 3 is threshold
		}
		else if (!strcmp(data, "sphere"))
		{
			//output(fileName, lightSource); // only for saving s and diffuse
			CvMat* normal =  surfaceNormal1(fileName, 4, 5, data);
			colorAlbedo(fileName, 4, normal, data); // if want to show color albedo
			reconstructSurface1(fileName, normal, 5); // 5 is threshold
		}
		else if (!strcmp(data, "diffSphere"))
		{
			//output(fileName, lightSource); // only for saving s and diffuse
			CvMat* normal = surfaceNormal1(fileName, 4, 5, data);
			reconstructSurface1(fileName, normal, 5); // 5 is threshold
		}
		else
		{
			strcat(fileName,data);
			CvMat* normal = surfaceNormal(fileName, lightDirection, numLights);		
			colorAlbedo(fileName, lightDirection, numLights, normal);
			reconstructSurface(fileName, normal);
		}
		cvWaitKey();
        return 0;
}