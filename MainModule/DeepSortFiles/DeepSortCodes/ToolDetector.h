#ifndef _TOOL_DETECTOR_H_
#define _TOOL_DETECTOR_H_


#include <vector>
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

#include "Detector.h"


using namespace cv;
using namespace std;
using namespace dnn;


class ToolDetector
{
	public:
		ToolDetector();
		void detectObjects(cv::Mat frame, vector<Rect> &boxes, vector<int> &objectIDVector);
		
		tensorflow::Status readTensorFromMat(const Mat &mat, tensorflow::Tensor &outTensor) ;
		
		cv::Mat getSquareImage2( const cv::Mat& img, int target_width, cv::Rect &roi, float &scale );
		void normalizeImage( cv::Mat &image);

	private:
		
		float confThreshold = 0.01;
		float nmsThreshold = 0.4;
		
		

		string model_path = "DeepSortFiles/config/2tools_complex_frozen_graph.pb";
		
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


#endif /* _TOOL_DETECTOR_H_ */

