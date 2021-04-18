#ifndef _VIDEOTRACKER_H_
#define _VIDEOTRACKER_H_

#include <string>
#include <future>
#include "model.h"
#include "param.h"


#include "opencv2/opencv.hpp"
#include "tracker.h"
#include "FeatureTensor.h"
#include "tracker.h"
#include "Detector.h"

/**
 * VideoTracker
 *
 * run:
 * -vpath: the video's path.
 * -showSwitch: whether show the tracking result.
 *
 * run_sequenceWithGT:
 * -motDir: the path of MOT directory.
 * -showSwitch: show or not.
 *
 * run_sequence:
 * -motDir: the path of MOT directory.
 * -showSwitch: show or not.
 */
class VideoTracker 
{
	public:
		VideoTracker(const DeepSortParam& tracker_params);
		bool run();
		std::vector<cv::Rect> runFrame(cv::Mat &image);
		std::string showErrMsg();

	private:
		DeepSortParam params;
		std::string errorMsg;
		ModelDetection modelDetection;
		FeatureTensor featureTensor;
		tracker mytracker;
		int detectorIterator = 0;
		Detector detector;
		
};


#endif /* VIDEOTRACKER_H_ */

