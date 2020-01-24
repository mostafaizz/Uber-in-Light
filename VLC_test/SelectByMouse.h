/*
Copyright (c) 2015, mostafa izz
izz.mostafa@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of MyVLC nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "Header.h"

const string MY_WINDOW_NAME = "My Window";

class SelectByMouse
{
	static vector<Point> points;
	static Mat img;
	static void CallBackFunc(int event, int x, int y, int flags, void* userdata)
	{
		if (event == EVENT_LBUTTONDOWN)
		{
			cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
			points.push_back(Point(x, y));
			Mat imgtmp = img.clone();
			cv::polylines(imgtmp, points, true, cv::Scalar(255, 0, 0));
			imshow(MY_WINDOW_NAME.c_str(), imgtmp);
			//cv::waitKey(0);
		}
	}
public:


	static vector<Point> getROI(Mat inputImage)
	{
		points.clear();
		// Read image from file 
		img = inputImage.clone();

		//if fail to read the image
		if (img.empty())
		{
			cout << "Error loading the image" << endl;
			return points;
		}

		//Create a window
		namedWindow(MY_WINDOW_NAME.c_str(), 1);

		//set the callback function for any mouse event
		setMouseCallback(MY_WINDOW_NAME.c_str(), SelectByMouse::CallBackFunc, NULL);

		//show the image
		imshow(MY_WINDOW_NAME.c_str(), img);

		// Wait until user press some key
		waitKey(0);

		destroyWindow(MY_WINDOW_NAME.c_str());
		cout << "start processing now!!!!\n";
		return points;
	}

	static void test_grabcut()
	{
		// Open another image
		Mat image;
		image = cv::imread("D:\\MSECE_IUPUI\\MSECE_IUPUI\\Testing_image\\test_fg_bg.png");

		Mat image2 = image.clone();

		// define bounding rectangle

		int width = image.cols * 0.8;
		int height = image.rows * 0.8;
		int lowerX = (image.cols - width) / 2;
		int lowerY = (image.rows - height) / 2;
		cv::Rect rectangle(lowerX, lowerY, width, height);

		cv::Mat result; // segmentation result (4 possible values)
		cv::Mat bgModel, fgModel; // the models (internally used)

		// GrabCut segmentation
		cv::grabCut(image,    // input image
			result,   // segmentation result
			rectangle,// rectangle containing foreground
			bgModel, fgModel, // models
			1,        // number of iterations
			cv::GC_INIT_WITH_RECT); // use rectangle
		cout << "oks pa dito" << endl;
		// Get the pixels marked as likely foreground
		cv::compare(result, cv::GC_PR_FGD, result, cv::CMP_EQ);
		// Generate output image
		cv::Mat foreground(image.size(), CV_8UC3, cv::Scalar(255, 255, 255));
		//cv::Mat background(image.size(),CV_8UC3,cv::Scalar(255,255,255));
		image.copyTo(foreground, result); // bg pixels not copied

		// draw rectangle on original image
		cv::rectangle(image, rectangle, cv::Scalar(255, 255, 0), 3);

		imshow("img_1.jpg", image);

		imshow("Foreground.jpg", foreground);
		Mat background = image2 - foreground;
		imshow("Background.jpg", background);
		cv::waitKey(0);
	}
};

vector<Point> SelectByMouse::points;
Mat SelectByMouse::img;