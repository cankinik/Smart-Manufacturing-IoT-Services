#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector> 
#include <thread> 
#include <algorithm>
using namespace std;
using namespace cv;

void getAndShowRegularFrame(VideoCapture videoFeed, string windowTitle, Mat *inputFrame)
{
	while(true)
	{
		videoFeed >> *inputFrame;
		imshow(windowTitle, *inputFrame);
		resizeWindow(windowTitle, 1280, 720);
		userInputKey = waitKey(1);
		if( userInputKey == 'q' )
		{
		    break;
		}
	}
	
}

int main()
{
	int leftFeedIndex = 1;
	int rightFeedIndex = 2;
	int photoIndex = 0;
	char userInputKey;
	int width = 1920;
	int height = 1080;    
    string leftFeedPhotoDirectory;
    string rightFeedPhotoDirectory;       
    Mat leftImage;
    Mat rightImage;   	 
    VideoCapture leftVideoFeed;
	VideoCapture rightVideoFeed;
	leftVideoFeed = VideoCapture(leftFeedIndex);
	rightVideoFeed = VideoCapture(rightFeedIndex);
	leftVideoFeed.set(CAP_PROP_FOURCC, 0x47504A4D);	//Using MJPG format rather than YUVY so that 30FPS 1080p is enabled
	rightVideoFeed.set(CAP_PROP_FOURCC, 0x47504A4D);
	leftVideoFeed.set(CAP_PROP_AUTOFOCUS, 0);		//Disabling autofocus so that the cameras will always see the same way
	rightVideoFeed.set(CAP_PROP_AUTOFOCUS, 0);
	leftVideoFeed.set(CAP_PROP_FRAME_WIDTH, width);
	leftVideoFeed.set(CAP_PROP_FRAME_HEIGHT, height);
	rightVideoFeed.set(CAP_PROP_FRAME_WIDTH, width);
	rightVideoFeed.set(CAP_PROP_FRAME_HEIGHT, height);
	std::thread leftFeedThread;
	std::thread rightFeedThread;
	namedWindow("Left Feed", 0);
	namedWindow("Right Feed", 0);	


	leftFeedThread = std::thread(getAndShowRegularFrame, leftVideoFeed, "Left Feed", &leftImage);
	rightFeedThread = std::thread(getAndShowRegularFrame, rightVideoFeed, "Right Feed", &rightImage);
	rightFeedThread.join();
	leftFeedThread.join();	
	// //Capturing synchronized images when 'c' is pressed until 'q' is pressed
    // while(true)
    // {   
    //     leftFeedThread = std::thread(getAndShowRegularFrame, leftVideoFeed, "Left Feed", &leftImage);
	// 	rightFeedThread = std::thread(getAndShowRegularFrame, rightVideoFeed, "Right Feed", &rightImage);
	// 	rightFeedThread.join();
	// 	leftFeedThread.join();	
	// 	// leftVideoFeed >> leftImage;
	// 	// rightVideoFeed >> rightImage;
	// 	// imshow("Left Feed", leftImage);
	// 	// resizeWindow("Left Feed", 1280, 720);
	// 	// imshow("Right Feed", rightImage);
	// 	// resizeWindow("Right Feed", 1280, 720);
		
        
    //     // userInputKey = waitKey(1);
    //     // if( userInputKey == 'c' )
    //     // {
    //     //     //Saving both frames when c was pressed to the appropriate directory.
    //     //     leftFeedPhotoDirectory = "Feed1Pictures/Pic" + to_string(photoIndex) + ".png";
    //     //     rightFeedPhotoDirectory = "Feed2Pictures/Pic" + to_string(photoIndex) + ".png";
    //     //     imwrite(leftFeedPhotoDirectory, leftImage);
    //     //     imwrite(rightFeedPhotoDirectory, rightImage);
    //     //     photoIndex++;
    //     // }
    //     // else if ( userInputKey == 'q' )
    //     // {
    //     //     break;
    //     // }
    // }
	// leftVideoFeed.release();
	// rightVideoFeed.release();
	return 0;
}
