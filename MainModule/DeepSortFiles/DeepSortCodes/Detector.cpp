#include "Detector.h"

Detector::Detector()
{

    ifstream ifs(classesFile.c_str());
    string line;
    
    while (getline(ifs, line)) 
    	classes.push_back(line);
    	
    net = readNetFromDarknet(modelConfiguration, modelWeights);
    // net.setPreferableBackend(DNN_BACKEND_OPENCV);
    // net.setPreferableTarget(DNN_TARGET_CPU);
    net.setPreferableBackend(DNN_BACKEND_CUDA);
    net.setPreferableTarget(DNN_TARGET_CUDA);
}



/** Convert Mat image into tensor of shape (1, height, width, d) where last three dims are equal to the original dims.
TODO: support batch mat input
 */



void Detector::detectObjects(cv::Mat frame, vector<Rect> &boxes){

	Mat blob;
	vector<Mat> outs;
	// Create a 4D blob from a frame.
    blobFromImage(frame, blob, 1/255.0, cv::Size(inpWidth, inpHeight), Scalar(0,0,0), true, false);
	
	net.setInput(blob);
	net.forward(outs, getOutputsNames(net));
	
	boxes = postprocess(frame, outs);
}



// Remove the bounding boxes with low confidence using non-maxima suppression
vector<Rect> Detector::postprocess(Mat& frame, vector<Mat>& outs)
{

    vector<float> confidences;
    vector<Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i)
    {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
        {
            Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold && (classIdPoint.x == 0))	// 0 = person, 56 = chair, 63 = laptop, 66 = keyboard		
            {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;
                
                Rect newRect = Rect(left, top, width, height);
                
                int index = checkOverlap(newRect, boxes, 0.7);
                
                //if no overlapping rectangles are found or overalpping rectangles are from different classes
                if ( index == -1 ){
                
		            //cout<<to_string(classIdPoint.x)<<endl;
		            
		            confidences.push_back((float)confidence);
		            boxes.push_back(newRect);
                }
                else{
                	//cout<<"Found overlapping box!!"<<endl;
                	//cout<<newRect<<endl;
                	//cout<<boxes[index]<<endl;
                	
                }
            }
        }
    }
    
    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    
    /**
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        rectangle(frame, Point(box.x, box.y), Point(box.x + box.width, box.y + box.height), Scalar(255, 178, 50), 5);
        
        cout<<"Person box "<<to_string(i)<<endl;
    }   */ 
    
    return boxes;

}



// Get the names of the output layers
vector<String> Detector::getOutputsNames(const Net& net)
{
    static vector<String> names;
    if (names.empty())
    {
        //Get the indices of the output layers, i.e. the layers with unconnected outputs
        vector<int> outLayers = net.getUnconnectedOutLayers();
        
        //get the names of all the layers in the network
        vector<String> layersNames = net.getLayerNames();
        
        // Get the names of the output layers in names
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
        names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}


//returns intersection over union of two rectangles
float getIOU(Rect first, Rect second){

	float intersection_area = (first & second).area();
	float union_area = (first|second).area();
	
	float iou = intersection_area / union_area ;
	
	//cout<<"IOU : "<<iou<<endl;
	
	return iou  ;
}

//returns the index of hte rectangle that overlaps with the new rectangle if there exists one, returns -1 otherwise
int checkOverlap(Rect newRect, vector<Rect> &existingRects, float threshold ) {

	int index = -1;
	float maxIOU = 0;
	
	for( int i = 0; i < existingRects.size() ; i++ ){
	
		float newIOU = getIOU( existingRects[i] , newRect );
		if( newIOU > threshold && newIOU > maxIOU ){
			index = i;
			maxIOU = newIOU;
			//cout<<"found one"<<endl;
		}
	}
	return index;
}


//returns the index of hte rectangle that overlaps with the new rectangle if there exists one, returns -1 otherwise





