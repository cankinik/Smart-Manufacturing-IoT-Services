#ifndef _DETECTOR_H_
#define _DETECTOR_H_

#include <string>
#include "opencv2/opencv.hpp"
#include <opencv4/opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>

/**
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/platform/env.h"

**/



using namespace cv;
using namespace std;
using namespace dnn;

class Detector
{
	public:
		Detector();
		void detectObjects(cv::Mat image, vector<Rect> &boxes);
		
		vector<Rect> postprocess(Mat& frame, vector<Mat>& boxes);
		
		vector<String> getOutputsNames(const Net& net);
		

	private:
		/**
		string graph_path = "insert path";
		unique_ptr<tensorflow::Session> session;
		Status status;
		GraphDef graph_def; **/
		
		float confThreshold = 0.7;
		float nmsThreshold = 0.8;
		int inpWidth = 416;
		int inpHeight = 416;
		
		string classesFile = "DeepSortFiles/config/coco.names";
		String modelConfiguration = "DeepSortFiles/config/yolov3.cfg";
		String modelWeights = "DeepSortFiles/RUNNINGDATA/yolov3.weights";
		
		vector<string> classes;
		
		Net net;
};

//returns intersection over union of two rectangles
float getIOU(Rect first, Rect second);

//returns the index of hte rectangle that overlaps with the new rectangle if there exists one, returns -1 otherwise
int checkOverlap(Rect newRect, vector<Rect> &existingRects, float threshold=0.9 );


#endif /* _DETECTOR_H_ */

