/* --Sparse Optical Flow Demo Program--
 * Written by David Stavens (david.stavens@ai.stanford.edu)
 */
#include <string.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <vector>
#include <iostream>
#define MAX_CLUSTERS 5

using namespace std;

static const double pi = 3.14159265358979323846;



inline static double square(int a)
{
	return a * a;
}

/* This is just an inline that allocates images.  I did this to reduce clutter in the
 * actual computer vision algorithmic code.  Basically it allocates the requested image
 * unless that image is already non-NULL.  It always leaves a non-NULL image as-is even
 * if that image's size, depth, and/or channels are different than the request.
 */
inline static void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels)
{
	if ( *img != NULL )
    {
        return;
    }

	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		fprintf(stderr, "Error: Couldn't allocate image.  Out of memory?\n");
		exit(-1);
	}
}
void clustering(IplImage* image, CvPoint* myPoints, int numOfClusters, int numOfPoint, int lane, IplImage* original )
{
	
   
    IplImage* img = cvCreateImage( cvSize(image->width, image->height ), 8, 3 );
    CvRNG rng = cvRNG(-1);
    CvPoint ipt;
    CvScalar color[7];

    for ( int i = 0; i < 7; i++)
    {
        color[i] = CV_RGB( rand()&255, rand()&255, rand()&255 );
    }

    //color_tab[4] = CV_RGB(255,255,0);

   // cvNamedWindow( "clusters", 1 );
    int minX1, minY1, maxX1, maxY1;
		minX1 = minY1 = 1000;
		maxX1 = maxY1 = 0;

		int minX2, minY2, maxX2, maxY2;
		minX2 = minY2 = 1000;
		maxX2 = maxY2 = 0;   
    for(;;)
    {
        
        int  cluster_count = numOfClusters ;
		int i, sample_count = numOfPoint;

        CvMat* points = cvCreateMat(numOfPoint*2, 1, CV_32FC2 );
		// put data to mat
		for (int i=0; i< numOfPoint; i++){
			points->data.fl[2*i] = myPoints[i].x;
			points->data.fl[2*i+1] = myPoints[i].y;
		//	cvGetRows(
		}

        CvMat* clusters = cvCreateMat(numOfPoint*2, 1, CV_32SC1 );


        cvKMeans2( points, cluster_count, clusters, cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 20, 0.2 ));

        //cvZero( img );
        for( i = 0; i < sample_count; i++ )
        {
            
            int cluster_idx = clusters->data.i[i];
			if (cluster_idx == 0){
				ipt.x = (int)points->data.fl[i*2];
				ipt.y = (int)points->data.fl[i*2+1];
				if (ipt.x > maxX1) 
                    maxX1 = ipt.x;
				if (ipt.x < minX1)
                    minX1 = ipt.x;
				if (ipt.y > maxY1) 
                    maxY1 = ipt.y;
				if (ipt.y < minY1) 
                    minY1 = ipt.y;
				if (lane ==1){                    
					cvCircle( original, ipt, 3, color[0], 3, CV_AA, 0 );
				}
				if (lane ==2){
                    cvCircle( original, ipt, 3, color[1], 3, CV_AA, 0 );
				}
				if (lane ==3){
					cvCircle( original, ipt, 3, color[2], 3, CV_AA, 0 );
				}
				
			}
			if (cluster_idx == 1){
				ipt.x = (int)points->data.fl[i*2];
				ipt.y = (int)points->data.fl[i*2+1];                
				cvCircle( original, ipt, 3, color[3], 3, CV_AA, 0 );
				if (ipt.x > maxX2) maxX2 = ipt.x;
				if (ipt.x < minX2) minX2 = ipt.x;
				if (ipt.y > maxY2) maxY2 = ipt.y;
				if (ipt.y < minY2) minY2 = ipt.y;
				if (lane ==1){
					cvCircle( original, ipt, 3, color[4], 3, CV_AA, 0 );
				}
				if (lane ==2){
					cvCircle( original, ipt, 3, color[5], 3, CV_AA, 0 );
				}
				if (lane ==3){                    
					cvCircle( original, ipt, 3, color[6], 3, CV_AA, 0 );
				}
			}
            
        }

        cvReleaseMat( &points );
        cvReleaseMat( &clusters );
            break;
    }
    
    //cvDestroyWindow( "clusters" );
}
void FieldOfView(IplImage *image, int& x, int& y)
{
    IplImage* dst = 0, *src = 0;
    IplImage* color_dst = 0;
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* lines = 0;

    int i;
    
    dst = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U,1 );
    color_dst = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U,3);
    src = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U,1 );

    cvCvtColor ( image, src, CV_BGR2GRAY);
    cvCanny( src, dst, 50, 200, 3 );
	lines = cvHoughLines2( dst, storage, CV_HOUGH_STANDARD, 1, CV_PI/45, 100, 0, 0 );

    float* line = 0;
    float rho, rhoMin, rhoMax;
    float theta, thetaMin, thetaMax;
    CvPoint pt1, pt2, pt3, pt4,foe;
    double a, b;
    double x0, y0;

    double a11, a12, a21, a22, b1, b2;

    line = (float*)cvGetSeqElem(lines,0);

    rho = line[0];
    theta = line[1];

    thetaMin = theta;
    rhoMin = rho;
    thetaMax = 0;
    rhoMax = 0;

    for( i = 1; i < MIN(lines->total,100); i++ )
    {
       line = (float*)cvGetSeqElem(lines,i);
       rho = line[0];
       theta = line[1];
    
       if (theta < thetaMin)
       {
           thetaMin = theta;
           rhoMin = rho;
       }
       if (theta > thetaMax)
       {
           thetaMax = theta;
           rhoMax = rho;
       }

    }
    
    //get the co-ordinates of 1st parallel line
    a = cos(thetaMin), b = sin(thetaMin);
    x0 = a*rhoMin, y0 = b*rhoMin;
    pt1.x = cvRound(x0 + 1000*(-b));
    pt1.y = cvRound(y0 + 1000*(a));
    pt2.x = cvRound(x0 - 1000*(-b));
    pt2.y = cvRound(y0 - 1000*(a));
    cvLine( color_dst, pt1, pt2, CV_RGB(255,0,0), 3, CV_AA, 0 );

    //get the co-ordinates of 2nd parallel line
    a = cos(thetaMax), b = sin(thetaMax);
    x0 = a*rhoMax, y0 = b*rhoMax;
    pt3.x = cvRound(x0 + 1000*(-b));
    pt3.y = cvRound(y0 + 1000*(a));
    pt4.x = cvRound(x0 - 1000*(-b));
    pt4.y = cvRound(y0 - 1000*(a));
    cvLine( color_dst, pt3, pt4, CV_RGB(255,0,0), 3, CV_AA, 0 );


	a11 = pt2.y - pt1.y;
	a12 = pt1.x - pt2.x;
	b1 = (a11*pt1.x + a12*pt1.y);
	a21 = pt4.y - pt3.y;
	a22 = pt3.x - pt4.x;
	b2 = (a21*pt3.x + a22*pt3.y);

	double tmpX[2][1] = {  {0}, 
						   {0}};

	CvMat xx = cvMat(2, 1, CV_64FC1, tmpX);
	double tmpA[2][2] = {{a11, a12},
					  {a21, a22}};
	CvMat A = cvMat(2, 2, CV_64FC1,    tmpA);
	double tmpb[2][1] = {  {b1}, 
						{b2}};
	CvMat bb = cvMat(2,  1,  CV_64FC1, tmpb);
	cvSolve(&A,&bb,&xx,CV_LU);

	x = tmpX[0][0];
	y = tmpX[1][0];

	foe.x = x;
	foe.y = y;
}
CvPoint FindIntersessionPoint(CvPoint p1Line1, CvPoint p2Line1, CvPoint p1Line2, CvPoint p2Line2){
//get the co-ordinates of 1st parallel line

	int a11 = p2Line1.y - p1Line1.y;
	int a12 = p1Line1.x - p2Line1.x;
	int b1 = (a11*p1Line1.x + a12*p1Line1.y);

	int a21 = p2Line2.y - p1Line2.y;
	int a22 = p1Line2.x - p2Line2.x;
	int b2 = (a21*p1Line2.x + a22*p1Line2.y);

	double tmpX[2][1] = {  {0}, 
						   {0}};

	CvMat xx = cvMat(2, 1, CV_64FC1, tmpX);
	double tmpA[2][2] = {{a11, a12},
					  {a21, a22}};
	CvMat A = cvMat(2, 2, CV_64FC1,    tmpA);
	double tmpb[2][1] = {  {b1}, 
						{b2}};
	CvMat bb = cvMat(2,  1,  CV_64FC1, tmpb);
	cvSolve(&A,&bb,&xx,CV_LU);

	int x = tmpX[0][0];
	int y = tmpX[1][0];

	return cvPoint(x,y);
}
int Distance(CvPoint p1, CvPoint p2){
	return sqrt(float((p1.x- p2.x)*(p1.x-p2.x) + (p1.y- p2.y)*(p1.y-p2.y))); 
}

int main(void)
{
    int counter;
    char buff[33];
    string out = "out.avi";

    //read();
    // holds the frames got from the video
     cout<<"Enter the clip number 1, 2, 3, and 4 : ";
    cin>>counter;
    string filename = "clip";
    string name =  filename + itoa(counter, buff, 10) + ".avi";

	// Create an object that decodes the input video stream. 
    CvCapture *input_video = cvCaptureFromFile(name.c_str());
	if (input_video == NULL)
	{
		// Either the video didn't exist OR it uses a codec OpenCV doesn't support.
		fprintf(stderr, "Error: Can't open video.\n");
		return -1;
	}
	

	// Read the video's frame size out of the AVI.
	CvSize frame_size;
	frame_size.height =	(int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_HEIGHT );
	frame_size.width =	(int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_WIDTH );

    
    double fps = cvGetCaptureProperty(input_video, CV_CAP_PROP_FPS);
    CvVideoWriter *writer = cvCreateVideoWriter(out.c_str(), -1, fps/2, frame_size);


    // create one channel image of same size as captured frame
	IplImage * imgBinary = cvCreateImage(frame_size, 8, 1);
	
	// Determine the number of frames in the AVI.
    long number_of_frames;


	number_of_frames = (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_COUNT); //CV_CAP_PROP_POS_FRAMES );
	
    // Return to the beginning frame of the video
	cvSetCaptureProperty( input_video, CV_CAP_PROP_POS_FRAMES, 0. );
    

	long current_frame = 0;
	int nFrame=0;
	CvPoint pMidLanePoints[1000];
	int midLaneCount =0;

	CvPoint pRightLanePoints[1000];
	int rightLaneCount =0;

	CvPoint pLeftLanePoints[1000];
	int leftLaneCount =0;
	CvVideoWriter* pResult = NULL;

    
	while(true)
	{
        
		static IplImage *frame = NULL, *frame1 = NULL, *frame1_1C = NULL, *frame2_1C = NULL, *frame2 = NULL;
        static IplImage *eig_image = NULL, *temp_image = NULL, *pyramid1 = NULL, *pyramid2 = NULL;
        static IplImage *frame1_1Color = NULL, *frame2_1Color = NULL;
        //static IplImage *frame = NULL;

		cvSetCaptureProperty( input_video, CV_CAP_PROP_POS_FRAMES, current_frame);
        frame = cvQueryFrame(input_video); //cvLoadImage(name.c_str());

        //query a frame from the video
        if (!cvGrabFrame(input_video))
		{
			cout<<"We reached the end of the video"<<endl;
            system("pause");
			exit(0);
		}
		nFrame++;		

		allocateOnDemand(&frame1_1C, frame_size, IPL_DEPTH_8U, 1 );
		cvConvertImage(frame, frame1_1C);  //, CV_CVTIMG_FLIP

		allocateOnDemand( &frame1, frame_size, IPL_DEPTH_8U, 3 );
		cvConvertImage(frame, frame1);  //, CV_CVTIMG_FLIP


		// vanishing point
		int vPx, vPy;
		
		if (nFrame == 1)
        {
            FieldOfView(frame, vPx, vPy);  //frame2
        } 

        current_frame++;
        cvSetCaptureProperty( input_video, CV_CAP_PROP_POS_FRAMES, current_frame );
        frame = cvQueryFrame(input_video);
        allocateOnDemand( &frame2, frame_size, IPL_DEPTH_8U, 3 );
        cvConvertImage(frame, frame2);

		if (frame == NULL)
		{
			fprintf(stderr, "Error: Hmm. The end came sooner than we thought.\n");
			return -1;
		}
		allocateOnDemand( &frame2_1C, frame_size, IPL_DEPTH_8U, 1 );
		cvConvertImage(frame, frame2_1C);  //, CV_CVTIMG_FLIP

		/* Shi and Tomasi Feature Tracking! */
		allocateOnDemand( &eig_image, frame_size, IPL_DEPTH_32F, 1 );
		allocateOnDemand( &temp_image, frame_size, IPL_DEPTH_32F, 1 );

		/* Preparation: This array will contain the features found in frame 1. */
		CvPoint2D32f frame1_features[450];

		int number_of_features;
		number_of_features = 450;

        for(int kk = 0; kk < frame1_1C->width; kk++)
            for(int jj = 0; jj < frame1_1C->height; jj++)
                if(jj < 100 || jj > frame1_1C->height - 100)
                {
                    frame1_1C->imageData[jj*frame1_1C->widthStep + kk] = 0;
                    frame2_1C->imageData[jj*frame2_1C->widthStep + kk] = 0;
                }
        //canny edge
        cvCanny(frame2_1C, frame2_1C, 30,50);
        cvCanny(frame1_1C, frame1_1C, 30,50);

		cvGoodFeaturesToTrack(frame1_1C, eig_image, temp_image, frame1_features, &number_of_features, .01, .01, NULL);

		CvPoint2D32f frame2_features[450];
		char optical_flow_found_feature[450];
		float optical_flow_feature_error[450];
		CvSize optical_flow_window = cvSize(5,5);
		
		CvTermCriteria optical_flow_termination_criteria = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 30, .3 );

		allocateOnDemand( &pyramid1, frame_size, IPL_DEPTH_8U, 1 );
		allocateOnDemand( &pyramid2, frame_size, IPL_DEPTH_8U, 1 );

        // pyramidal lukas kanade optical flow
		cvCalcOpticalFlowPyrLK(frame1_1C, frame2_1C, pyramid1, pyramid2, frame1_features, frame2_features, number_of_features, optical_flow_window, 5, optical_flow_found_feature, optical_flow_feature_error, optical_flow_termination_criteria, 0 );
		
        //================================== Homography ========================================
        // Make an image of the results
        CvMat* src_points=cvCreateMat(2, number_of_features, CV_32FC1);
        CvMat* dst_points=cvCreateMat(2, number_of_features, CV_32FC1);
        CvMat* homography=cvCreateMat(3,3,CV_32FC1);

        for( int j=0; j<number_of_features; j++ )
        {
            
            CvPoint p0 = cvPoint( cvRound( frame1_features[j].x ), cvRound( frame1_features[j].y ) );
            CvPoint p1 = cvPoint( cvRound( frame2_features[j].x ), cvRound( frame2_features[j].y ) );

            cvmSet(src_points,0,j,cvRound(frame1_features[j].x));
            cvmSet(src_points,1,j,cvRound(frame1_features[j].y));

            cvmSet(dst_points,0,j,cvRound(frame2_features[j].x));
            cvmSet(dst_points,1,j,cvRound(frame2_features[j].y));
            //cvCircle(frame, cvPoint(frame2_features[j].x,frame2_features[j].y),2,cvScalar(0,255,255));
        }
          
        /*warp image1 to image2*/
        cvFindHomography(src_points,dst_points,homography,CV_RANSAC,7);

        for(int i = 0; i < number_of_features; i++)
		{
            CvMat *source = cvCreateMat(3,1,CV_32FC1);
            CvMat *destination = cvCreateMat(3,1,CV_32FC1);
            cvmSet(source,0,0,cvmGet(src_points,0,i));
            cvmSet(source,1,0,cvmGet(src_points,1,i));
            cvmSet(source,2,0,1.0);

            cvMatMul(homography, source, destination);
            if( sqrt(pow((cvmGet(destination,0,0)- cvmGet(dst_points,0,i)) , 2.0) + pow((cvmGet(destination,1,0)- cvmGet(dst_points,1,i)) , 2.0)) < 7)
            {
                optical_flow_found_feature[i] = 0;
            }
        }
        //======================================================================================

 		for(int i = 0; i < number_of_features; i++)
		{
			if ( optical_flow_found_feature[i] == 0 )	continue;

			int line_thickness = 1;
	
			CvScalar line_color = CV_RGB(255,0,0);
	
			CvPoint p,q;
			p.x = (int) frame1_features[i].x;
			p.y = (int) frame1_features[i].y;
			q.x = (int) frame2_features[i].x;
			q.y = (int) frame2_features[i].y;

			double angle, angle1;		
            angle = atan2((double) q.y - vPy, (double) q.x - vPx );
			angle1 = atan2((double) p.y - vPy, (double) p.x - vPx );

			double hypotenuse;	
            hypotenuse = sqrt( square(p.y - q.y) + square(p.x - q.x) );

			/* Here we lengthen the arrow by a factor of three. */
			q.x = (int) (p.x - 1 * hypotenuse * cos(angle));
			q.y = (int) (p.y - 1 * hypotenuse * sin(angle));

			/* Now we draw the main line of the arrow. */
			/* "frame1" is the frame to draw on.
			 * "p" is the point where the line begins.
			 * "q" is the point where the line stops.
			 * "CV_AA" means antialiased drawing.
			 * "0" means no fractional bits in the center cooridinate or radius.
			 */
			//cvLine( frame, p, q, line_color, line_thickness, CV_AA, 0 );
			/* Now draw the tips of the arrow.  I do some scaling so that the
			 * tips look proportional to the main line of the arrow.
			 */			
			p.x = (int) (q.x + 9 * cos(angle + pi / 4));
			p.y = (int) (q.y + 9 * sin(angle + pi / 4));
			cvLine( frame1, p, q, line_color, line_thickness, CV_AA, 0 );
			p.x = (int) (q.x + 9 * cos(angle - pi / 4));
			p.y = (int) (q.y + 9 * sin(angle - pi / 4));
			//cvLine( frame, p, q, line_color, line_thickness, CV_AA, 0 );

           

            //cvCircle(frame, cvPoint(frame1_features[i].x,frame1_features[i].y),2,cvScalar(0,0,255));
				
				if (abs(angle - angle1) > 0.01)
				{
					 // Midle Land
					 if ( Distance(p,cvPoint(0,0)) > Distance(FindIntersessionPoint(p,cvPoint(0,0),cvPoint(0,frame1->height), cvPoint(vPx, vPy)),cvPoint(0,0)) +5 && 
						Distance(p,cvPoint(frame1->width,0)) > Distance(FindIntersessionPoint(p,cvPoint(frame1->width,0),cvPoint(frame1->width,frame1->height), cvPoint(vPx, vPy)),cvPoint(frame1->width,0))+5)
					 {
						pMidLanePoints[midLaneCount].x = p.x;
						pMidLanePoints[midLaneCount].y = p.y;
						midLaneCount++;
						//cvDrawCircle(frame, p, 2,line_color);
					 }
					 // Right Lane
					 if ( Distance(p,cvPoint(frame1->width,0)) +10 < Distance(FindIntersessionPoint(p,cvPoint(frame1->width,0),cvPoint(frame1->width,frame1->height), cvPoint(vPx, vPy)),cvPoint(frame1->width,0)))
					 {
						pRightLanePoints[rightLaneCount].x = p.x;
						pRightLanePoints[rightLaneCount].y = p.y;
						rightLaneCount++;
						//cvDrawCircle(frame, p, 2,line_color);
					 }
					 // Left Lane
					 if ( Distance(p,cvPoint(0,0)) +10< Distance(FindIntersessionPoint(p,cvPoint(0,0),cvPoint(0,frame1->height), cvPoint(vPx, vPy)),cvPoint(0,0)))
					 {
						pLeftLanePoints[leftLaneCount].x = p.x;
						pLeftLanePoints[leftLaneCount].y = p.y;
						leftLaneCount++;
						//cvDrawCircle(frame, p, 2,line_color);
					 }
				}				
			//}	
        }
		// clustering
		
		if (nFrame %2==0)
        { 
 			if (midLaneCount > 3)
                clustering(frame, pMidLanePoints,1, midLaneCount, 1,frame);
			if (rightLaneCount > 3)
                clustering(frame, pRightLanePoints,2, rightLaneCount, 2,frame);
			if (leftLaneCount > 3)
                clustering(frame, pLeftLanePoints,3, leftLaneCount, 3,frame);
			midLaneCount =0;
			rightLaneCount =0;
			leftLaneCount =0;
 
            cvShowImage("Detection", frame);
            cvWriteFrame(writer, frame);
            cvWaitKey(15);
		}
	}
    cvReleaseVideoWriter(&writer);
    cvDestroyWindow("Detection");
}