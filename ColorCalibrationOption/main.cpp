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
	videoFeed >> *inputFrame;
	imshow(windowTitle, *inputFrame);
}

int main()
{
	//Camera parameters that have been found to make a difference
	//	CAP_PROP_FRAME_WIDTH	
	//	CAP_PROP_FRAME_HEIGHT
	//	CAP_PROP_BRIGHTNESS
	//	CAP_PROP_CONTRAST
	//	CAP_PROP_SATURATION
	// 	CAP_PROP_HUE
	//	CAP_PROP_GAMMA
	//	CAP_PROP_BACKLIGHT
	int width = 1920;
	int height = 1080;
	int leftFeedIndex = 1;
	int rightFeedIndex = 2;
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
	
	char userInputKey;
	Mat leftImage;
    Mat rightImage;   	 
	std::thread leftFeedThread;
	std::thread rightFeedThread;
	int inputValue;

	cout << "Please use the buttons: b, c, s, h, and g to change Brightness, Contrast, Saturation, Hue, and Gamma." << endl;
	cout << "After selecting the parameter, please enter the value you would like to set to the left camera. Press q to quit" << endl;
	while(true)
	{
		leftFeedThread = std::thread(getAndShowRegularFrame, leftVideoFeed, "Left Feed", &leftImage);
		rightFeedThread = std::thread(getAndShowRegularFrame, rightVideoFeed, "Right Feed", &rightImage);
		rightFeedThread.join();
		leftFeedThread.join();	
		userInputKey = waitKey(1);
        if ( userInputKey == 'q' )
        {
            break;
        }	
		switch ( userInputKey ) 
		{
			case 'b': 
				cout << "Please enter a value in the range: [-64, 64], default is 0, current value is " << to_string( leftVideoFeed.get(CAP_PROP_BRIGHTNESS) ) << endl;
				cin >> inputValue;
				if ( (inputValue > -64) && (inputValue < 64) )
				{
					leftVideoFeed.set(CAP_PROP_BRIGHTNESS, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
			case 'c': 
				cout << "Please enter a value in the range: [0, 64], default is 32, current value is " << to_string( leftVideoFeed.get(CAP_PROP_CONTRAST) ) << endl;
				cin >> inputValue;
				if ( (inputValue > 0) && (inputValue < 64) )
				{
					leftVideoFeed.set(CAP_PROP_CONTRAST, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
			case 's': 
				cout << "Please enter a value in the range: [0, 128], default is 64, current value is " << to_string( leftVideoFeed.get(CAP_PROP_SATURATION) ) << endl;
				cin >> inputValue;
				if ( (inputValue > 0) && (inputValue < 128) )
				{
					leftVideoFeed.set(CAP_PROP_SATURATION, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
			case 'h': 
				cout << "Please enter a value in the range: [-40, 40], default is 0, current value is " << to_string( leftVideoFeed.get(CAP_PROP_HUE) ) << endl;
				cin >> inputValue;
				if ( (inputValue > -40) && (inputValue < 40) )
				{
					leftVideoFeed.set(CAP_PROP_HUE, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
			case 'g': 
				cout << "Please enter a value in the range: [72, 500], default is 100, current value is " << to_string( leftVideoFeed.get(CAP_PROP_GAMMA) ) << endl;
				cin >> inputValue;
				if ( (inputValue > 72) && (inputValue < 500) )
				{
					leftVideoFeed.set(CAP_PROP_GAMMA, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
		}
	}
	return 0;
}
