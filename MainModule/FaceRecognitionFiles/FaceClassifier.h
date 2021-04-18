

#ifndef __FACE_CLASSIFIER_H
#define __FACE_CLASSIFIER_H

#include <iostream>
#include <fstream>
#include<cstdlib>
#include <vector>
#include <sstream>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "FaceDetector.h"

#include <math.h>



#include <opencv2/opencv.hpp>
#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"


#include <opencv2/face.hpp>

#include <dirent.h>


using namespace std;

using namespace cv;
using namespace cv::face;


class FaceClassifier {

public:

    FaceClassifier();
    ~FaceClassifier();


    //loads the ID number - Name mapping for employees to the  "labelMap" variable
    int loadIDs();

    //saves the ID number - Name mapping in the "labelMap" variable to a text file
    int saveIDs();

    //identifies a person given images
    string recognizePerson( std::vector< cv::Mat > &images ) ;

    //prepares the training data for the face classifier given a file path to training images
    void extractEmbeddings_Path(string path, std::vector<Mat>& images, std::vector<int>& labels);

    //retrains the whole system
    void updateSystem();

    //detects the faces in an image and retrieves the image patches for the faces
    void detectFaces(cv::Mat& image, std::vector< cv::Mat > &faces );
    
    cv::Mat getSquareImage( const cv::Mat& img, int target_width );



private:

    void parse_images_path(std::pair<std::vector<std::string>, std::vector<int>>& image_files);

    void normalizeImage( cv::Mat &image);


    //path to image files for employees
    const string images_path = "FaceRecognitionFiles/images/train_images";

    //path to mapping between IDs and names
    const string labelMap_path = "FaceRecognitionFiles/label_map.txt";
    
        //path to mapping between IDs and names
    const string recognizer_path = "FaceRecognitionFiles/face2.xml";

    //input image size for the face detector
    const cv::Size image_size = cv::Size(300, 300);

    //to be used to map label names to indexes
    std::map<string, string> labelMap;

    double threshold = 10.0;



    Ptr<FaceRecognizer> model;

    //old face detector from dlib
    //dlib::frontal_face_detector face_detector;

    //new face detector taken from https://github.com/bewagner/visuals/tree/blog-post-1
    FaceDetector face_detector;


};

#endif

