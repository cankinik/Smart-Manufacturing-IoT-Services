/*
 * VideoTracker.cpp
 *
 *  Created on: Dec 15, 2017
 *      Author: zy
 */
#include "VideoTracker.h"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

VideoTracker::VideoTracker(const DeepSortParam& tracker_params) : params(tracker_params)
{
	modelDetection._init(params.detections(), params.images());
	mytracker._init(params);
	featureTensor._init(params.metric_model(), params.feature_model());
}



bool VideoTracker::run()
{
	//tracker mytracker(params);
	//std::shared_ptr<FeatureTensor> featureTensor(new FeatureTensor(params.metric_model(), params.feature_model()));
	int detectorIterator = 0;
	while(true) 
	{
		detectorIterator++;
		Mat frame;

		DETECTIONS detections;
		mytracker.predict();
		if(detectorIterator > 5)
		{
			detectorIterator = 0;
			if(modelDetection.getFrameDetections(frame, detections) == false) 
			{
				this->errorMsg = "No more frames!";
				return false;
			}
			//modelDetection->dataMoreConf(args_min_confidence, detections);
			//modelDetection->dataPreprocessing(args_nms_max_overlap, detections);
			
			//TENSORFLOW get rect's feature.
			if(featureTensor.getRectsFeature(frame, detections) == false) 
			{
				this->errorMsg = "Tensorflow get feature failed!";
				return false;
			}
			mytracker.update(detections);
		}			
		
		std::vector<RESULT_DATA> result;
		for(Track& track : mytracker.tracks) 
		{
			if(!track.is_confirmed() || track.time_since_update > 1) continue;
			result.push_back(std::make_pair(std::make_pair(track.track_id, track.detection_class), std::make_pair(track.to_tlwh(), track.color)));
		}
		std::stringstream ss;
		for(unsigned int k = 0; k < result.size(); k++) 
		{
			DETECTBOX tmp = result[k].second.first;
			std::string det_class = result[k].first.second;
			cv::Scalar color = result[k].second.second;
			Rect rect = Rect(tmp(0), tmp(1), tmp(2), tmp(3));
			//rectangle(frame, rect, color, 2);
			ss << result[k].first.first << " - " << det_class;
			//putText(frame, ss.str(), Point(rect.x, rect.y), cv::FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
			ss.str("");
		}
		//imshow("DeepSortTracking", frame);
		//waitKey(1);
	}//end while;
	return true;
}

std::vector<cv::Rect> VideoTracker::runFrame(cv::Mat &frame)
{
		DETECTIONS detections;
		
		//---------------------------------------------------------- DETECTOR CODE
		detectorIterator++;
		std::vector<cv::Rect> rects, result_rects;
		if(detectorIterator > 0)
		{
			detectorIterator = 0;
			detector.detectObjects(frame, rects);
			for (int i = 0; i<rects.size() ; i++){
			
				DETECTION_ROW tmpRow;
				tmpRow.class_num = 0;
				tmpRow.confidence = 1.0;
				tmpRow.tlwh = DETECTBOX(rects[i].x, rects[i].y, rects[i].width, rects[i].height);
				detections.push_back(tmpRow);
			}
			
			
			//modelDetection->dataMoreConf(args_min_confidence, detections);
			//modelDetection->dataPreprocessing(args_nms_max_overlap, detections);
			
			//TENSORFLOW get rect's feature.
			if(featureTensor.getRectsFeature(frame, detections) == false) 
			{
				this->errorMsg = "Tensorflow get feature failed!";
				vector<Rect> emptyTemp;
				return emptyTemp;
			}
			mytracker.update(detections);
		}
		mytracker.predict();
				
		std::vector<RESULT_DATA> result;
		
		for(Track& track : mytracker.tracks) 
		{
			if(!track.is_confirmed() || track.time_since_update > 1) continue;
			result.push_back(std::make_pair(std::make_pair(track.track_id, track.detection_class), std::make_pair(track.to_tlwh(), track.color)));
		}

		// if(params.show_detections())
		// {
		// 	for(unsigned int k = 0; k < detections.size(); k++) 
		// 	{
		// 		DETECTBOX tmpbox = detections[k].tlwh;
		// 		Rect rect(tmpbox(0), tmpbox(1), tmpbox(2), tmpbox(3));
		// 		//rectangle(frame, rect, Scalar(0,0,255), 4);
		// 	}
		// }

		std::stringstream ss;
		for(unsigned int k = 0; k < result.size(); k++) 
		{
			DETECTBOX tmp = result[k].second.first;
			std::string det_class = result[k].first.second;
			cv::Scalar color = result[k].second.second;
			Rect rect = Rect(tmp(0), tmp(1), tmp(2), tmp(3));
			//rectangle(frame, rect, color, 2);
			ss << result[k].first.first << " - " << det_class;
			//putText(frame, ss.str(), Point(rect.x, rect.y), cv::FONT_HERSHEY_SIMPLEX, 0.8, color, 2);
			ss.str("");
			result_rects.push_back(rect);
			
		}
		//imshow("DeepSortTracking", frame);
		//waitKey(1);
		
		return result_rects;

}




std::string VideoTracker::showErrMsg() 
{
	return this->errorMsg;
}
