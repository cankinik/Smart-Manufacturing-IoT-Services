#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector> 
#include <unistd.h>

#include "opencv2/core.hpp"
#ifdef HAVE_OPENCV_XFEATURES2D
#include "opencv2/highgui.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <time.h>
#include <thread> 

#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/public/session.h>

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
#include <ctime>

using namespace cv::xfeatures2d;
using namespace cv;
using namespace std;

//NOTE: Both stereo calibration and rotation calibration use {6, 9}, rather than {9, 6}, you might want to try calibrating while holding the paper vertically so that the dimensions match.

//Function Declerations
void stereoCalibration();
void takePictures(int startIndex);
void loadResults();
vector < vector<Point2f> > flannPoints(Mat leftFrame, Mat rightFrame, int minimumPoint);
void createUndistortionMapping();
Mat myUndistort(bool isLeft, Mat originalFrame);
Mat maskedImage(Mat inputImage, vector<Rect> ROIs);
Mat maskSingleImage(Mat inputImage, Rect ROI);
vector<vector<vector<float>>> findPositions(Mat inputLeftImage, Mat inputRightImage, vector<Rect> leftROIs, vector<Rect> rightROIs);
void initializeCameras(VideoCapture *leftVideoFeed, VideoCapture *rightVideoFeed);
void getAndShowRegularFrame(VideoCapture videoFeed, string windowTitle, Mat *inputFrame);
void continuallyUpdateUndistortedFrames(VideoCapture videoFeed, Mat *inputFrame, bool isLeft, bool *programNotTerminated);
void colorCalibrate(VideoCapture *leftVideoFeed, VideoCapture *rightVideoFeed);
vector<vector<float>> extractFinalPositions(vector<vector<vector<float>>> allFoundPoints);
void calculateRotationAndTranslation();
vector<vector<float>> correctResultsForRotation(vector<vector<float>> resultsToBeCorrected);
void serverThreadFunction();
void toolLocationUpdater(Mat *updatedLeftImage, Mat *updatedRightImage, bool *programNotTerminated);
Rect scaleROI(Rect ROI);
void matchROIs(vector<Rect> *leftROIs, vector<Rect> *rightROIs);
void matchIDROIs(vector<Rect> *leftROIs, vector<Rect> *rightROIs, vector<int> *leftFeedObjectIDVector, vector<int> *rightFeedObjectIDVector);
int alertForSocialDistancing(vector<vector<float>> correctedFinalPoints, float socialDistancingTreshold);


//Global Constants
const double squareSize = 0.038; //0.029 for A4, for cagan it is 0.034
const int width = 1920; 
const int height = 1080; 
const Size imageSize = Size(1920, 1080); 
const int leftFeedIndex = 1;	//This is also referred to as feed 1
const int rightFeedIndex = 2;	//This is also referred to as feed 2
int leftExtent = 400;
int rightExtent = 400;
int upExtent = 1550;
int downExtent = 30; 
const int ROIScalingTreshold = 200;

//Global Variables
Mat cameraMatrix1, distCoeffs1, R1, T1;
Mat cameraMatrix2, distCoeffs2, R2, T2;
Mat R, F, E;
Vec3d T;
Mat Rot1, Rot2, P1, P2, Q;
Mat leftMapX, leftMapY, rightMapX, rightMapY;
Mat correctingRotationalMatrix, correctingTranslationalVector;

void calculateRotationAndTranslation()
{
	Mat checkerboardImage = imread("Pic100.png");
	int CHECKERBOARD[2]{6, 9}; 
    vector<Point3f> objectPoints;
	vector<Point2f> cornerPoints;
	TermCriteria criteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 30, 0.001);
	Mat grayImage;
	Mat correctingRotationVector;
    
    for(int i = 0; i < CHECKERBOARD[1]; i++)
    {
        for(int j = 0; j < CHECKERBOARD[0]; j++) 
        {
            objectPoints.push_back(Point3f(j,i,0) * squareSize);
        }
    }
    
    cvtColor(checkerboardImage, grayImage, COLOR_BGR2GRAY);
    if( findChessboardCorners(grayImage, Size(CHECKERBOARD[0],CHECKERBOARD[1]), cornerPoints, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE) )
    {
    	cornerSubPix(grayImage, cornerPoints, Size(11,11), Size(-1,-1), criteria);
    	solvePnP(objectPoints, cornerPoints, cameraMatrix1, distCoeffs1, correctingRotationVector, correctingTranslationalVector, false);
    	Rodrigues(correctingRotationVector, correctingRotationalMatrix); 
    	//These steps are switching the columns, they will probably not be necessary when we calibrate properly in terms of horizontal or landscape of the checkerboard being compliant with the dimensions
    	double temp;
    	temp = correctingRotationalMatrix.at<double>(0, 0);
    	correctingRotationalMatrix.at<double>(0, 0) = correctingRotationalMatrix.at<double>(0, 1);
    	correctingRotationalMatrix.at<double>(0, 1) = temp;
    	temp = correctingRotationalMatrix.at<double>(1, 0);
    	correctingRotationalMatrix.at<double>(1, 0) = correctingRotationalMatrix.at<double>(1, 1);
    	correctingRotationalMatrix.at<double>(1, 1) = temp;
    	temp = correctingRotationalMatrix.at<double>(2, 0);
    	correctingRotationalMatrix.at<double>(2, 0) = correctingRotationalMatrix.at<double>(2, 1);
    	correctingRotationalMatrix.at<double>(2, 1) = temp;
	}
    else
    {
    	cout << "Unable to find the checkerboard in the image, please use another image to calibrate the rotation and translation matrices." << endl;
    }
}

vector<vector<float>> correctResultsForRotation(vector<vector<float>> resultsToBeCorrected)
{
	float x, y, z, row1, row2, row3, shiftingX, shiftingY, shiftingZ;
	vector<vector<float>> result = resultsToBeCorrected;
	//rotated = ( rotation_matrix.T @ ( point - translation_matrix ) ) + (rotation_matrix @ translation_matrix) * np.array([[1],[1],[0]])
	for(int i = 0; i < resultsToBeCorrected.size(); i++)
	{
		x = resultsToBeCorrected[i][4] - correctingTranslationalVector.at<double>(0, 0);
		y = resultsToBeCorrected[i][5] - correctingTranslationalVector.at<double>(0, 1);
		z = resultsToBeCorrected[i][6] - correctingTranslationalVector.at<double>(0, 2);
		//Subtracted translation vector at this point, now we should do cross product with the transpose of the rotational matrix
		row1 = x * correctingRotationalMatrix.at<double>(0, 0) + y * correctingRotationalMatrix.at<double>(1, 0) + z * correctingRotationalMatrix.at<double>(2, 0);
		row2 = x * correctingRotationalMatrix.at<double>(0, 1) + y * correctingRotationalMatrix.at<double>(1, 1) + z * correctingRotationalMatrix.at<double>(2, 1);
		row3 = x * correctingRotationalMatrix.at<double>(0, 2) + y * correctingRotationalMatrix.at<double>(1, 2) + z * correctingRotationalMatrix.at<double>(2, 2);
		//Have ( rotation_matrix.T @ ( point - translation_matrix ) ) at this point, now we need to add (rotation_matrix @ translation_matrix) * np.array([[1],[1],[0]])
		shiftingX = correctingTranslationalVector.at<double>(0, 0) * correctingRotationalMatrix.at<double>(0, 0) + correctingTranslationalVector.at<double>(0, 1) * correctingRotationalMatrix.at<double>(0, 1) + correctingTranslationalVector.at<double>(0, 2) * correctingRotationalMatrix.at<double>(0, 2);
		shiftingY = correctingTranslationalVector.at<double>(0, 0) * correctingRotationalMatrix.at<double>(1, 0) + correctingTranslationalVector.at<double>(0, 1) * correctingRotationalMatrix.at<double>(1, 1) + correctingTranslationalVector.at<double>(0, 2) * correctingRotationalMatrix.at<double>(1, 2);
		shiftingZ = correctingTranslationalVector.at<double>(0, 0) * correctingRotationalMatrix.at<double>(2, 0) + correctingTranslationalVector.at<double>(0, 1) * correctingRotationalMatrix.at<double>(2, 1) + correctingTranslationalVector.at<double>(0, 2) * correctingRotationalMatrix.at<double>(2, 2);
		x = row1 + shiftingX;
		y = row2 + shiftingY;
		z = row3;	//We will be displaying the Z coordinate with respect to the floor
		result[i][4] = x * 100;
		result[i][5] = y * 100;
		result[i][6] = z * -100;
		
	}
	return result;
}

//Initiralizes the VideoCapture variables and sets the correct dimension for them using the constants width and height
void initializeCameras(VideoCapture *leftVideoFeed, VideoCapture *rightVideoFeed)
{
	// *leftVideoFeed = VideoCapture(leftFeedIndex);
	// *rightVideoFeed = VideoCapture(rightFeedIndex);
	*leftVideoFeed = VideoCapture("/home/cankinik/Desktop/left.avi");
	*rightVideoFeed = VideoCapture("/home/cankinik/Desktop/right.avi");	
	(*leftVideoFeed).set(CAP_PROP_FOURCC, 0x47504A4D);	//Using MJPG format rather than YUVY so that 30FPS 1080p is enabled
	(*rightVideoFeed).set(CAP_PROP_FOURCC, 0x47504A4D);
	(*leftVideoFeed).set(CAP_PROP_AUTOFOCUS, 0);		//Disabling autofocus so that the cameras will always see the same way
	(*rightVideoFeed).set(CAP_PROP_AUTOFOCUS, 0);
	(*leftVideoFeed).set(CAP_PROP_FRAME_WIDTH, width);
	(*leftVideoFeed).set(CAP_PROP_FRAME_HEIGHT, height);
	(*rightVideoFeed).set(CAP_PROP_FRAME_WIDTH, width);
	(*rightVideoFeed).set(CAP_PROP_FRAME_HEIGHT, height);
}

//Function to be executed on a separate thread, updates the undistorted frame to be processed by the algorithms.
void continuallyUpdateUndistortedFrames(VideoCapture videoFeed, Mat *inputFrame, bool isLeft, bool *programNotTerminated)
{	
	Mat tempFrame;
	while( (*programNotTerminated) )
	{
		videoFeed >> tempFrame;
		*inputFrame = myUndistort(isLeft, tempFrame);
		this_thread::sleep_for( chrono::milliseconds(35) );	//Camera input is around 25 FPS with the delays, which is the fastest update.
	}	
}

//Calibrates the color of the cameras so that they are as close as possible. Configured by the user adjusting parameters and comparing the two feeds.
void colorCalibrate(VideoCapture *leftVideoFeed, VideoCapture *rightVideoFeed)
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
	char userInputKey;
	Mat leftImage;
    Mat rightImage;   	 
	std::thread leftFeedThread;
	std::thread rightFeedThread;
	int inputValue;

	initializeCameras(leftVideoFeed, rightVideoFeed);
	cout << "Please use the buttons: b, c, s, h, and g to change Brightness, Contrast, Saturation, Hue, and Gamma." << endl;
	cout << "After selecting the parameter, please enter the value you would like to set to the left camera. Press q to quit" << endl;
	while(true)
	{
		leftFeedThread = std::thread(getAndShowRegularFrame, *leftVideoFeed, "Left Feed", &leftImage);
		rightFeedThread = std::thread(getAndShowRegularFrame, *rightVideoFeed, "Right Feed", &rightImage);
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
				cout << "Please enter a value in the range: [-64, 64], default is 0, current value is " << to_string( (*leftVideoFeed).get(CAP_PROP_BRIGHTNESS) ) << endl;
				cin >> inputValue;
				if ( (inputValue > -64) && (inputValue < 64) )
				{
					(*leftVideoFeed).set(CAP_PROP_BRIGHTNESS, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
			case 'c': 
				cout << "Please enter a value in the range: [0, 64], default is 32, current value is " << to_string( (*leftVideoFeed).get(CAP_PROP_CONTRAST) ) << endl;
				cin >> inputValue;
				if ( (inputValue > 0) && (inputValue < 64) )
				{
					(*leftVideoFeed).set(CAP_PROP_CONTRAST, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
			case 's': 
				cout << "Please enter a value in the range: [0, 128], default is 64, current value is " << to_string( (*leftVideoFeed).get(CAP_PROP_SATURATION) ) << endl;
				cin >> inputValue;
				if ( (inputValue > 0) && (inputValue < 128) )
				{
					(*leftVideoFeed).set(CAP_PROP_SATURATION, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
			case 'h': 
				cout << "Please enter a value in the range: [-40, 40], default is 0, current value is " << to_string( (*leftVideoFeed).get(CAP_PROP_HUE) ) << endl;
				cin >> inputValue;
				if ( (inputValue > -40) && (inputValue < 40) )
				{
					(*leftVideoFeed).set(CAP_PROP_HUE, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
			case 'g': 
				cout << "Please enter a value in the range: [72, 500], default is 100, current value is " << to_string( (*leftVideoFeed).get(CAP_PROP_GAMMA) ) << endl;
				cin >> inputValue;
				if ( (inputValue > 72) && (inputValue < 500) )
				{
					(*leftVideoFeed).set(CAP_PROP_GAMMA, inputValue);
				}
				else
				{
					cout << "Entered value is not in range!" << endl;
				}
				break;
		}
	}
	
}

void getAndShowRegularFrame(VideoCapture videoFeed, string windowTitle, Mat *inputFrame)
{
	videoFeed >> *inputFrame;
	imshow(windowTitle, *inputFrame);
}

//Uses the checkerboard images to find coefficients for undistortion, saves results in CalibrationResults.yml
void stereoCalibration()
{
    int CHECKERBOARD[2]{6,9}; 

    vector<vector<Point3f> > objpoints1;
    vector<vector<Point2f> > imgpoints1;
    vector<Point3f> objp1;

    vector<vector<Point3f> > objpoints2;
    vector<vector<Point2f> > imgpoints2;
    vector<Point3f> objp2;

    for(int i = 0; i < CHECKERBOARD[1]; i++)
    {
        for(int j = 0; j < CHECKERBOARD[0]; j++) 
        {
            objp1.push_back(Point3f(j,i,0) * squareSize);
            objp2.push_back(Point3f(j,i,0) * squareSize);
        }
    }

    // Extracting path of individual image stored in a given directory
    vector<String> images1;
    vector<String> images2;

    string path1 = "./Feed1Pictures/*.png";
    string path2 = "./Feed2Pictures/*.png";

    glob(path1, images1);
    glob(path2, images2);
    
    Mat frame1, frame2, gray1, gray2;
    // vector to store the pixel coordinates of detected checker board corners 
    vector<Point2f> corner_pts1;
    vector<Point2f> corner_pts2;
    bool success1;
    bool success2;

    // Looping over all the images in the directory
    for(int i = 0; i < images1.size(); i++)
    {
        frame1 = imread(images1[i]);
        frame2 = imread(images2[i]);

        cvtColor(frame1, gray1, COLOR_BGR2GRAY);
        cvtColor(frame2, gray2, COLOR_BGR2GRAY);
        
        // If desired number of corners are found in the image then success = true  
        success1 = findChessboardCorners(gray1, Size(CHECKERBOARD[0],CHECKERBOARD[1]), corner_pts1, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
        success2 = findChessboardCorners(gray2, Size(CHECKERBOARD[0],CHECKERBOARD[1]), corner_pts2, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
        
        if(success1 & success2) //Both need to be successful if we want to count them in image points.
        {
            TermCriteria criteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 30, 0.001);
            
            // refining pixel coordinates for given 2d points.
            cornerSubPix(gray1, corner_pts1, Size(11,11), Size(-1,-1),criteria);
            cornerSubPix(gray2, corner_pts2, Size(11,11), Size(-1,-1),criteria);  
                        
            objpoints1.push_back(objp1);
            objpoints2.push_back(objp2);
            
            imgpoints1.push_back(corner_pts1);
            imgpoints2.push_back(corner_pts2);
        }
        else
        {
            cout << "Discarding image " << i << endl;
        }
    }
   
    //Calibrating cameras for both stereo and individually
    double ret1 = calibrateCamera(objpoints1, imgpoints1, imageSize, cameraMatrix1, distCoeffs1, R1, T1);
    cout << "Camera 1 RMS: " << ret1 << endl;
    double ret2 = calibrateCamera(objpoints2, imgpoints2, imageSize, cameraMatrix2, distCoeffs2, R2, T2);
    cout << "Camera 2 RMS: " << ret2 << endl;    

    double ret3 = stereoCalibrate(objpoints1, imgpoints1, imgpoints2, cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2, imageSize, R, T, E, F);
    cout << "Stereo RMS: " << ret3 << endl; 

    stereoRectify(cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2, imageSize, R, T, Rot1, Rot2, P1, P2, Q, 0, -1);


    //Storing results in the .yml file
    FileStorage file("CalibrationResults.yml", FileStorage::WRITE);

    file << "cameraMatrix1" << cameraMatrix1;
    file << "distCoeffs1" << distCoeffs1;
    file << "cameraMatrix2" << cameraMatrix2;
    file << "distCoeffs2" << distCoeffs2;
    file << "R" << R;
    file << "F" << F;
    file << "E" << E;
    file << "T" << T;
    file << "Rot1" << Rot1;
    file << "Rot2" << Rot2;
    file << "P1" << P1;
    file << "P2" << P2;
    file << "Q" << Q;

    file.release();
}

//Loads the coefficient and parameter results from the .yml file
void loadResults()
{
	//These are correct for our system, but currently we are testing using the 1080 calibration yml file done in python
	
    FileStorage file("/home/cankinik/Desktop/QTProject/CameraCalibratingOption/CalibrationResults.yml", FileStorage::READ);	//"CalibrationResults.yml" ///home/cankinik/Desktop/QTProject/CameraCalibratingOption/CalibrationResults.yml

    file["cameraMatrix1"] >> cameraMatrix1;
    file["distCoeffs1"] >> distCoeffs1;
    file["cameraMatrix2"] >> cameraMatrix2;
    file["distCoeffs1"] >> distCoeffs2;
    file["R"] >> R;
    file["F"] >> F;
    file["E"] >> E;
    file["T"] >> T;
    file["Rot1"] >> Rot1;
    file["Rot2"] >> Rot2;
    file["P1"] >> P1;
    file["P2"] >> P2;
    file["Q"] >> Q;

    file.release();
}

//Generates undistortion maps for both using the previously generated coefficients.
void createUndistortionMapping()
{
    initUndistortRectifyMap(cameraMatrix1, distCoeffs1, Rot1, P1, imageSize, CV_32FC1, leftMapX, leftMapY);
    initUndistortRectifyMap(cameraMatrix2, distCoeffs2, Rot2, P2, imageSize, CV_32FC1, rightMapX, rightMapY);
}

//Undistorts the given image using the undistortion of left or right, bool parameter is 1 if left image.
Mat myUndistort(bool isLeft, Mat originalFrame)
{
    Mat resultingFrame;
    if (isLeft)
    {
        remap(originalFrame, resultingFrame, leftMapX, leftMapY, INTER_LINEAR, BORDER_CONSTANT);
    }
    else
    {
        remap(originalFrame, resultingFrame, rightMapX, rightMapY, INTER_LINEAR, BORDER_CONSTANT);
    }
    return resultingFrame;
}

//The video feed is shown for the user to see what will be captured, c for capture, q for quit
//Saves pictures at Feed1Pictures/Pic#.png, where # is the index of the picture taken.
void takePictures(int startIndex)
{   
    char userInputKey;
    int photoIndex = startIndex;
    string leftFeedPhotoDirectory;
    string rightFeedPhotoDirectory;       
    Mat leftImage;
    Mat rightImage;   	 
    VideoCapture leftVideoFeed;
	VideoCapture rightVideoFeed;
	std::thread leftFeedThread;
	std::thread rightFeedThread;

	initializeCameras(&leftVideoFeed, &rightVideoFeed);

	//Capturing synchronized images when 'c' is pressed until 'q' is pressed
    while(true)
    {   
        leftFeedThread = std::thread(getAndShowRegularFrame, leftVideoFeed, "Left Feed", &leftImage);
		rightFeedThread = std::thread(getAndShowRegularFrame, rightVideoFeed, "Right Feed", &rightImage);
		rightFeedThread.join();
		leftFeedThread.join();	
		
        
        userInputKey = waitKey(1);
        if( userInputKey == 'c' )
        {
            //Saving both frames when c was pressed to the appropriate directory.
            leftFeedPhotoDirectory = "Feed1Pictures/Pic" + to_string(photoIndex) + ".png";
            rightFeedPhotoDirectory = "Feed2Pictures/Pic" + to_string(photoIndex) + ".png";
            imwrite(leftFeedPhotoDirectory, leftImage);
            imwrite(rightFeedPhotoDirectory, rightImage);
            photoIndex++;
        }
        else if ( userInputKey == 'q' )
        {
            break;
        }
    }
	leftVideoFeed.release();
	rightVideoFeed.release();
}

//Takes the regions of interest as rectangles, then masks all except the given ROIs. NOTE: Parameters in the given rect are: (x0, y0, width, height)!!!!
Mat maskedImage(Mat inputImage, vector<Rect> ROIs)
{
    Mat masker;
    inputImage.copyTo(masker);
    rectangle(masker, Rect(0, 0, width, height), Scalar(0,0,0),-1);
    for(int i = 0; i < ROIs.size(); i++ )
    {
        rectangle(masker, ROIs[i], Scalar(1,1,1),-1);
    }
    return masker.mul(inputImage);
}

//Creates a masked image with a single ROI
Mat maskSingleImage(Mat inputImage, Rect ROI)
{
	Mat masker;
	Rect scaledROI;
    inputImage.copyTo(masker);
    rectangle(masker, Rect(0, 0, width, height), Scalar(0,0,0),-1);
	//rectangle(masker, ROI, Scalar(1,1,1),-1);		//This is for when you don't want to scale the ROI
	rectangle(masker, scaleROI(ROI), Scalar(1,1,1),-1);
    return masker.mul(inputImage);	
}

Rect scaleROI(Rect ROI)
{
	int width = ROI.width;
	int height = ROI.height;
	//Change width according to the scaling treshold
	if(width < ROIScalingTreshold)
	{
		//Don't change size of it is already less than the treshold
	}
	else if(width < ROIScalingTreshold*2)
	{
		width = ROIScalingTreshold;	//If half of the size is less than the treshold, then we will use the treshold
	}
	else
	{
		width = (int)(width/2);			//If the size is quite big, then we will use half of the size
	}
	//Repeat same operations for height
	if(height < ROIScalingTreshold)
	{
		//Don't change size of it is already less than the treshold
	}
	else if(height < ROIScalingTreshold*2)
	{
		height = ROIScalingTreshold;	//If half of the size is less than the treshold, then we will use the treshold
	}
	else
	{
		height = (int)(height/2);			//If the size is quite big, then we will use half of the size
	}
	//Output the final scaled ROI
	return Rect(ROI.x + ROI.width/2 - width/2, ROI.y + ROI.height/2 - height/2, width, height);
}

//Tries to find at least "minimumPoint" matches with the strictest treshold possible. If not able to find up to 1.0f, then no result. 
//Return vector of vectors. The two vectors are the Point2f type positions of the matched points. The first one is for the left image.
vector <vector<Point2f>> flannPoints(Mat leftFrame, Mat rightFrame, int minimumPoint)
{		
	int minHessian = 400;
	float tresholdRatio = 0.08f;
	vector<DMatch> good_matches;
	vector<Point2f> points1;
	vector<Point2f> points2;
	vector < vector<Point2f> > result;
	//Creating the detectors and matchers, and filling the keypoints through matches
	Ptr<SURF> detector = SURF::create( minHessian );
	vector<KeyPoint> keypoints1, keypoints2;
	Mat descriptors1, descriptors2;
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
	vector< vector<DMatch> > knn_matches;
    detector->detectAndCompute( leftFrame, noArray(), keypoints1, descriptors1 );
    detector->detectAndCompute( rightFrame, noArray(), keypoints2, descriptors2 );	
	if(keypoints1.size() >= 2 && keypoints2.size() >= 2)	//https://stackoverflow.com/questions/25089393/opencv-flannbasedmatcher
	{
		matcher->knnMatch( descriptors1, descriptors2, knn_matches, 2 );
		//Once all matches are made, we pick only the best matches. Treshold goes from 0.1 to 1.0 until minimumPoint are found
		do
		{
			tresholdRatio += 0.02f;
			points1.clear();
			points2.clear();
			for (size_t i = 0; i < knn_matches.size(); i++)
			{
				if (knn_matches[i][0].distance < tresholdRatio * knn_matches[i][1].distance)
				{
				    points1.push_back(keypoints1[knn_matches[i][0].queryIdx].pt);				    
				    points2.push_back(keypoints2[knn_matches[i][0].trainIdx].pt);
				}        
			}

		}while((points1.size() < minimumPoint ) && (tresholdRatio <= 1.0f));
		
		//If the treshold is not too bad, then the found points are passed as the result, otherwise an empty vector of vectors is returned
		
		if (tresholdRatio < 1.0f)
		{
			result.push_back(points1);
			result.push_back(points2);
		} 
		else
		{
			cout << "Couldn't find points out of the key points" << endl;
		}   
		//Note: might also return the treshold to evaluate confidence in the future 
	}
	else
	{
		cout << "Unable to detect any key-points in the frame!" << endl;
	}    	
    return result;
}

//Gets each ROI, masks each of them, passes to flann, extracts points (10), finds positions, returns a vector for each point in each region, representing their position in both images and also in the real world. Points with negative z components are eliminated.
vector<vector<vector<float>>> findPositions(Mat inputLeftImage, Mat inputRightImage, vector<Rect> leftROIs, vector<Rect> rightROIs)
{
	vector<vector<vector<Point2f>>> matchedPoints;
	vector<vector<Point2f>> flannPointsofOneObject;
	Mat temp3DPositions;
	float temX, tempY, tempZ;
	vector<vector<float>> emptyVectorOfVector;
	vector<vector<vector<float>>> result;

	for(int i = 0; i < leftROIs.size(); i++)
	{
		flannPointsofOneObject = flannPoints( inputLeftImage(leftROIs[i]), inputRightImage(rightROIs[i]), 10 );
		if(flannPointsofOneObject.size() == 2)	//Flann points were found for this ROI (Since size == 2), now we can find the positions from the matched points inside the region
		{			
			//Correcting for the coordinate change due to extracting the ROI out of the frame
			for(int j = 0; j < flannPointsofOneObject[0].size(); j++)
			{
				flannPointsofOneObject[0][j] += Point2f(leftROIs[i].x, leftROIs[i].y);
				flannPointsofOneObject[1][j] += Point2f(rightROIs[i].x, rightROIs[i].y);
			}
			matchedPoints.push_back( flannPointsofOneObject );
		}
	}

	for(int i = 0; i < matchedPoints.size(); i++)	//i is the index of the ROI for all operations inside this for-loop
	{
		result.push_back(emptyVectorOfVector);	//Adding an empty vector of vector for each ROI into the result, will be filled with the points accordingly later on.
		//Used to mask all ROIs for left and right, and find the flann points for each ROI, then push the found points for that ROI into matchedPoints
		// matchedPoints.push_back( flannPoints( maskSingleImage(inputLeftImage, leftROIs[i]), maskSingleImage(inputRightImage, rightROIs[i]), 10 ) );	
		//Now we used the cropped/extracted regions from the images, which means less pixels to be processed. This significantly decreases the delay.		
		triangulatePoints( P1, P2, matchedPoints[i][0], matchedPoints[i][1], temp3DPositions );	//Position of the points inside the ROI are now calculated, time to normalize them and eliminate incorrect ones, also cluster
		for(int j = 0; j < matchedPoints[i][0].size(); j++)
		{				
			tempZ = (temp3DPositions.at<float>(2, j))/(temp3DPositions.at<float>(3, j));
			if(tempZ > 0)	
			{
				//In this case, this particular point inside this particular ROI is acceptable, so normalize the other components and add to the result
				vector<float> tempSingleResult;
				tempSingleResult.push_back(matchedPoints[i][0][j].x);			//Adding x of point on the left image
				tempSingleResult.push_back(matchedPoints[i][0][j].y);			//Adding y of point on the left image						
				tempSingleResult.push_back(matchedPoints[i][1][j].x);			//Adding x of point on the right image
				tempSingleResult.push_back(matchedPoints[i][1][j].y);			//Adding y of point on the right image
				tempSingleResult.push_back((temp3DPositions.at<float>(0, j))/(temp3DPositions.at<float>(3, j)));	//Adding X of point in 3D world
				tempSingleResult.push_back((temp3DPositions.at<float>(1, j))/(temp3DPositions.at<float>(3, j)));	//Adding Y of point in 3D world
				tempSingleResult.push_back(tempZ);																	//Adding Z of point in 3D world
				result[i].push_back(tempSingleResult);	//For ROI i, if appropriate, a vector<float> representing the 3D position of a single point has been added, this is repeated for each ROI, and for each appropriate points inside those ROIs
			} 
			else
			{
				//Do we need to do someting here?
			}   
		}			
	}
	return result;
}

vector<vector<float>> extractFinalPositions(vector<vector<vector<float>>> allFoundPoints)
{
	vector<vector<float>> finalPositions;
	int pointNumberInROI;
	for(int i = 0; i < allFoundPoints.size(); i++)
	{
		pointNumberInROI = allFoundPoints[i].size();
		if(pointNumberInROI > 3)
		{
			std::sort(allFoundPoints[i].begin(), allFoundPoints[i].end(), [](const vector<float>& a, const vector<float>& b) {return a[6] < b[6];});	//Sorts the points inside every ROI using the 6th index, which is the Z component of the real world.
			finalPositions.push_back(allFoundPoints[i][int((pointNumberInROI - 1)/2)]);	//In the final result, put the point with the median Z component in the real world.
		}
		else
		{
			cout << "Error in extract final positions" << endl;
		}
	}
	return finalPositions;
}

void serverThreadFunction()
{
	sql::Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
	sql::ResultSet *res;

	ostringstream stringStream;
	string query_str;

	int last_index = -1;

	//Create an SQL connection to database
	try{
		// Create a connection 
		driver = get_driver_instance();
		con = driver->connect("localhost", "root", "password");
		// Connect to the MySQL test database 
		con->setSchema("data");
		stmt = con->createStatement();
	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;		
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
	
	
	vector<Mat> myPictures;
	vector<Mat> singlePictureVector;
	FaceClassifier classifier = FaceClassifier();
	
	try
	{
		stringStream.str("");
		stringStream << "SELECT * FROM data.images ORDER BY image_id DESC;";
		res = stmt->executeQuery(stringStream.str());
		res -> next();
		last_index = std::stoi(res->getString("image_id"));
	}
	catch(sql::SQLException &e)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
	while(true)
	{
		try {
			stringStream.str("");			
			stringStream << "SELECT * FROM data.images WHERE image_id > " << last_index<< ";";
			res = stmt->executeQuery(stringStream.str());
            while(res -> next()){            	
            	myPictures.push_back(imread("new_server/image_files/" + res->getString("image_name")));
				singlePictureVector.push_back(imread("new_server/image_files/" + res->getString("image_name")));
				last_index = std::stoi(res->getString("image_id"));
				stringStream.str("");				
				stringStream << "UPDATE data.images set person_name='" << classifier.recognizePerson(singlePictureVector) << "' WHERE image_id=" << res->getString("image_id") << ";";
				//stringStream << "UPDATE data.images set person_name='" << "Person" << "' WHERE image_id=" << res->getString("image_id") << ";";		//No longer putting the name in SQL
				stmt->execute(stringStream.str());
				singlePictureVector.clear();	//This one is cleared at each iteration so that it only holds a single image at a time for the individual classifications at the SQL DB side
            }   
			if (myPictures.size() > 0)
			{
				HatDetector hatDetector;
				//Mat hatImage = imread("FaceRecognitionFiles/images/hard_hat_sample_images/hard_hat_workers114.png");
				Mat hatImage;
				int hatDetectionCounter = 0;
				for(int i = 0; i < myPictures.size(); i++)
				{
					hatImage = myPictures[i];
					hatDetectionCounter += hatDetector.detectObjects(hatImage);
				}
				if(hatDetectionCounter > 0)
				{
					cout << "Hat Detected! (" << hatDetectionCounter << " / " << myPictures.size() << ")" << endl;
				}	
				else
				{
					cout << "Hat not detected. (" << hatDetectionCounter << " / " << myPictures.size() << ")" << endl;
				}
				cout << "Person: " << classifier.recognizePerson(myPictures) << endl;				
				myPictures.clear();   					//This one is cleared once all of the images have been used together. This is for reporting the final result, determined by the best of three images
			}     			 

		} catch (sql::SQLException &e) {
			std::cout << "# ERR: SQLException in " << __FILE__;
			std::cout << "# ERR: " << e.what();
			std::cout << " (MySQL error code: " << e.getErrorCode();
			std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		}

		delete res;    
		this_thread::sleep_for(chrono::milliseconds(1000) );	//Main program updates the changes over on the database side every 1 second   
	}
}

void toolLocationUpdater(Mat *updatedLeftImage, Mat *updatedRightImage, bool *programNotTerminated)
{
	Mat leftImage, rightImage;
	vector<Rect> leftROIs, rightROIs;
	vector<int> leftFeedObjectIDVector, rightFeedObjectIDVector;
	vector<vector<vector<float>>> positionsInROIs;
	vector<vector<float>> finalPoints;
	vector<vector<float>> correctedFinalPoints;
	int leftObjectID, rightObjectID;
	Mat floorPlan = imread("EE102 PLAN.png");
	int imageWidth = floorPlan.size().width;
	int imageHeight = floorPlan.size().height;
	Mat tempFloorPlan, tempSquaredImage;
	string toolName;
	float x, y, z;

	vector<vector<int>> toolIDIndices;	
	vector<int> emptyVector;
	toolIDIndices.push_back(emptyVector);	//Chair
	toolIDIndices.push_back(emptyVector);	//Laptop
	toolIDIndices.push_back(emptyVector);	//Keyboard

	ToolDetector toolDetector;

	while ((*programNotTerminated))
	{
		leftImage = (*updatedLeftImage);
		rightImage = (*updatedRightImage);
		toolDetector.detectObjects(leftImage, leftROIs, leftFeedObjectIDVector);
		toolDetector.detectObjects(rightImage, rightROIs, rightFeedObjectIDVector);
		matchIDROIs(&leftROIs, &rightROIs, &leftFeedObjectIDVector, &rightFeedObjectIDVector);
		if( leftROIs.size() != 0 && rightROIs.size() != 0 )
		{
			positionsInROIs = findPositions(leftImage, rightImage, leftROIs, rightROIs);
			finalPoints = extractFinalPositions(positionsInROIs);
			correctedFinalPoints = correctResultsForRotation(finalPoints);
			time_t now = time(0);
			string currentTimeAndDate = ctime(&now);

			//Adding the indices of the points to their respective vectors depending on the object ID
			for(int i = 0; i < correctedFinalPoints.size(); i++)
			{
				leftObjectID = leftFeedObjectIDVector[i];
				if ( leftObjectID == rightFeedObjectIDVector[i] )
				{
					if ( leftObjectID == 1 )
					{
						//Drill
						toolIDIndices[0].push_back(i);
					}
					else if ( leftObjectID == 2 )
					{
						//Grinder
						toolIDIndices[1].push_back(i);
					}
					else if ( leftObjectID == 66 )
					{
						//Keyboard
						toolIDIndices[2].push_back(i);
					}
				}
				else
				{
					cout << "Object matching misfit" << endl;
				}
			}
			
			for(int i = 0; i < toolIDIndices.size(); i++)
			{
				floorPlan.copyTo(tempFloorPlan);			//Generate a separate floor plan for each tool type.
				leftImage.copyTo(tempSquaredImage);			//Generate a separate image to show the squares for each tool type.
				if ( i == 0 )
				{
					toolName = "Drill";
				}
				else if ( i == 1 )
				{
					toolName = "Grinder";
				}
				else if ( i == 2 )
				{
					toolName = "Keyboard";
				}	
				for (int j = 0; j < toolIDIndices[i].size(); j++)	//Iterating over all objects of the same type of tool. (For example, each j is for another chair) toolIDIndices[i][j] is the index of the object in the results.
				{									
					x = correctedFinalPoints[toolIDIndices[i][j]][4];
					y = correctedFinalPoints[toolIDIndices[i][j]][5];
					z = correctedFinalPoints[toolIDIndices[i][j]][6];
					putText(tempFloorPlan, "o " + to_string((int)z), Point2f( ( x + leftExtent ) / (leftExtent + rightExtent) * imageWidth, ( upExtent - y ) / (upExtent + downExtent) * imageHeight ), FONT_HERSHEY_SIMPLEX, 0.6, CV_RGB(120, 160, 0), 2);
					rectangle(tempSquaredImage, leftROIs[toolIDIndices[i][j]], Scalar(0, 0, 255), 5, LINE_8, 0);
					putText(tempFloorPlan, currentTimeAndDate, Point2f(20, 30), FONT_HERSHEY_SIMPLEX, 1, CV_RGB(0, 0, 255), 3);
					putText(tempSquaredImage, currentTimeAndDate, Point2f(20, 30), FONT_HERSHEY_SIMPLEX, 1, CV_RGB(0, 0, 255), 3);
					imwrite("new_server/tool_image_files/Latest" + toolName + "LocationsLayout.png", tempFloorPlan);
					imwrite("new_server/tool_image_files/Latest" + toolName + "LocationsSquared.png", tempSquaredImage);
				}
				toolIDIndices[i].clear();	//Once you have covered all of the points of a certain toop type, clear that portion so that the next update is not affected by it				
			}
		}			
		this_thread::sleep_for(chrono::milliseconds(10000) );	//Update the tool locations every 60 seconds
	}
	
}

//Matches the ROIs from the detector based on their coordinates, and also corrects them if they fall outside the regions of the image.
void matchROIs(vector<Rect> *leftROIs, vector<Rect> *rightROIs)
{
	vector<Rect> correctedLeftROI;
	vector<Rect> correctedRightROI;

	sort( (*leftROIs).begin(), (*leftROIs).end(), [](const Rect& a, const Rect& b) {return a.x < b.x;});
	sort( (*rightROIs).begin(), (*rightROIs).end(), [](const Rect& a, const Rect& b) {return a.x < b.x;});
	//Can also use &&  ( abs(leftROIs[i].width - rightROIs[j].width) < 100 ) && ( abs(leftROIs[i].height - rightROIs[j].height) < 100 ) in the if condition, but this makes it not work when an object is partially in (partial bb)
	//Another idea is to look at the pixel range of each camera, where the other camera doesn't see anything, so that if the boundary box falls in x + width = 30 for right box, then we know it is not seen in the left feed.
	//We can look for the best matches, but that will be considerably slower (N squared vs N, maybe slightly less even)
	for(int i = 0; i < (*leftROIs).size(); i++)
	{
		for(int j = 0; j < (*rightROIs).size(); j++)
		{
			if( ( (*leftROIs)[i].x < (*rightROIs)[j].x ) && ( abs((*leftROIs)[i].y - (*rightROIs)[j].y) < 100  ))
			{
				//Correct for boundary boxes that go outside the boundaries of the image
				if( (*leftROIs)[i].x < 0 )
				{
					(*leftROIs)[i].x = 0;
				}
				if( (*leftROIs)[i].y < 0 )
				{
					(*leftROIs)[i].y = 0;
				}
				if( (*rightROIs)[j].x < 0 )
				{
					(*rightROIs)[j].x = 0;
				}
				if( (*rightROIs)[j].y < 0 )
				{
					(*rightROIs)[j].y = 0;
				}
				if( (*leftROIs)[i].x + (*leftROIs)[i].width > width )
				{
					(*leftROIs)[i].width = width - (*leftROIs)[i].x;
				}
				if( (*leftROIs)[i].y + (*leftROIs)[i].height > height )
				{
					(*leftROIs)[i].height = height - (*leftROIs)[i].y;
				}
				if( (*rightROIs)[j].x + (*rightROIs)[j].width > width )
				{
					(*rightROIs)[j].width = width - (*rightROIs)[j].x;
				}
				if( (*rightROIs)[j].y + (*rightROIs)[j].height > height )
				{
					(*rightROIs)[j].height = height - (*rightROIs)[j].y;
				}
				//Push the matched boxes to be equated to the result
				correctedLeftROI.push_back((*leftROIs)[i]);
				correctedRightROI.push_back((*rightROIs)[j]);
				(*rightROIs).erase((*rightROIs).begin() + j);							//Removing that box so that it will not be paired with other boxes from left
				// cout << "Y diff: " << to_string(abs(leftROIs[i].y - rightROIs[j].y)) << endl;
				// cout << "Width diff: " << to_string(abs(leftROIs[i].width - rightROIs[j].width)) << endl;
				// cout << "Height diff: " << to_string(abs(leftROIs[i].height - rightROIs[j].height)) << endl;
				break;
			}
		}
	}
	*leftROIs = correctedLeftROI;
	*rightROIs = correctedRightROI;
}


//Important note: Since sort() over the ROIs change the order of IDs, they have been deducted. In the future, make the IDs also change with the change of sort(), so that box matching is more accurate
void matchIDROIs(vector<Rect> *leftROIs, vector<Rect> *rightROIs, vector<int> *leftFeedObjectIDVector, vector<int> *rightFeedObjectIDVector)
{
	vector<Rect> correctedLeftROI;
	vector<Rect> correctedRightROI;
	vector<int> correctedLeftIDs;
	vector<int> correctedRightIDs;

	//sort( (*leftROIs).begin(), (*leftROIs).end(), [](const Rect& a, const Rect& b) {return a.x < b.x;});
	//sort( (*rightROIs).begin(), (*rightROIs).end(), [](const Rect& a, const Rect& b) {return a.x < b.x;});

	//Can also use &&  ( abs(leftROIs[i].width - rightROIs[j].width) < 100 ) && ( abs(leftROIs[i].height - rightROIs[j].height) < 100 ) in the if condition, but this makes it not work when an object is partially in (partial bb)
	//Another idea is to look at the pixel range of each camera, where the other camera doesn't see anything, so that if the boundary box falls in x + width = 30 for right box, then we know it is not seen in the left feed.
	//We can look for the best matches, but that will be considerably slower (N squared vs N, maybe slightly less even)
	for(int i = 0; i < (*leftROIs).size(); i++)
	{
		for(int j = 0; j < (*rightROIs).size(); j++)
		{
			if( ( (*leftROIs)[i].x < (*rightROIs)[j].x ) && ( abs((*leftROIs)[i].y - (*rightROIs)[j].y) < 100  ))
			{
				//Correct for boundary boxes that go outside the boundaries of the image
				if( (*leftROIs)[i].x < 0 )
				{
					(*leftROIs)[i].x = 0;
				}
				if( (*leftROIs)[i].y < 0 )
				{
					(*leftROIs)[i].y = 0;
				}
				if( (*rightROIs)[j].x < 0 )
				{
					(*rightROIs)[j].x = 0;
				}
				if( (*rightROIs)[j].y < 0 )
				{
					(*rightROIs)[j].y = 0;
				}
				if( (*leftROIs)[i].x + (*leftROIs)[i].width > width )
				{
					(*leftROIs)[i].width = width - (*leftROIs)[i].x;
				}
				if( (*leftROIs)[i].y + (*leftROIs)[i].height > height )
				{
					(*leftROIs)[i].height = height - (*leftROIs)[i].y;
				}
				if( (*rightROIs)[j].x + (*rightROIs)[j].width > width )
				{
					(*rightROIs)[j].width = width - (*rightROIs)[j].x;
				}
				if( (*rightROIs)[j].y + (*rightROIs)[j].height > height )
				{
					(*rightROIs)[j].height = height - (*rightROIs)[j].y;
				}
				correctedLeftROI.push_back((*leftROIs)[i]);
				correctedRightROI.push_back((*rightROIs)[j]);
				correctedLeftIDs.push_back((*leftFeedObjectIDVector)[i]);
				correctedRightIDs.push_back((*rightFeedObjectIDVector)[j]);
				(*rightFeedObjectIDVector).erase((*rightFeedObjectIDVector).begin() + j);	
				(*rightROIs).erase((*rightROIs).begin() + j);							//Removing that box so that it will not be paired with other boxes from left
				break;
			}
		}
	}
	*leftROIs = correctedLeftROI;
	*rightROIs = correctedRightROI;
	*leftFeedObjectIDVector = correctedLeftIDs;
	*rightFeedObjectIDVector = correctedRightIDs;
}


//Looks at the proximity of people between each other and alerts if social distancing is violated
int alertForSocialDistancing(vector<vector<float>> correctedFinalPoints, float socialDistancingTreshold)
{	
	float tempXDistance, tempYDistance, tempProximity;
	if( correctedFinalPoints.size() != 0 )
	{
		for (int i = 0; i < correctedFinalPoints.size() - 1; i++)
		{
			for (int j = i + 1; j < correctedFinalPoints.size(); j++)
			{
				tempXDistance = correctedFinalPoints[i][4] - correctedFinalPoints[j][4];
				tempYDistance = correctedFinalPoints[i][5] - correctedFinalPoints[j][5];
				tempProximity = tempXDistance * tempXDistance + tempYDistance * tempYDistance;
				if( tempProximity  < socialDistancingTreshold * socialDistancingTreshold  )
				{			
					return 1;
				}
			}
		}
	}
	return 0;
}

#endif
