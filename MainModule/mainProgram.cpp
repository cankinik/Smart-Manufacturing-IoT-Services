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
#include <time.h>
#include <thread> 
#include <algorithm>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <sstream>
#include "functions.h"
#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/public/session.h>
#include "VideoTracker.h"
#include "param.h"
#include "FaceClassifier.h"
#include "HatDetector.h"
#include "ToolDetector.h"

using namespace std;
using namespace tensorflow;
using namespace cv;

int main()	
{	//Variable declerations and initializations
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
	
	getSystemParameters();
	extern float leftExtent, rightExtent, upExtent, downExtent;	
	extern string layoutPictureDirectory;	
	Mat finalImage, floorPlan, tempFloorPlan;	
	floorPlan = imread(layoutPictureDirectory);
	floorPlan.copyTo(tempFloorPlan);
	char userInputKey;	
	int imageWidth = floorPlan.size().width;
	int imageHeight = floorPlan.size().height;
	float x, y, z;	
	Mat resultImage;	
	vector<vector<vector<float>>> lastFewCorrectedFinalPoints;	
	const int numberOfLastPointsKept = 20;

	Mat tempFrame1, tempFrame2;
	//Start of the program
	serverThread = std::thread(serverThreadFunction);
	serverThread.detach();	
	initializeCameras(&leftVideoFeed, &rightVideoFeed);	
	loadResults();		
	createUndistortionMapping();
	calculateRotationAndTranslation();
	// usleep(1000000);	//Wait for 1 seconds so that the cameras open and the frames start being updated.
	// toolLocationUpdaterThread = std::thread(toolLocationUpdater, &leftImage, &rightImage, &programNotTerminated);		
	while(programNotTerminated)
	{		   			 
		auto t1 = std::chrono::high_resolution_clock::now();
		floorPlan.copyTo(tempFloorPlan);
		leftVideoFeed >> tempFrame1;
		rightVideoFeed >> tempFrame2;
		leftImage = myUndistort(1, tempFrame1);
		rightImage = myUndistort(0, tempFrame2);
		leftROIs = leftDeepsortTracker.runFrame(leftImage);
		rightROIs = rightDeepsortTracker.runFrame(rightImage);		
		leftImage.copyTo(resultImage);	
		matchROIs(&leftROIs, &rightROIs);									//Also makes it so that the bounds of the ROIs fall inside the regions of the images.	
		if( leftROIs.size() != 0 && rightROIs.size() != 0 )
		{ 		
			positionsInROIs = findPositions(leftImage, rightImage, leftROIs, rightROIs);		
			finalPoints = extractFinalPositions(positionsInROIs);
			correctedFinalPoints = correctResultsForRotation(finalPoints);	

			alertForSocialDistancing(correctedFinalPoints, 150.0, 20);	//150cm is distance treshold, 20 frames is the exposure treshold
			alertProhibitedAreaEntry(correctedFinalPoints);

			//Plotting location of workers			
			for(int i = 0; i < correctedFinalPoints.size(); i++)
			{				
				x = correctedFinalPoints[i][4];
				y = correctedFinalPoints[i][5];
				z = correctedFinalPoints[i][6];
				//Setting up image of camera feed				
				rectangle(resultImage, leftROIs[i], Scalar( 0, 0, 255 ), 2);
				circle(resultImage, Point2f(correctedFinalPoints[i][0], correctedFinalPoints[i][1]), 4, Scalar( 0, 0, 255 ), FILLED, LINE_8);
				putText(resultImage, to_string( (int)y ), Point2f(correctedFinalPoints[i][0], correctedFinalPoints[i][1]), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), 2);
				//Setting up image of layout view
				putText(tempFloorPlan, "o", Point2f( ( x + leftExtent ) / (leftExtent + rightExtent) * imageWidth, ( upExtent - y ) / (upExtent + downExtent) * imageHeight ), FONT_HERSHEY_SIMPLEX, 0.8, CV_RGB(120, 160, 0), 4);			
			}		

			//Plotting the position of workers in the last few frames with more emphasis on recent positions. This is completely optional and is only implemented for visual purposes
			lastFewCorrectedFinalPoints.push_back(correctedFinalPoints);
			if (lastFewCorrectedFinalPoints.size() == numberOfLastPointsKept)
			{
				lastFewCorrectedFinalPoints.erase(lastFewCorrectedFinalPoints.begin() + 0);	//FIFO		
			}
			for (int j = 0; j < lastFewCorrectedFinalPoints.size(); j++)
			{
				for(int i = 0; i < lastFewCorrectedFinalPoints[j].size(); i++)
				{				
					x = lastFewCorrectedFinalPoints[j][i][4];
					y = lastFewCorrectedFinalPoints[j][i][5];
					putText(tempFloorPlan, "o", Point2f( ( x + leftExtent ) / (leftExtent + rightExtent) * imageWidth, ( upExtent - y ) / (upExtent + downExtent) * imageHeight ), FONT_HERSHEY_SIMPLEX, (0.6 * j / numberOfLastPointsKept), CV_RGB(120, 160, 0), (3 * j / numberOfLastPointsKept));			
				}	
			}

		}		
		auto t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
		putText(resultImage, to_string( double(1000000.0/duration) ), Point2f(0, 40), FONT_HERSHEY_SIMPLEX, 1, CV_RGB(0, 0, 255), 3);
		imshow("Result", resultImage);
		imshow("Layout Plan", tempFloorPlan);
		userInputKey = waitKey(1);
		if ( userInputKey == 'q' )
		{
			programNotTerminated = 0;
			break;
		}		
	}
	// toolLocationUpdaterThread.join();	
	return 0;
}

