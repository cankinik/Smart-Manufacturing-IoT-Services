#ifndef _HAT_DETECTOR_H_
#define _HAT_DETECTOR_H_


#include <vector>
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


class HatDetector
{
	public:
		HatDetector();
		bool detectObjects(cv::Mat &image);
		//cppflow::tensor convertMatToTensor(Mat &input);
		
		//vector<Rect> postprocess(Mat& frame, vector<Mat>& boxes);
		
		tensorflow::Status readTensorFromMat(const Mat &mat, tensorflow::Tensor &outTensor) ;
		//tensorflow::Tensor readTensorFromMat(const Mat &mat);
		
		cv::Mat getSquareImage( const cv::Mat& img, int target_width );
		void normalizeImage( cv::Mat &image);

	private:
		/**
		string graph_path = "insert path";
		unique_ptr<tensorflow::Session> session;
		Status status;
		GraphDef graph_def; **/
		
		float confThreshold = 0.5;
		float nmsThreshold = 0.4;
		
		

		string model_path = "FaceRecognitionFiles/models/complex_frozen_graph.pb";
		
		    bool status;

	    //tensorflow objects
	    tensorflow::GraphDef graph_def;
	    tensorflow::Session *session;

	    //layer names for the FaceNet embedder model
	    const std::string input_layer = "x:0";
	    const vector<string> outputLayer = { "Identity:0", 
		            "Identity_1:0", 
		            "Identity_2:0", 
		            "Identity_3:0",
		            "Identity_4:0",
		            "Identity_5:0",
		            "Identity_6:0",
		            "Identity_7:0"};
	    
	    const int size = 640;
		


};


#endif /* _HAT_DETECTOR_H_ */

