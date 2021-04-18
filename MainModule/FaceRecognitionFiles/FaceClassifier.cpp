#include "FaceClassifier.h"
#include <time.h>


using namespace cv;

FaceClassifier::FaceClassifier() {


    // SVM CLASSIFIER LOADING

    if ( access( recognizer_path.c_str(), 0 ) != -1 ){

        try {
      	 	model = FisherFaceRecognizer::create();
      	 	//cout<<"created!"<<endl;
   			model->read(recognizer_path);
           	cout<<"System Loaded!"<<endl;
           	
        } catch (std::exception &e) {
            cout << "Cannot load classifier from " << recognizer_path<< ": " << e.what() << std::endl;
        }
    }
    //if the classifier does not exist
    else {
        updateSystem();
        cout<<"System Updated!"<<endl;
    }
}




FaceClassifier::~FaceClassifier(){

}

//prepares the training data for the face classifier given a file path to training images
void FaceClassifier::detectFaces( cv::Mat &image , std::vector< cv::Mat > &faces ) {


    cv::Mat new_image;
    if (image.empty()) {
        cerr << "Image is empty in detectFaces"<< endl;
        return;
    }
    else if (image.size() != image_size) {
        new_image = getSquareImage( image, 300 );
        //cout << "Resized squared image: height: " << to_string(new_image.size().height) << " Width: " << to_string(new_image.size().width) << endl;
    }
    else
	{
		image.copyTo(new_image);
	}

    std::vector<cv::Rect> dets = face_detector.detect_face_rectangles(new_image);
    for (auto& det : dets) {

        int edge = max(det.height, det.width);
        edge = min(edge, min(new_image.size().height - det.y, new_image.size().width - det.x));

        cv::Rect R = cv::Rect( det.x, det.y,edge,edge );
        cv::Mat ROI = new_image(R);
        Mat greyMat;
        cv::cvtColor(ROI, greyMat, cv::COLOR_BGR2GRAY);
        
        //cout << "ROI: " << to_string(ROI.size().height) << " Width: " << to_string(ROI.size().width) << endl;
        faces.push_back(greyMat);
    }
}

//prepares the training data for the face classifier given a file path to training images
void FaceClassifier::extractEmbeddings_Path(string path, std::vector<Mat> &images, std::vector<int> &labels ) {


    std::pair<std::vector<std::string>, std::vector<int>> image_files;

    parse_images_path(image_files);

    for (int i = 0; i < image_files.first.size(); i++){
    
		cv::Mat image;
		try
 		{
       		image = cv::imread(image_files.first[i]);
		}
		catch(int e){
        	cout << "An exception occurred in imRead in extractEmbeddings_Path. Exception Nr. " << e << '\n';
			cout <<"For image path: "<<image_files.first[i]<<endl;
			continue;
		}
		
		std::vector< cv::Mat > faces;
		
 		try
 		{
   	     	detectFaces(image, faces);
		}
		catch (int e){
			cout << "An exception occurred in detectFaces in extractEmbeddings_Path. Exception Nr. " << e << '\n';
			cout <<"For image path: "<<image_files.first[i]<<endl;
			continue;
		}
	
	
        if (!faces.empty()){


                //if the face was detected, its label is added
                labels.push_back(image_files.second[i]);
                Mat temp;               
                temp = getSquareImage( faces[0] , 300 );
		 images.push_back( temp );

        }
        else {
            cout<<"No faces found!"<<endl;
			cout << "" << image_files.first[i] << endl;
        }
    }
}



//retrains the whole system
void FaceClassifier::updateSystem(){
    std::vector<cv::Mat> images;
    std::vector<int> labels;

    extractEmbeddings_Path(images_path, images, labels );
    
	
    model = FisherFaceRecognizer::create();
	model->train(images, labels);

    model->write(recognizer_path);
    saveIDs();
    cout<<"Model saved!"<<endl;
   
}


//identifies a person given images
string FaceClassifier::recognizePerson( std::vector< cv::Mat > &images ) {

        if( labelMap.empty() && loadIDs()== 0 ){
            cout<<"No IDs registered to the system. Please add photos and update the system."<<endl;
        }

        std::vector< int > predictions;

        for (int i = 0; i < images.size(); i++) {

            std::vector< cv::Mat > faces;
            try
 	   		{
        		detectFaces( images[i], faces);
			}
	    	catch (int e){
				cout << "An exception occurred in detectFaces. Exception Nr. " << e << '\n';
				cout <<"For image number: "<<to_string(i)<<endl;
				continue;
			}

            if (!faces.empty()){
            
            	Mat temp;
           
           		temp = getSquareImage( faces[0] , 300 );
				double confidence = 0.0;
				int prediction = -1;
				
				model->predict(temp, prediction, confidence);
				
				//cout << "Prediction: " << to_string(prediction) << endl;
                predictions.push_back( prediction );
                //cout << "Confidence: " << to_string(confidence) << endl;
            }

        }

        if (predictions.empty()) {
            return "Not Found";
        }

        //CALCULATE THE MOST FREQUENT LABEL

        sort(predictions.begin(), predictions.end());

        int max_count = 1;
        int result = predictions[0];
        int temp_count = 1;
		//cout << "Predictions[0]: " << to_string(predictions[0]) << endl;
        for (int i = 1; i < predictions.size(); i++) {
			//cout << "Predictions[" <<to_string(i)<< "]: " << to_string(predictions[i]) << endl;
            if (predictions[i] == predictions[i - 1])
                temp_count++;
            else {
                if (temp_count > max_count) {
                    max_count = temp_count;
                    result = predictions[i - 1];
                }
                temp_count = 1;
            }
        }

        //if the result is at the end
        if (temp_count > max_count)
        {
            max_count = temp_count;
            result = predictions[predictions.size() - 1];
        }

        //retrieving name of the person from the label map
        string name = labelMap[ to_string(result) ];
        //cout<<"Identified person ID: "+ to_string(result) + " Name: " + name <<endl;

        return name;
}

//loads the ID number - Name mapping for employees to the  "labelMap" variable
int FaceClassifier::loadIDs(){
        int count = 0;

        if (access(labelMap_path.c_str(), R_OK) < 0)
                return count;

        FILE *fp = fopen(labelMap_path.c_str(), "r");

        if (!fp)
                return count;

        labelMap.clear();

        char *buf = 0;
        size_t buflen = 0;

        while(getline(&buf, &buflen, fp) > 0) {

                char *nl = strchr(buf, '\n');
                if (nl == NULL)
                        continue;
                *nl = 0;

                char *sep = strchr(buf, '=');
                if (sep == NULL)
                        continue;
                *sep = 0;
                sep++;

                std::string s1 = buf;
                std::string s2 = sep;

                labelMap[s1] = s2;

                count++;
        }

        if (buf)
                free(buf);

        fclose(fp);
        return count;
}


//saves the ID number - Name mapping in the "labelMap" variable to a text file
int FaceClassifier::saveIDs(){
        int count = 0;
        if (labelMap.empty())
                return 0;

        //creates or overwrites a file for writing the label map
        FILE *fp = fopen(labelMap_path.c_str(), "w");
        if (!fp)
                return -errno;

        for(std::map<std::string, std::string>::iterator it = labelMap.begin(); it != labelMap.end(); it++) {
                //writes label mappings
                fprintf(fp, "%s=%s\n", it->first.c_str(), it->second.c_str());
                count++;
        }

        fclose(fp);
        return count;
}



//given a directory path, this method extracts the images and their labels
void FaceClassifier::parse_images_path(std::pair<std::vector<std::string>, std::vector<int>> &image_files){
        DIR *dir;
        struct dirent *entry;

        int class_id = 1;

        //opens images directory
        if ((dir = opendir(images_path.c_str())) != nullptr) {

            //while there are class folder
            while ((entry = readdir(dir)) != nullptr) {

                //if the entry is a folder
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {

                    std::string class_name = std::string(entry->d_name);

                    //maps new label id to label name
                    labelMap[ to_string(class_id) ] = class_name;

                    

                    //parsing file paths for each image in a class folder

                    DIR *classDir;
                    struct dirent *classEntry;

                    if ((classDir = opendir( (images_path + "/" + class_name).c_str() ) ) != nullptr) {

                        while ((classEntry = readdir(classDir)) != nullptr) {

                            //if the entry is not a folder (it should be an image)
                            if (classEntry->d_type != DT_DIR) {

                                std::string file_path = images_path + "/" + class_name + "/" + classEntry->d_name;
                                image_files.first.emplace_back(file_path);
                                image_files.second.emplace_back(class_id);
								//cout << "Class ID: " << to_string(class_id) << endl;
                            }
                        }
                        closedir(classDir);

                    }
					class_id++;
                }
            }
            closedir(dir);
        }
}


static Mat norm_0_255(InputArray _src) {
    Mat src = _src.getMat();
    // Create and return normalized image:
    Mat dst;
    switch(src.channels()) {
    case 1:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
        break;
    case 3:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
        break;
    default:
        src.copyTo(dst);
        break;
    }
    return dst;
}


//normalizes the image
void FaceClassifier::normalizeImage(cv::Mat &image){
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
}

//image scaling that preserves aspect ratio, taken from: https://stackoverflow.com/questions/28562401/resize-an-image-to-a-square-but-keep-aspect-ratio-c-opencv
cv::Mat FaceClassifier::getSquareImage( const cv::Mat& img, int target_width = 300 )
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
