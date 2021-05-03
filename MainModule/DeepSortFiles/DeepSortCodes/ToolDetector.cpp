
#include "ToolDetector.h"

ToolDetector::ToolDetector()
{

    tensorflow::Status status = ReadBinaryProto(tensorflow::Env::Default(), model_path, &graph_def);
    if (!status.ok()) {
        throw std::runtime_error(status.error_message());
    }

    tensorflow::SessionOptions options;
    status = tensorflow::NewSession(options, &session);
    if (!status.ok()) {
        throw std::runtime_error(status.error_message());
    }
    status = session->Create(graph_def);
    if (!status.ok()) {
        throw std::runtime_error(status.error_message());
    }

}



void ToolDetector::detectObjects(cv::Mat frame, vector<Rect> &boxes, vector<int> &objectIDVector){

	cv::Rect roi;
	float scale;   
	Mat tempImg = frame; 

    cv::Mat new_image;
    if (frame.empty()) {
        cerr << "Image is empty "<< endl;
        return;
    }
    else if (frame.size() != cv::Size(size, size)) {
        new_image = getSquareImage2( frame, size, roi, scale );
    }
    else
	{
		scale = 1;
		roi.x = 0;
		roi.y = 0;
		roi.height = size;
		roi.width = size;
		frame.copyTo(new_image);
	}
    		

    cv::cvtColor(new_image, new_image, cv::COLOR_BGR2RGB);
	
	/** THIS PART IS NO IN USE
    //NORMALIZE
    try
 	{
    	normalizeImage(new_image);
	}
	catch(int e){
        cout << "An exception occurred in normalizeImage. Exception Nr. " << e << '\n';
		return;
	}
	
	*/
	
	
    //INPUT TENSOR
    tensorflow::Tensor input_tensor(tensorflow::DT_UINT8, tensorflow::TensorShape({1, size, size, 3}));


	try
 	{

    // get pointer to memory for that Tensor
    uint8_t *p = input_tensor.flat<uint8_t>().data();
    

    // create a "fake" cv::Mat from it
    cv::Mat camera_image(size, size, CV_8UC3, p);
    new_image.convertTo(camera_image, CV_8UC3);

	}
	catch(int e){
        cout << "An exception occurred in conversion. Exception Nr. " << e << '\n';
		return;
	}
	
    std::vector<tensorflow::Tensor> outputs;
    std::vector<std::pair<std::string, tensorflow::Tensor>> feed_dict = {
                {input_layer,   input_tensor},
    };

    tensorflow::Status run_status = session->Run(feed_dict, {outputLayer}, {}, &outputs);

    if (!run_status.ok()) {
        cout << "Running model failed: " << run_status << std::endl;
        return;
    }
	
	//Detetion bounding boxes
	float_t* output0Ptr = outputs[1].flat<float_t>().data();
	cv::Mat mat_row(cv::Size(4,300), CV_32F, output0Ptr);
	
	/**
	float_t* output1Ptr = outputs[5].flat<float_t>().data();
	cv::Mat mat_num(cv::Size(1,1), CV_32F, output1Ptr); */
	 
	//Confidence values
	float_t* output2Ptr = outputs[4].flat<float_t>().data();
	cv::Mat mat_conf(cv::Size(300,1), CV_32F, output2Ptr);
	
	//multiclass Confidence values
	float_t* outputmPtr = outputs[3].flat<float_t>().data();
	cv::Mat mat_multi(cv::Size(300,1), CV_32F, outputmPtr);
	
	//Labels
	float_t* output3Ptr = outputs[2].flat<float_t>().data();
	cv::Mat mat_labels(cv::Size(300,1), CV_32F, output3Ptr);
	
	
	for( int i = 0; i<300; i++){
	
	int y_left_bottom = static_cast<int>(size*mat_row.at<float>(i,0));
    int x_left_bottom = static_cast<int>(size*mat_row.at<float>(i,1));
    int y_right_top = static_cast<int>(size*mat_row.at<float>(i,2));
    int x_right_top = static_cast<int>(size*mat_row.at<float>(i,3));
    
    	if (x_left_bottom== 0 && y_left_bottom== 0 && x_right_top== 0 && x_left_bottom== 0)
    		continue;
    		
    //transforming the coordinates to the form of the oiginal image 
    y_left_bottom =( y_left_bottom - roi.y)/scale;
    x_left_bottom =(x_left_bottom - roi.x)/scale;
    y_right_top =( y_right_top - roi.y )/scale;
    x_right_top =( x_right_top - roi.x )/scale;

	//cout<<mat_row<<endl;
	Rect rect = Rect(x_left_bottom, y_left_bottom, (x_right_top - x_left_bottom), (y_right_top - y_left_bottom));
	
	//
	
	int index = checkOverlap(rect, boxes, 0.5);
	
	if (index > -1 || mat_conf.at<float>(i) < confThreshold ){
	
		/**
		if(i==0 ||i==1){
			cout<<mat_conf.at<float>(i)<<endl;
			cout<<"multi"<<endl;	
			cout<<mat_multi.at<float>(i)<<endl;
		
		}**/
		
		continue;
	}
	/**
	

	cout<<"PASSED CONFIDENCE"<<endl;	
	cout<<mat_conf.at<float>(i)<<endl;			 
	//cout<<rect<<endl;
	cout<<"MULTICLASS CONFIDENCE"<<endl;	
	cout<<mat_multi.at<float>(i)<<endl;
	
	**/
	
	boxes.push_back(rect);
	objectIDVector.push_back( round( mat_labels.at<float>(i) ));
	
	
	//rectangle(tempImg, rect, Scalar(100, 0, 100), 10, LINE_8, 0);
	
	}
	
	//string name = "/home/mert/Desktop/outputs/a" + to_string(mat_conf.at<float>(0)) + ".png";
	
	//imwrite(name,tempImg);

}



//normalizes the image **TO UINT8 FORMAT**
void ToolDetector::normalizeImage(cv::Mat &image){
    //mean and std
    cv::Mat temp = image.reshape(1, image.rows * 3);
    cv::Mat mean3;
    cv::Mat stddev3;
    cv::meanStdDev(temp, mean3, stddev3);

    double mean_pxl = mean3.at<double>(0);
    double stddev_pxl = stddev3.at<double>(0);

    cv::Mat image2;
    image.convertTo(image2, CV_64FC1);

    image = image2;
    image = image - cv::Scalar(mean_pxl, mean_pxl, mean_pxl);
    image = image / stddev_pxl;
    
    image = image*256;
    
}



cv::Mat ToolDetector::getSquareImage2( const cv::Mat& img, int target_width, cv::Rect &roi, float &scale )
{
    int width = img.cols;
    int height = img.rows;

    cv::Mat square = cv::Mat::zeros( target_width, target_width, img.type() );

    int max_dim = ( width >= height ) ? width : height;
    scale = ( ( float ) target_width ) / max_dim;
    
    if ( width >= height )
    {
        roi.width = target_width;
        
        roi.x = 0;
        roi.height = height * scale;
        roi.y = ( target_width - roi.height ) / 2;
    }
    else
    {
        roi.y = 0;
        roi.height = target_width;
        roi.width = width * scale;
        roi.x = ( target_width - roi.width ) / 2;
    }

    cv::resize( img, square( roi ), roi.size() );

    return square;
}



tensorflow::Status ToolDetector::readTensorFromMat(const Mat &mat, tensorflow::Tensor &outTensor) {

    auto root = tensorflow::Scope::NewRootScope();
    using namespace ::tensorflow::ops;

    // Trick from https://github.com/tensorflow/tensorflow/issues/8033
    float *p = outTensor.flat<float>().data();
    Mat fakeMat(mat.rows, mat.cols, CV_32FC3, p);
    mat.convertTo(fakeMat, CV_32FC3);

    auto input_tensor = Placeholder(root.WithOpName("input"), tensorflow::DT_FLOAT);
    vector<pair<string, tensorflow::Tensor>> inputs = {{"input", outTensor}};
    auto uint8Caster = Cast(root.WithOpName("uint8_Cast"), outTensor, tensorflow::DT_UINT8);

    // This runs the GraphDef network definition that we've just constructed, and
    // returns the results in the output outTensor.
    tensorflow::GraphDef graph;
    TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));

    vector<tensorflow::Tensor> outTensors;
    unique_ptr<tensorflow::Session> session(tensorflow::NewSession(tensorflow::SessionOptions()));

    TF_RETURN_IF_ERROR(session->Create(graph));
    TF_RETURN_IF_ERROR(session->Run({inputs}, {"uint8_Cast"}, {}, &outTensors));

    outTensor = outTensors.at(0);
    return tensorflow::Status::OK();
}





