
#include "HatDetector.h"

HatDetector::HatDetector()
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



bool HatDetector::detectObjects(cv::Mat &frame){

    cv::Mat new_image;
    if (frame.empty()) {
        cerr << "Image is empty "<< endl;
        return false;
    }
    else if (frame.size() != cv::Size(size, size)) {
        new_image = getSquareImage( frame, size );
    }
    else
	{
		frame.copyTo(new_image);
	}
    		

    cv::cvtColor(new_image, new_image, cv::COLOR_BGR2RGB);
	//Mat tempImg = new_image;
	
    //NORMALIZE
    try
 	{
    	normalizeImage(new_image);
	}
	catch(int e){
        cout << "An exception occurred in normalizeImage. Exception Nr. " << e << '\n';
		return false;
	}
	
	
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
		return false;
	}
	
	
	
    //RUNNING FACE_NET EMBEDDER
    std::vector<tensorflow::Tensor> outputs;
    std::vector<std::pair<std::string, tensorflow::Tensor>> feed_dict = {
                {input_layer,   input_tensor},
    };

    tensorflow::Status run_status = session->Run(feed_dict, {outputLayer}, {}, &outputs);

    if (!run_status.ok()) {
        cout << "Running model failed: " << run_status << std::endl;
        return false;
    }


	//boxes = outputs[1];
	
	//cout<<"Outputs size"<<endl;
	//cout<<outputs.size()<<endl;
	
	//cout<<outputs[1].shape().dims()<<endl;
	

// Print the results
//std::cout << outputs[1].DebugString() << "\n"; // Tensor<type: float shape: [] values: 30>


	int num_dimensions = outputs[1].shape().dims();
    for(int ii_dim=0; ii_dim<num_dimensions; ii_dim++) {
        //cout << to_string(outputs[1].shape().dim_size(ii_dim)) << " ";
    }
	//cout << "" << endl;
	
	float_t* output0Ptr = outputs[1].flat<float_t>().data();
	cv::Mat mat_row(cv::Size(4,300), CV_32F, output0Ptr);
	
	/**
	float_t* output1Ptr = outputs[5].flat<float_t>().data();
	cv::Mat mat_num(cv::Size(1,1), CV_32F, output1Ptr); */
	 
	float_t* output2Ptr = outputs[4].flat<float_t>().data();
	cv::Mat mat_conf(cv::Size(300,1), CV_32F, output2Ptr);
	
	//cout<<"NUM"<<endl;
	//cout<<mat_num<<endl;
	//cout<<"CONFIDENCE"<<endl;
	//cout<<mat_conf<<endl;
	
	for( int i = 0; i<300; i++){
	
	
	/**
	cout<<mat_row.at<float>(0,0)<<endl;
	cout<<mat_row.at<float>(0,1)<<endl;
	cout<<mat_row.at<float>(0,2)<<endl;
	cout<<mat_row.at<float>(0,3)<<endl; */
	
	int y_left_bottom = static_cast<int>(size*mat_row.at<float>(i,0));
    int x_left_bottom = static_cast<int>(size*mat_row.at<float>(i,1));
    int y_right_top = static_cast<int>(size*mat_row.at<float>(i,2));
    int x_right_top = static_cast<int>(size*mat_row.at<float>(i,3));
    
    	if (x_left_bottom== 0 && y_left_bottom== 0 && x_right_top== 0 && x_left_bottom== 0)
    		continue;

	//cout<<mat_row<<endl;
	Rect rect = Rect(x_left_bottom, y_left_bottom, (x_right_top - x_left_bottom), (y_right_top - y_left_bottom));
	
	if (mat_conf.at<float>(i) < 0.85){
		//cout<<mat_conf.at<float>(i)<<endl;
		continue;
	}
	
	cout<<mat_conf.at<float>(i)<<endl;
	return true;
	//cout<<"PASSED CONFIDENCE"<<endl;	
	//cout<<mat_conf.at<float>(i)<<endl;			 
	//cout<<rect<<endl;
	
	//boxes.push_back(rect);
	
	//rectangle(tempImg, rect, Scalar(100, 0, 100), 10, LINE_8, 0);
	
	}
	
	//cv::cvtColor(tempImg, tempImg, cv::COLOR_RGB2RGB);
	//char p;
	
	//imshow("start",tempImg);
	
	//imwrite("/home/mert/Desktop/attempt_savedmodel/new.jpg",tempImg);
	
	return false;

}



//normalizes the image **TO UINT8 FORMAT**
void HatDetector::normalizeImage(cv::Mat &image){
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



cv::Mat HatDetector::getSquareImage( const cv::Mat& img, int target_width = 640 )
{
    int width = img.cols;
    int height = img.rows;

    cv::Mat square = cv::Mat::zeros( target_width, target_width, img.type() );

    int max_dim = ( width >= height ) ? width : height;
    float scale = ( ( float ) target_width ) / max_dim;
    cv::Rect roi;
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



tensorflow::Status readTensorFromMat(const Mat &mat, tensorflow::Tensor &outTensor) {

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





