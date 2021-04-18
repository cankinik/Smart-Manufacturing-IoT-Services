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

#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/public/session.h>

#include <time.h>
#include <thread> 

#include <algorithm>
#include "VideoTracker.h"
#include "param.h"

#include "FaceClassifier.h"
#include "HatDetector.h"
#include "ToolDetector.h"

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <sstream>


#include "functions.h"

using namespace std;
using namespace tensorflow;
using namespace cv;

//v4l2-ctl --all, v4l2-ctl -l, v4l2-ctl -c white_balance_temperature_auto=0, v4l2-ctl -d /dev/video2 --all, ls -ltrh /dev/video*

//Removed condition of objects having ID = 0 just for testing, it was in the if condition of displayFinalResults: localLeftIDs[i] == localRightIDs[i] vs localLeftIDs[i] == 0 && localRightIDs[i] == 0
//Recent changes: lowered precision of positions and changed the font, made displayFinalResults save local copies so that they are not changed in the process, added new function scaleROI to be used in maskSingleImage
//Things to think about: since deepsort requires 3 frames, tool update starts quite late (30 seconds since 3 x 10 seconds for three updates)
int main()	
{		
	//Variable declerations and initializations
	bool programNotTerminated = 1;
	const string configPath = "DeepSortFiles/config/deepsort_config.txt";
	Mat leftImage, rightImage, updatedLeftImage, updatedRightImage;  
	vector<Rect> leftROIs, rightROIs;
	vector<vector<vector<float>>> positionsInROIs;
	vector<vector<float>> finalPoints;
	vector<vector<float>> correctedFinalPoints;
	std::thread leftFeedThread, rightFeedThread, serverThread, toolLocationUpdaterThread;
	VideoCapture leftVideoFeed, rightVideoFeed;
	DeepSortParam params;
	params.read(configPath);
	VideoTracker leftDeepsortTracker(params);
	VideoTracker rightDeepsortTracker(params);	
	//Variables from displaying thread function
	Mat finalImage, floorPlan, tempFloorPlan;
	floorPlan = imread("EE102 PLAN.png");
	floorPlan.copyTo(tempFloorPlan);
	char userInputKey;	
	int imageWidth = floorPlan.size().width;
	int imageHeight = floorPlan.size().height;
	float x, y, z;
	const int leftExtent = 400;
	const int rightExtent = 400;
	const int upExtent = 1550;
	const int downExtent = 30; 	
	Mat tempFrame1, tempFrame2;
	
	//Start of the program
	serverThread = std::thread(serverThreadFunction);
	serverThread.detach();	
	initializeCameras(&leftVideoFeed, &rightVideoFeed);	
	// leftFeedThread = std::thread(continuallyUpdateUndistortedFrames, leftVideoFeed, &updatedLeftImage, 1, &programNotTerminated);
	// rightFeedThread = std::thread(continuallyUpdateUndistortedFrames, rightVideoFeed, &updatedRightImage, 0, &programNotTerminated);
	loadResults();	
	getSystemParameters();
	createUndistortionMapping();
	calculateRotationAndTranslation();
	usleep(1000000);	//Wait for 1 seconds so that the cameras open and the frames start being updated.
	toolLocationUpdaterThread = std::thread(toolLocationUpdater, &leftImage, &rightImage, &programNotTerminated);		
	while(programNotTerminated)
	{		   			 
		auto t1 = std::chrono::high_resolution_clock::now();
		floorPlan.copyTo(tempFloorPlan);
		
		leftVideoFeed >> tempFrame1;
		rightVideoFeed >> tempFrame2;
		leftImage = myUndistort(1, tempFrame1);
		rightImage = myUndistort(0, tempFrame2);

		// leftImage = updatedLeftImage;
		// rightImage = updatedRightImage;	
		
		leftROIs = leftDeepsortTracker.runFrame(leftImage);
		rightROIs = rightDeepsortTracker.runFrame(rightImage);		
			
		matchROIs(&leftROIs, &rightROIs);									//Also makes it so that the bounds of the ROIs fall inside the regions of the images.		
		if( leftROIs.size() != 0 && rightROIs.size() != 0 )
		{ 		
			try
			{
				positionsInROIs = findPositions(leftImage, rightImage, leftROIs, rightROIs);		
				finalPoints = extractFinalPositions(positionsInROIs);
				correctedFinalPoints = correctResultsForRotation(finalPoints);	

				alertForSocialDistancing(correctedFinalPoints, 150.0, 20);
				alertProhibitedAreaEntry(correctedFinalPoints);

				//Plotting location of workers
				for(int i = 0; i < correctedFinalPoints.size(); i++)
				{
					x = correctedFinalPoints[i][4];
					y = correctedFinalPoints[i][5];
					z = correctedFinalPoints[i][6];
					//Setting up image of camera feed
					circle(leftImage, Point2f(correctedFinalPoints[i][0], correctedFinalPoints[i][1]), 4, Scalar( 0, 0, 255 ), FILLED, LINE_8);
					putText(leftImage, to_string( (int)y ), Point2f(correctedFinalPoints[i][0], correctedFinalPoints[i][1]), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), 2);	//Change 5 to 6 if you are using the untilted version, 5 = y, 6 = z
					//Setting up image of layout view
					putText(tempFloorPlan, "o " + to_string( (int) z), Point2f( ( x + leftExtent ) / (leftExtent + rightExtent) * imageWidth, ( upExtent - y ) / (upExtent + downExtent) * imageHeight ), FONT_HERSHEY_SIMPLEX, 0.6, CV_RGB(120, 160, 0), 2);			
				}			
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}			
					
		}		
		auto t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		putText(leftImage, to_string( double(1000000.0/duration) ), Point2f(0, 40), FONT_HERSHEY_SIMPLEX, 1, CV_RGB(0, 0, 255), 3);
		imshow("Result", leftImage);
		imshow("Layout Plan", tempFloorPlan);
		userInputKey = waitKey(1);
		if ( userInputKey == 'q' )
		{
			programNotTerminated = 0;
			break;
		}		
	}
	// leftFeedThread.join();
	// rightFeedThread.join();	
	toolLocationUpdaterThread.join();	
	return 0;
}

