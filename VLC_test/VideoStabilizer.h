#pragma once

#include "Header.h"

class VideoStabilizer
{
public:
	static void stabilizeVideo(string videoFileName)
	{
		VideoCapture cap(videoFileName); // open the default camera
		if (!cap.isOpened())  // check if we succeeded
			return;
		double count = cap.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
		cout << count << endl;
		//Mat edges;
		//namedWindow("edges", 1);
		cout << "Processing Frames..." << endl;
		cap.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
		Mat frame1, frame2;
		bool success = cap.read(frame1);
		int minHessian = 400;

		SurfFeatureDetector detector;

		std::vector<KeyPoint> keypoints_1, keypoints_2;
		detector.detect(frame1, keypoints_1);

		//-- Step 2: Calculate descriptors (feature vectors)
		SurfDescriptorExtractor extractor;

		Mat descriptors_1, descriptors_2;
		extractor.compute(frame1, keypoints_1, descriptors_1);
		VideoWriter vidWriter;
		Size sz = frame1.size();
		//sz.width *= 2;
		vidWriter.open("slow.avi", CV_FOURCC('D', 'I', 'V', 'X'), 30, sz);
		int ind = 0;
		vidWriter << frame1;
		while (success)
		{
			printf(".");
			//frame1 = frame2;
			success = cap.read(frame2);
			if (!success) break;
			detector.detect(frame2, keypoints_2);
			extractor.compute(frame2, keypoints_2, descriptors_2);
			// stabilize frame
			//-- Step 3: Matching descriptor vectors with a brute force matcher
			BFMatcher matcher(NORM_L2);
			std::vector< DMatch > matches;
			matcher.match(descriptors_1, descriptors_2, matches);

			std::vector<cv::Point2f> srcPoints;
			std::vector<cv::Point2f> dstPoints;
			for (int i = 0; i < matches.size(); i++)
			{
				dstPoints.push_back(keypoints_1[matches[i].queryIdx].pt);
				srcPoints.push_back(keypoints_2[matches[i].trainIdx].pt);
			}
			cv::Mat homography = cv::findHomography(srcPoints, dstPoints, CV_RANSAC);
			Mat temp;
			cv::warpPerspective(frame2, temp, homography, frame2.size());
			//hconcat(frame2, temp, temp);
			vidWriter << temp;
			//imshow("temp", temp);
			//waitKey(33);

			//frame1 = frame2;
			//keypoints_1 = keypoints_2;
			//descriptors_1 = descriptors_2;
		}
	}

	static void stabilizeVideo2(string videoFileName)
	{
		VideoCapture cap(videoFileName); // open the default camera
		if (!cap.isOpened())  // check if we succeeded
			return;
		double count = cap.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
		cout << count << endl;
		//Mat edges;
		//namedWindow("edges", 1);
		cout << "Processing Frames..." << endl;
		cap.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
		Mat frame1, frame2;
		bool success = cap.read(frame1);

		VideoWriter vidWriter;
		Size sz = frame1.size();
		//sz.width *= 2;
		vidWriter.open("rigid_transform.avi", CV_FOURCC('D', 'I', 'V', 'X'), 30, sz);
		int ind = 0;
		success = cap.read(frame2);
		vidWriter << frame1;
		while (success)
		{
			printf(".", ind++);
			//frame1 = frame2;

			// stabilize frame
			Mat temp;
			Mat M = estimateRigidTransform(frame1, frame2, 0);
			if (M.size().width == 0 || M.size().height == 0)
			{
				temp = frame2.clone();
			}
			else
			{
				warpAffine(frame2, temp, M, frame2.size(), INTER_NEAREST | WARP_INVERSE_MAP);
			}
			//hconcat(frame2, temp, temp);
			//imshow("stabilized", temp);
			//puts("\t...done");
			//cv::waitKey(10);
			vidWriter << temp;
			success = cap.read(frame2);
		}
	}

	static void DisplatStabilizeVideo(string videoFileName, string stabilizedName)
	{
		VideoCapture cap(videoFileName); // open the default camera
		VideoCapture cap1(stabilizedName); // open the default camera
		if (!(cap.isOpened() && cap1.isOpened()))  // check if we succeeded
			return;
		double count = cap.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
		cout << count << endl;
		count = cap1.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
		cout << count << endl;
		cout << "Processing Frames..." << endl;
		cap.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
		cap1.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
		Mat frame, frame1;
		bool success = cap.read(frame) && cap1.read(frame1);
		//VideoWriter vidWriter1;
		//Size sz = frame.size();
		//sz.width *= 2;
		//if (vidWriter1.open("too_slow.avi", CV_FOURCC('D', 'I', 'V', 'X'), 30, sz))
		{
			int ind = 0;
			while (success)
			{
				printf("%d\n", ind++);
				Mat temp;
				cv::resize(frame1, frame1, frame.size());
				cv::hconcat(frame, frame1, temp);
				//vidWriter1.write(temp);
				cv::waitKey(33);
				imshow("stabilized", temp);
				success = cap.read(frame) && cap1.read(frame1);
			}
		}
	}
};