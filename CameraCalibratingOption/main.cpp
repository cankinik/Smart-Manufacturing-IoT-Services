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

const double squareSize = 0.038; //0.029 for A4, for cagan it is 0.034
const int width = 1920; 
const int height = 1080; 
const Size imageSize = Size(1920, 1080); 
Mat cameraMatrix1, distCoeffs1, R1, T1;
Mat cameraMatrix2, distCoeffs2, R2, T2;
Mat R, F, E;
Vec3d T;
Mat Rot1, Rot2, P1, P2, Q;
Mat leftMapX, leftMapY, rightMapX, rightMapY;
Mat correctingRotationalMatrix, correctingTranslationalVector;

int main()
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
	return 0;
}
