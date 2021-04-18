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
#endif
