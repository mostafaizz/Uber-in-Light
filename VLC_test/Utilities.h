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

//#include "Properties.h"
#include "Header.h"
#include "SelectByMouse.h"
#include "background_subtractor.h"
#include "Parameters.h"
#include "WaveGenerator.h"
//#include "Properties.h"

//extern class Properties;

// the frequency component and the percentage it represent in the frequency components given
struct Frequency
{
	float freq;
	float percent;
};

class Utilities
{
public:
	/// This function will not check the range for the ROI
	/// this function adds alpha to the value channel in the selected ROI
	// the mask must be same size as the ROI and the colors must be 0 and 1 means use this pixel
	static Mat createVLayer(int &width, int &height, double &percentage)
	{
		//Mat aux = frame(ROI);
		//aux = (aux + percentage * 255);
		double val = percentage * 255;
		double absVal = abs(val);
		int addVal = floor(absVal);
		int sign = percentage > 0 ? 1 : -1;
		double diff = absVal - addVal;
		addVal *= sign;
		int sz = (width * height);
		int threshold = diff * sz;
		//cout << threshold << endl;
		// make sure there is no overflow or underflow
		//int tmpBGR[3];
		//std::random_device rd;
		//std::mt19937 mt(rd());
		std::mt19937 mt(19937);
		std::uniform_int_distribution<int> dist(0, sz - 1);
		// create v-layer
		Mat vChannel = Mat::zeros(height, width, CV_32SC1);
		int* vData = (int*)vChannel.data;
		for (int i = 0; i < sz; i++)
		{
			vData[i] = addVal;
			if (dist(mt) < threshold)
			{
				vData[i] += sign;
			}
			/*else
			{
				cout << i << endl;
			}*/
		}
		return vChannel;
	}
	static void updateFrameWithVchannel(Mat &frame, Rect &ROI, double percentage)
	{
		long long a = (percentage * 10000);
		long long b = ROI.width;
		long long c = ROI.height;
		long long key = (a << 30) | (b << 15) | (c);
		map<long long, Mat>::iterator vLayerIterator = Parameters::vLayers.find(key);
		if (vLayerIterator == Parameters::vLayers.end())
		{
			Parameters::vLayers[key] = createVLayer(ROI.width, ROI.height, percentage);
		}
		// and copy
		int* vData = (int*)Parameters::vLayers[key].data;
		//unsigned char* maskData = (unsigned char*)mask.data;
		unsigned char* data = (unsigned char*)frame.data;
		int rows = ROI.y + ROI.height;
		int cols = ROI.x + ROI.width;
		int inc = frame.cols - ROI.width;
		int channels = frame.channels();
		for (int r = ROI.y, i = ROI.y * frame.cols + ROI.x, j = 0; r < rows; r++)
		{
			for (int c = ROI.x; c < cols; c++, j++, i++)
			{
				// copy back
				for (int k = 0; k < channels; k++)
				{
					int ind = i * channels + k;
					//int tmp = (vData[j] * maskData[i + k] + data[ind]);
					int tmp = (vData[j] + data[ind]);
					data[ind] = tmp > 255 ? 255 : (tmp < 0) ? 0 : tmp;
				}
			}
			i += inc;
		}
	}
	static void updateFrameWithVchannel(Mat &frame, Rect &ROI, double* percentageArr)
	{

		unsigned char* data = (unsigned char*)frame.data;
		const int rows = ROI.y + ROI.height;
		const int cols = ROI.x + ROI.width;
		const int inc = frame.cols - ROI.width;
		const int channels = frame.channels();
		int *vData[4];
		for (int i = 0; i < channels; i++)
		{
			long long a = (percentageArr[i] * 10000);
			long long b = frame.cols;
			long long c = frame.rows;
			long long key = (a << 30) | (b << 15) | (c);
			map<long long, Mat>::iterator vLayerIterator = Parameters::vLayers.find(key);
			if (vLayerIterator == Parameters::vLayers.end())
			{
				Parameters::vLayers[key] = createVLayer(ROI.width, ROI.height, percentageArr[i]);
			}
			// and copy
			vData[i] = (int*)Parameters::vLayers[key].data;
			//unsigned char* maskData = (unsigned char*)mask.data;
		}
		
		for (int r = ROI.y, i = ROI.y * frame.cols + ROI.x, j = 0; r < rows; r++)
		{
			for (int c = ROI.x; c < cols; c++, j++, i++)
			{
				// copy back
				for (int k = 0; k < channels; k++)
				{
					int ind = i * channels + k;
					//int tmp = (vData[j] * maskData[i + k] + data[ind]);
					int tmp = (vData[k][j] + data[ind]);
					data[ind] = tmp > 255 ? 255 : (tmp < 0) ? 0 : tmp;
				}
			}
			i += inc;
		}
	}
	static void updateFrameWithAlpha(Mat &frame, Rect ROI, double percentage)
	{
		Mat aux = frame(ROI);
		Mat tmp = aux.clone() * 0;
		
		if (percentage < 0)
		{
			percentage = 0.99;
		}
		else
		{
			percentage = 1;
		}
		double beta = 1 - percentage;
		addWeighted(aux, percentage, tmp, beta, 0.0, aux);
	}
	static int gcd(int a, int b)
	{
		if (b == 0) return a;
		return gcd(b, a%b);
	}

	static int lcm(int a, int b)
	{
		return (a * (b / gcd(a, b)));
	}

	static int getOuputVideoFrameRate(int inputFps)
	{
		//if (Parameters::fps)
		//{
			return Parameters::fps;
		//}
		//return lcm(inputFps, Utilities::lcm(2 * FREQ[ZERO], 2 * FREQ[ONE]));
	}
	/*
	static void myft()
	{
		double Fs = 30;                    // Sampling frequency
		double T = 1 / Fs;                     // Sample time
		int L = 24;                     // Length of signal
		// Sum of a 50 Hz sinusoid and a 120 Hz sinusoid
		// t = (0:L - 1)*T;                % Time vector
		Mat inp = Mat::zeros(1, L, CV_32F);
		for (int i = 0; i < L; i++)
		{
			((float*)inp.data)[i] = sin(2 * MM_PI * 14 * (i*T));// +sin(2 * M_PI * 30 * (i*T));
		}

		Mat planes[] = { Mat_<float>(inp), Mat::zeros(inp.size(), CV_32F) };
		Mat complexI;
		merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

		Mat oup;
		cv::dft(complexI, oup);

		split(oup, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
		magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
		Mat magI = planes[0];

		for (int i = 0; i < oup.cols / 2; i++){
			//if (((float*)magI.data)[i] > .1)
				{
					cout << i * (Fs / oup.cols) << "\t" << ((float*)magI.data)[i] << endl;
				}
		}
		cout << endl;
	}*/

	// take array of inputs: luminance
	// sampling frequency: Fs
	// start from frame number: start
	// Number of frames to take: L
	// returns vector<Frequency> 
	static vector<Frequency> myft(vector<float> luminance, double Fs, int start, int L)
	{
		Mat inp = Mat::zeros(1, L, CV_32F);
		for (int i = 0; i < L; i++)
		{
			((float*)inp.data)[i] = luminance[i + start];
		}

		Mat oup;
		cv::dft(inp, oup, DFT_COMPLEX_OUTPUT);

		vector<Mat> planes;
		split(oup, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
		magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
		Mat magI = planes[0];

		vector<Frequency> ret;
		float total = 1;
		for (int i = 1; (i * (Fs / oup.cols)) <= Fs / 2; i++){
			Frequency f;
			f.freq = i * (Fs / oup.cols);
			f.percent = ((float*)magI.data)[i];
			total += f.percent;
			ret.push_back(f);
		}
		// cout << endl;
		for (int i = 0; i < ret.size(); i++){
			ret[i].percent /= total;
			// display for testing
			//cout << ret[i].freq << "\t" << ret[i].percent << endl;
		}

		return ret;
	}

	// x0,y0 are from the prev frame
	// x1,y1 are from the current frame
	// frames here are V-channels onlt in floating point
	static float getDifference(Mat &prev, Mat &frame, int x0, int y0,int x1,int y1)
	{
		if (x1 < 0 || x1 >= frame.cols)
		{
			return 10000000000;
		}
		if (y1 < 0 || y1 >= frame.rows)
		{
			return 10000000000;
		}
		// then get the real difference
		return (((float*)frame.data)[y1 * frame.cols + x1] - ((float*)prev.data)[y0 * frame.cols + x0]);
	}

	// the input frames here are the V-channels only in floating point
	static float getSmallestDifference(Mat &prev, Mat &frame, int radius, int x, int y)
	{
		// loop on all points in HSV1[2] and check the nearest point corresponds to this point
		float minimum = getDifference(prev, frame, x, y, x, y);
		// check the excact location
		for (int r = 1; r <= radius; r++)
		{
			for (int i = 0; i <= r; i++)
			{
				int j = r - i;
				if (i == 0)
				{
					// check i , j
					minimum = std::min(minimum, getDifference(prev, frame, x, y, x, y + j));
					// check i , -j
					minimum = std::min(minimum, getDifference(prev, frame, x, y, x, y - j));
				}
				else if (j == 0)
				{
					// check i , j
					minimum = std::min(minimum, getDifference(prev, frame, x, y, x + i, y));
					// check -i , j
					minimum = std::min(minimum, getDifference(prev, frame, x, y, x - i, y));
				}
				else
				{
					// check i , j
					minimum = std::min(minimum, getDifference(prev, frame, x, y, x + i, y + j));
					// check i , -j
					minimum = std::min(minimum, getDifference(prev, frame, x, y, x + i, y - j));
					// check -i , j
					minimum = std::min(minimum, getDifference(prev, frame, x, y, x - i, y + j));
					// check -i , -j
					minimum = std::min(minimum, getDifference(prev, frame, x, y, x - i, y - j));
				}
			}
		}
		return minimum;
	}

	static void getVchannel(Mat &frame, vector<float> &amplitudes, Mat &mask)
	{
		vector<Mat> BGR1;
		cv::split(frame, BGR1);
		cv::max(BGR1[0], BGR1[1], BGR1[1]);
		cv::max(BGR1[1], BGR1[2], BGR1[2]);

		float luminance = cv::mean(BGR1[2]).val[0];
		//float summation = (cv::sum((*add_mask)(ROIs[i])).val[0] / 255.0) + 1;
		float summation = (cv::sum(mask).val[0] / 255.0) + 1;
		luminance /= summation;
		if (abs(luminance) < 0.001 && amplitudes.size())
		{
			amplitudes.push_back(amplitudes[amplitudes.size() - 1]);
		}
		else
		{
			amplitudes.push_back(luminance);
		}
	}
	static void getIntensity(Mat &frame, vector<float> &amplitudes, Mat &mask)
	{
		// save the ROI
		Mat tmp = Mat::zeros(frame.size(),CV_32FC1);
		int sz = frame.cols * frame.rows;
		int channels = frame.channels();
		unsigned char * frameData = (unsigned char *)frame.data;
		float* intensityData = (float*)tmp.data;
		for (int i = 0; i < sz; i++)
		{
			for (int j = 0; j < channels; j++)
			{
				intensityData[i] += frameData[i*channels + j];
			}
		}
		float luminance = cv::mean(tmp).val[0];
		//float summation = (cv::sum((*add_mask)(ROIs[i])).val[0] / 255.0) + 1;
		float summation = (cv::sum(mask).val[0] / 255.0) + 1;
		luminance /= summation;
		if (abs(luminance) < 0.001 && amplitudes.size())
		{
			amplitudes.push_back(amplitudes[amplitudes.size() - 1]);
		}
		else
		{
			amplitudes.push_back(luminance);
		}
	}

	// get differnce between neighbour frames
	// and get difference between B and R channels
	// the output matrix is integer
	static void getDiffBetweenFramesBR_G(Mat &prev, Mat &frame, cv::Rect &roi, vector<float> &amplitudes, Mat &mask, Mat &ret)
	{
		//Mat ret = Mat::zeros(roi.height, roi.width, CV_32SC1);
		long long luminance = 0;
		int* data = ((int*)ret.data);
		unsigned char * p = ((unsigned char*)prev.data);
		unsigned char * f = ((unsigned char*)frame.data);
		int sz = ret.cols * ret.rows;
		int rows = roi.y + roi.height;
		int cols = roi.x + roi.width;
		int j = 0;
		int i = roi.y * frame.cols + roi.x;
		int inc = frame.cols - roi.width;
		int channels = frame.channels();
		for (int r = roi.y; r < rows; r++)
		{
			for (int c = roi.x; c < cols; c++, j++, i++)
			{
				int a = f[i * channels + 1] + p[i * channels] + p[i * channels + 2];
				int b = p[i * channels + 1] + f[i * channels] + f[i * channels + 2];
				luminance += (a - b);
				data[j] = (a - b);
			}
			i += inc;
		}

		//float luminance = cv::mean(ret).val[0];
		//float summation = (cv::sum((*add_mask)(ROIs[i])).val[0] / 255.0) + 1;
		//float summation = (cv::sum(mask).val[0] / 255.0) + 1;
		//luminance /= summation;
		if (abs(luminance) < 0.001 && amplitudes.size())
		{
			amplitudes.push_back(amplitudes[amplitudes.size() - 1]);
		}
		else
		{
			amplitudes.push_back(luminance);
		}
		//return ret;
	}
	// get differnce between neighbour frames
	// and get difference between B and R channels
	static void getDiffInBGR(Mat &prev, Mat &frame, cv::Rect &roi, vector<float> &amplitudes, int thresh = 300, vector<Mat*> ret = vector<Mat*>(3, 0))
	{
		// new method
		unsigned char * p = ((unsigned char*)prev.data);
		unsigned char * f = ((unsigned char*)frame.data);
		int sz = frame.cols * frame.rows;
		int rows = roi.y + roi.height;
		int cols = roi.x + roi.width;
		int j = 0;
		int i = roi.y * frame.cols + roi.x;
		int inc = frame.cols - roi.width;
		int channels = frame.channels();
		long long sumBGR[3] = { 0, 0, 0 };
		long long countBGR[3] = { 0, 0, 0 };
		int* retData[3];
		for (int color = 0; color < channels; color++)
		{
			if (ret[color] != 0)
			{
				retData[color] = (int*)ret[color]->data;
			}
		}
		multiset<int> medians[3];
		for (int r = roi.y; r < rows; r++)
		{
			for (int c = roi.x; c < cols; c++, j++, i++)
			{
				for (int color = 0; color < channels; color++)
				{
					int tempColor = (int)f[i * channels + color] - (int)p[i * channels + color];
					int testColor = (tempColor > 0) ? tempColor : -tempColor;
					if (testColor <= thresh)
					{
						if (ret[color] != 0)
						{
							retData[color][j] = tempColor;
						}
						sumBGR[color] += tempColor;
						//medians[color].insert(tempColor);
						countBGR[color]++;
					}
				}
			}
			i += inc;
		}
		for (int color = 0; color < 3; color++)
		{
			float val = sumBGR[color];
			amplitudes.push_back(val / countBGR[color]);
		}
		// try to return median
		/*
		for (int color = 0; color < 3; color++)
		{
		multiset<int>::iterator itr = medians[color].begin();
		for (int j = 0; j < countBGR[color] / 2; j++, itr++);

		amplitudes.push_back(*itr);
		}*/
	}
	// get differnce between neighbour frames
	// and get difference between B and R channels
	static void getDiffInBGRnext(Mat &prev, Mat &frame, Mat& next, cv::Rect &roi, vector<float> &amplitudes, int thresh = 10, vector<Mat*> ret = vector<Mat*>(3, 0))
	{
		// new method
		unsigned char * p = ((unsigned char*)prev.data);
		unsigned char * f = ((unsigned char*)frame.data);
		unsigned char * n = ((unsigned char*)next.data);
		int sz = frame.cols * frame.rows;
		int rows = roi.y + roi.height;
		int cols = roi.x + roi.width;
		int j = 0;
		int i = roi.y * frame.cols + roi.x;
		int inc = frame.cols - roi.width;
		int channels = frame.channels();
		long long sumBGR[3] = { 0, 0, 0 };
		long long countBGR[3] = { 0, 0, 0 };
		int* retData[3];
		for (int color = 0; color < channels; color++)
		{
			if (ret[color] != 0)
			{
				retData[color] = (int*)ret[color]->data;
			}
		}
		for (int r = roi.y; r < rows; r++)
		{
			for (int c = roi.x; c < cols; c++, j++, i++)
			{
				for (int color = 0; color < channels; color++)
				{
					int tempColor = (int)f[i * channels + color] - (int)p[i * channels + color];
					int testColor = (tempColor > 0) ? tempColor : -tempColor;
					if (testColor <= thresh)
					{
						if (ret[color] != 0)
						{
							retData[color][j] = tempColor;
						}
						sumBGR[color] += tempColor;
						countBGR[color]++;
					}
					else
					{
						// check with the next frame
						tempColor = (int)f[i * channels + color] - (int)n[i * channels + color];
						testColor = (tempColor > 0) ? tempColor : -tempColor;
						if (testColor <= thresh)
						{
							if (ret[color] != 0)
							{
								retData[color][j] = tempColor;
							}
							sumBGR[color] += tempColor;
							countBGR[color]++;
						}
					}
				}
			}
			i += inc;
		}
		for (int color = 0; color < 3; color++)
		{
			float val = sumBGR[color];
			amplitudes.push_back(val / countBGR[color]);
		}
	}
	// get differnce between neighbour frames
	// and get difference between B and R channels
	static void getDiffBetweenFramesSeprateBR(Mat &prev, Mat &frame, cv::Rect &roi, vector<float> &amplitudes)
	{
		// save the ROI
		Mat tmp, tmp1, tmp2;
		vector<Mat> HSV1, HSV2;

		// new method
		//Mat ret = Mat::zeros(roi.height, roi.width, CV_32SC1);
		//int* data = ((int*)ret.data);
		long long sum1 = 0, sum2 = 0;
		unsigned char * p = ((unsigned char*)prev.data);
		unsigned char * f = ((unsigned char*)frame.data);
		int sz = frame.cols * frame.rows;
		int rows = roi.y + roi.height;
		int cols = roi.x + roi.width;
		int j = 0;
		int i = roi.y * frame.cols + roi.x;
		int inc = frame.cols - roi.width;
		int channels = frame.channels();
		for (int r = roi.y; r < rows; r++)
		{
			for (int c = roi.x; c < cols; c++, j++, i++)
			{
				sum1 += f[i * channels];
				sum1 -= p[i * channels];
				sum2 += f[i * channels + 2];
				sum2 -= p[i * channels + 2];
			}
			i += inc;
		}
		int add1 = 0, add2 = 0;
		/*if (amplitudes.size() >= 2)
		{
			add1 = *amplitudes.rbegin();
			add2 = *(amplitudes.rbegin() + 1);
		}*/
		amplitudes.push_back(sum1 + add2);
		amplitudes.push_back(sum2 + add1);
	}
	// the input frames here are the original frames
	static void getDiffInVchannelHSV(Mat &prev, Mat &frame, cv::Rect &roi, vector<float> &amplitudes, Mat &mask)
	{
		// save the ROI
		Mat tmp, tmp1, tmp2;
		vector<Mat> HSV1, HSV2;	

		// new method
		Mat ret = Mat::zeros(roi.height, roi.width, CV_32F);
		float* data = ((float*)ret.data);
		unsigned char * p = ((unsigned char*)prev.data);
		unsigned char * f = ((unsigned char*)frame.data);
		int sz = ret.cols * ret.rows;
		int rows = roi.y + roi.height;
		int cols = roi.x + roi.width;
		int j = 0;
		int i = roi.y * frame.cols + roi.x;
		int inc = frame.cols - roi.width;
		int channels = frame.channels();
		for (int r = roi.y; r < rows; r++)
		{
			for (int c = roi.x; c < cols; c++,j++,i++)
			{
				// previous
				int max1 = p[i * channels];
				// frame
				int max2 = f[i * channels];
				for (int k = 1; k < channels; k++)
				{
					if (max1 < p[i * channels + k])
					{
						max1 = p[i * channels + k];
					}

					if (max2 < f[i * channels + k])
					{
						max2 = f[i * channels + k];
					}
				}
				data[j] = (max2 - max1);
			}
			i += inc;
		}

		float luminance = cv::mean(ret).val[0];
		//float summation = (cv::sum((*add_mask)(ROIs[i])).val[0] / 255.0) + 1;
		float summation = (cv::sum(mask).val[0] / 255.0) + 1;
		luminance /= summation;
		if (abs(luminance) < 0.001 && amplitudes.size())
		{
			amplitudes.push_back(amplitudes[amplitudes.size() - 1]);
		}
		else
		{
			amplitudes.push_back(luminance);
		}
	}


	// convert video fps to certain given fps
	static void convertVideo(string inputVideo, string outputVideo, double newFPS, double starting_second, double ending_second)
	{
		VideoCapture videoReader(inputVideo);
		if (videoReader.isOpened())
		{
			int framerate;
#ifdef __unix__         
			framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
			framerate = 0.5 + videoReader.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
			int frame_width = videoReader.get(CV_CAP_PROP_FRAME_WIDTH);
			int frame_height = videoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
			while (frame_height > 1000)
			{
				frame_width /= 2;
				frame_height /= 2;
			}
			int starting_frame = starting_second * framerate;
			int ending_frame = ending_second * framerate - 1;
			videoReader.set(CV_CAP_PROP_POS_FRAMES, starting_frame); //Set index to last frame

			VideoWriter vidWriter;
			vidWriter.open(outputVideo, CV_FOURCC('D', 'I', 'V', 'X'), newFPS, cv::Size(frame_width, frame_height));
			Mat frame;
			int currrentFrame = starting_frame;
			while (videoReader.read(frame) && currrentFrame++ < ending_frame)
			{
				Mat tmp;
				cv::resize(frame, tmp, cv::Size(frame_width, frame_height));
				vidWriter << tmp;
			}
		}
	}
	// convert video fps to certain given fps
	static void repeatVideo(string inputVideo, string outputVideo, double fps, int numberOfRepetitions, double starting_second, double ending_second)
	{
		VideoCapture videoReader(inputVideo);
		if (videoReader.isOpened())
		{
			int numberOfFrames = videoReader.get(CV_CAP_PROP_FRAME_COUNT); // get frame count
			int framerate;
#ifdef __unix__         
			framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
			framerate = 0.5 + videoReader.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
			int frame_width = videoReader.get(CV_CAP_PROP_FRAME_WIDTH);
			int frame_height = videoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
			while (frame_height > 1000)
			{
				frame_width /= 2;
				frame_height /= 2;
			}
			int starting_frame = starting_second * framerate;
			int ending_frame = ending_second * framerate - 1;
			
			videoReader.set(CV_CAP_PROP_POS_FRAMES, starting_frame); //Set index to last frame
			VideoWriter vidWriter;
			vidWriter.open(outputVideo, CV_FOURCC('D', 'I', 'V', 'X'), fps, cv::Size(frame_width, frame_height));
			Mat frame;
			for (int i = 0; i < numberOfRepetitions;i++)
			{
				printf("%d\n", i);
				if (i & 1)
				{
					// move backward
					videoReader.set(CV_CAP_PROP_POS_FRAMES, ending_frame);
					for (int j = ending_frame; j >= starting_frame && videoReader.read(frame); j--)
					{
						videoReader >> frame;
						Mat tmp;
						cv::resize(frame, tmp, cv::Size(frame_width, frame_height));
						vidWriter << tmp;
						videoReader.set(CV_CAP_PROP_POS_FRAMES, j - 1);
					}
				}
				else
				{
					// move forward
					videoReader.set(CV_CAP_PROP_POS_FRAMES, starting_frame);
					for (int j = starting_frame; j <= ending_frame && videoReader.read(frame); j++)
					{
						//printf("%d\n", j);
						Mat tmp;
						cv::resize(frame, tmp, cv::Size(frame_width, frame_height));
						//for (int k = 0; k < 5; k++)
						{
							vidWriter << tmp;
						}
					}
				}
				
			}
		}
	}

	// convert video fps to certain given fps
	static void testVideoBackGround(string inputVideo, string outputVideo, double fps, double starting_second, double ending_second)
	{
		VideoCapture videoReader(inputVideo);
		if (videoReader.isOpened())
		{
			int numberOfFrames = videoReader.get(CV_CAP_PROP_FRAME_COUNT); // get frame count
			int framerate;
#ifdef __unix__         
			framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
			framerate = 0.5 + videoReader.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
			int frame_width = videoReader.get(CV_CAP_PROP_FRAME_WIDTH);
			int frame_height = videoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
			while (frame_height > 1000)
			{
				frame_width /= 2;
				frame_height /= 2;
			}
			int starting_frame = starting_second * framerate;
			int ending_frame = ending_second * framerate - 1;

			videoReader.set(CV_CAP_PROP_POS_FRAMES, starting_frame); //Set index to last frame
			VideoWriter vidWriter;
			vidWriter.open(outputVideo, CV_FOURCC('D', 'I', 'V', 'X'), fps, cv::Size(frame_width * 2, frame_height * 2));
			Mat frame;
			videoReader.set(CV_CAP_PROP_POS_FRAMES, starting_frame);
			Mat prev;
			if (videoReader.read(prev))
			{
				cv::resize(prev, prev, cv::Size(frame_width, frame_height));
				for (int j = starting_frame; j <= ending_frame && videoReader.read(frame); j++)
				{
					//printf("%d\n", j);
					Mat tmp;
					cv::resize(frame, tmp, cv::Size(frame_width, frame_height));

					Mat output = Mat::zeros(frame_height * 2, frame_width * 2, prev.type());
					prev.copyTo(output(cv::Rect(0, 0, frame_width, frame_height)));
					cv::putText(output, "Previous Frame", cv::Point(30, 30), FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 0), 2);
					tmp.copyTo(output(cv::Rect(frame_width, 0, frame_width, frame_height)));
					cv::putText(output, "Current Frame", cv::Point(30 + frame_width, 30), FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 0), 2);
					Mat old_mask = MaskFactory::getBackgroundMaskAlgo(prev, tmp);
					cvtColor(old_mask, output(cv::Rect(0, frame_height, frame_width, frame_height)), CV_GRAY2BGR);
					Mat new_mask = MaskFactory::getBackgroundMask(prev, tmp);
					cvtColor(new_mask, output(cv::Rect(frame_width, frame_height, frame_width, frame_height)), CV_GRAY2BGR);
					cv::putText(output, "Old Mask", cv::Point(30, 30 + frame_height), FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 0), 2);
					cv::putText(output, "New Mask", cv::Point(30 + frame_width, 30 + frame_height), FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 0), 2);
					Mat tmp2;
					cv::resize(output, tmp2, getFrameSize());
					//imshow("output", tmp2);
					//cv::waitKey(0);
					vidWriter << output;

					prev = tmp.clone();
				}
			}
		}
	}

	// calculate correlation in video frames
	static void correlateVideoDifference(string inputVideo, double starting_second, double ending_second, int n)
	{
		VideoCapture videoReader(inputVideo);
		if (videoReader.isOpened())
		{
			Mat frame, next;
			int numberOfFrames = videoReader.get(CV_CAP_PROP_FRAME_COUNT); // get frame count
			int framerate;
#ifdef __unix__         
			framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
			framerate = 0.5 + videoReader.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
			int frame_width = videoReader.get(CV_CAP_PROP_FRAME_WIDTH);
			int frame_height = videoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
			int starting_frame = starting_second * framerate;
			int ending_frame = ending_second * framerate - 1;
			videoReader.set(CV_CAP_PROP_POS_FRAMES, starting_frame); //Set index to last frame
			vector<double> correlations(n + 1, 0);
			vector<double> correlations_RGB(n + 1, 0);
			vector<int> count(n + 1, 0);
			for (int i = 1; i <= n; i++)
			{
				cout << i << endl;
				videoReader.set(CV_CAP_PROP_POS_FRAMES, starting_frame);
				for (int j = starting_frame; j <= ending_frame && videoReader.read(frame); j++)
				{
					videoReader.set(CV_CAP_PROP_POS_FRAMES, j + i);
					if (videoReader.read(next))
					{
						vector<float> diff;
						Mat mask = Mat::zeros(frame.size(), CV_8UC1) + 255;
						Rect tmpRect = cv::Rect(0, 0, frame_width, frame_height);
						getDiffInVchannelHSV(frame, next, tmpRect, diff, mask);
						correlations[i] += diff[0];
						
						frame.convertTo(frame, CV_32F);
						vector<Mat> BGR1;
						cv::split(frame, BGR1);
						BGR1[0] = BGR1[0] + BGR1[1];
						BGR1[0] = BGR1[0] + BGR1[2];
						next.convertTo(next, CV_32F);
						vector<Mat> BGR2;
						cv::split(next, BGR2);
						BGR2[0] = BGR2[0] + BGR2[1];
						BGR2[0] = BGR2[0] + BGR2[2];
						correlations_RGB[i] += abs(cv::mean(BGR2[0] - BGR1[0]).val[0]);
						
						count[i]++;
					}
					videoReader.set(CV_CAP_PROP_POS_FRAMES, j+1);
				}
			}
			ofstream ofstr_RGB("correlation_RGB.txt");
			ofstream ofstr("correlation.txt");
			for (int i = 1; i <= n; i++)
			{
				ofstr << 100*correlations[i] / (count[i]*255) << endl;
				ofstr_RGB << 100*correlations_RGB[i] / (count[i] * 255 * 3) << endl;
			}
			ofstr.close();
			ofstr_RGB.close();
		}
	}
	// compare two videos that should be identical
	static void compareVideos(string video1, string video2)
	{
		VideoCapture vid1(video1);
		VideoCapture vid2(video2);
		if (vid1.isOpened())
		{
			if (vid2.isOpened())
			{
				Mat frame1,frame2;
				while (vid1.read(frame1) && vid2.read(frame2))
				{
					imshow("diff", 255*(frame2 - frame1));
					cv::waitKey(0);
				}
			}
		}
	}

	// add dummy seconds to video
	static void addDummyFramesToVideo(VideoWriter &vidWriter, int n, Mat dummyFrame = Mat::zeros(getFrameSize(),CV_8UC3))
	{
		for (int i = 0; i < n; i++)
		{
			writeFrame(vidWriter,dummyFrame);
		}
	}

	static vector<float> createPreambleWave()
	{
		double amplitude = Parameters::symbolsData.allData[0].amplitudeRGB[0];
		vector<float> wave(Parameters::fps / 4, 0);
		vector<float> tmp = WaveGenerator::createSampledSquareWave(Parameters::fps, Parameters::fps / 4, 12, amplitude, -amplitude);
		wave.insert(wave.end(), tmp.begin(), tmp.end());
		wave.resize(Parameters::fps * 3 / 4, 0);
		tmp = WaveGenerator::createSampledSquareWave(Parameters::fps, Parameters::fps / 4, 9, 0.008, -0.008);
		wave.insert(wave.end(), tmp.begin(), tmp.end());
		wave.push_back(0);

		return wave;
	}

	static vector<float> createInterSynchWave()
	{
		double amplitude = Parameters::symbolsData.allData[0].amplitudeRGB[0];
		vector<float> wave = WaveGenerator::createSampledSquareWave(Parameters::fps, Parameters::fps / 5, 14, amplitude, -amplitude);
		wave.push_back(0);

		return wave;
	}

	// this method is combining the system parameters into a string to be used in the output video name
	static string createOuputVideoName(string inputMessage,string inputVideoFile,string outputVideoFile)
	{
		ostringstream outputVideoStream;
		outputVideoStream << inputMessage << "_" << Parameters::symbolsData.toString() << "_";
		outputVideoStream << Parameters::getSide() << "_";
		outputVideoStream << Parameters::getFull() << "_";
		outputVideoStream << Parameters::symbolTime << "ms_" << "levels_";
		outputVideoStream << Parameters::getCodec() << "_" << inputVideoFile << "_";
		outputVideoStream << Parameters::getSynch() << "_";
		string str = outputVideoStream.str();
		std::string::iterator end_pos = std::remove(str.begin(), str.end(), ' ');
		str.erase(end_pos, str.end());
		end_pos = std::remove(str.begin(), str.end(), '.');
		str.erase(end_pos, str.end());
		end_pos = std::remove(str.begin(), str.end(), '\\');
		str.erase(end_pos, str.end());
		end_pos = std::remove(str.begin(), str.end(), ':');
		str.erase(end_pos, str.end());
		end_pos = std::remove(str.begin(), str.end(), ' ');
		str.erase(end_pos, str.end());
		return str + outputVideoFile;
	}

	static void extractOneFrameLuminance(Mat *add_mask, vector<cv::Rect> &ROIs, 
		vector<vector<float> > &frames,Mat &tmp_frame,Mat &tmp_prev,int i)
	{
		switch (Parameters::amplitudeExtraction)
		{
		case ALPHA_CHANNEL:
		{
			Mat frame1 = tmp_frame(ROIs[i]);
			Mat tmpMask = (*add_mask)(ROIs[i]);
			Utilities::getIntensity(frame1, frames[i], tmpMask);
			break;
		}
		case V_CHANNEL:
		{
			Mat frame1 = tmp_frame(ROIs[i]);
			Mat tmpMask = (*add_mask)(ROIs[i]);
			Utilities::getVchannel(frame1, frames[i], tmpMask);
			break;
		}
		case V_CHANNEL_DIFF:
		{
			Mat tmpMask = (*add_mask)(ROIs[i]);
			Utilities::getDiffInVchannelHSV(tmp_prev, tmp_frame, ROIs[i], frames[i], tmpMask);
			break;
		}
		case BGR_CHANNELS:
			Utilities::getDiffInBGR(tmp_prev, tmp_frame, ROIs[i], frames[i]);
			break;
		}
	}

	// crop means cropping from the borders and keep the given percentage (1, 100)
	static bool ReadNextFrame(VideoCapture &cap, Mat &frame,int crop = 0, bool transform = true)
	{
		Mat img;
		if (cap.read(img))
		{
			if (crop)
			{

			}
			if (transform && Parameters::homography.cols > 0)
			{
				cv::warpPerspective(img, frame, Parameters::homography, Parameters::DefaultFrameSize);
			}
			else
			{
				frame = img;
			}
			/*imshow("frame", frame);
			cv::waitKey(0);*/
			return true;
		}
		cout << "False" << endl;
		return false;
	}

	static double extractG_BR(Mat &frame, Rect &roi)
	{
		// new method
		unsigned char * f = ((unsigned char*)frame.data);
		int sz = frame.cols * frame.rows;
		int rows = roi.y + roi.height;
		int cols = roi.x + roi.width;
		int j = 0;
		int i = roi.y * frame.cols + roi.x;
		int inc = frame.cols - roi.width;
		int channels = frame.channels();
		int* retData[3];
		
		int sum = 0;
		for (int r = roi.y; r < rows; r++)
		{
			for (int c = roi.x; c < cols; c++, j++, i++)
			{
			
				int bgr[4] = { 0, 0, 0, 0 };
				for (int color = 0; color < channels; color++)
				{
					bgr[color] += (int)f[i * channels + color];
				}
				sum += (bgr[1] - (2 * bgr[1] + bgr[0] + bgr[2]) / 4);
			}
			i += inc;
		}
		return ((sum * 1.0) / sz);
	}

	static vector<double> extractAdditionalSynchData(Mat &frame, vector<Rect> &rois)
	{
		vector<double> ret;
		for (int i = 0; i < rois.size(); i++)
		{
			ret.push_back(extractG_BR(frame, rois[i]));
		}
		return ret;
	}

	static Mat getGradient(Mat&src_gray)
	{
		Mat grad;

		int scale = 1;
		int delta = 0;
		int ddepth = CV_16S;

		GaussianBlur(src_gray, src_gray, Size(3, 3), 0, 0, BORDER_DEFAULT);

		/// Generate grad_x and grad_y
		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y;

		/// Gradient X
		//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
		Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
		convertScaleAbs(grad_x, abs_grad_x);

		/// Gradient Y
		//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
		Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
		convertScaleAbs(grad_y, abs_grad_y);

		/// Total Gradient (approximate)
		addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
		imshow("X", abs_grad_x);
		imshow("Y", abs_grad_y);
		cv::waitKey();
		return grad;
	}
	static int getIndexFromAdditionalSynchData(Mat &frame, vector<Rect> &rois1, vector<Rect> &rois2)
	{
		// check if first or second
		// now the width is 5 cells
		vector<double> data1 = extractAdditionalSynchData(frame, rois1);
		vector<double> data2 = extractAdditionalSynchData(frame, rois2);
		double res1 = 0;
		for (int i = 0; i < data1.size(); i++)
		{
			res1 += (1 - 2 * (i&1)) * data1[i];
		}
		double res2 = 0;
		for (int i = 0; i < data2.size(); i++)
		{
			res2 += (1 - 2 * (i & 1)) * data2[i];
		}
		res1 = abs(res1);
		res2 = abs(res2);

		if (res1 > res2)
		{
			return 1;
		}
		return 0;
	}

	static int getIndexFromAdditionalSynchData(Mat &frame, vector<Rect> &rois)
	{
		// check if first or second
		// now the width is 5 cells
		vector<double> data = extractAdditionalSynchData(frame, rois);
		int testVectors[4] = { 1, -1, 1, -1 };
		double res = 0;
		for (int i = 0; i < 4; i++)
		{
			res += testVectors[i] * data[i];
		}
		//res[0] = abs(res[0]);
		//res[1] = abs(res[1]);

		if (res > 0)
		{
			return 1;
		}
		return 0;
	}

	static int getIndexFromAdditionalSynchData1(Mat &frame, vector<Rect> &rois)
	{
		// edit ROIs
		vector<Rect> newRois;
		for (int i = 0; i < rois.size(); i += 2)
		{
			// the end of i
			Rect tmp1(rois[i].x + rois[i].width - 3,rois[i].y,3,rois[i].height);
			newRois.push_back(tmp1);
			// the beginning of i + 1
			Rect tmp2(rois[i + 1].x, rois[i].y, 3, rois[i].height);
			newRois.push_back(tmp2);
		}
		// check if first or second
		// now the width is 5 cells
		vector<double> data = extractAdditionalSynchData(frame, newRois);
		int sum = 0;
		for (int i = 0; i < data.size(); i+=2)
		{
			sum += (data[i] - data[i + 1]);
		}
		if (sum > 0)
		{
			return 1;
		}
		return 0;
	}

	/// get video frames luminance
	// VideoCapture as input
	// ROI as input
	// returns vector<float> with the luminances
	static vector<vector<float> > getVideoFrameLuminances(VideoCapture &cap, vector<cv::Rect> &ROIs, double fps, cv::Rect globalROI, bool oldMethod = false, bool useAlpha = true)
	{
		Parameters::endingIndex = Parameters::startingIndex + 1;
		int ROIsSize = ROIs.size();
		vector<vector<float> > frames(ROIsSize, vector<float>());
		cout << "Processing Frames..." << endl;
		Mat frame, prev;
		ReadNextFrame(cap, prev,0);
		double test_frame_rate = fps; // 30
		//cap.read(prev);
		//prev = prev(ROI);
		double nextIndex = 0; 
		int count = 1;
		//Mat mask, prev_mask; = getBinaryMask(prev);
		//frame = prev.clone();
		vector<float> synchFrames;
		cv::Size endPatternSize = Parameters::patternsize;
		endPatternSize.width--;
		endPatternSize.height--;

		vector<float> interGreenSynch;
		Parameters::luminancesDivisionStarts.push_back(0);
		int totalLength = Parameters::totalTime * test_frame_rate + 1;
		if (Parameters::synchMethod == SYNCH_GREEN_CHANNEL || Parameters::synchMethod == SYNCH_COMBINED)
		{
			totalLength += test_frame_rate * 3;
		}
		while (ReadNextFrame(cap, frame,0))
		{
			if (!(count++ & 5) && count > totalLength && (Parameters::synchMethod == SYNCH_CHESS || Parameters::synchMethod == SYNCH_COMBINED))
			{
				//Mat temp;
				if (canDetectMyBoard(frame, endPatternSize, cv::Size(640,480)))
				{
					break;
				}
			}
			else if (Parameters::synchMethod == SYNCH_GREEN_CHANNEL)
			{
				vector<float> tmpV;
				Utilities::getDiffInBGR(prev, frame, globalROI, tmpV);
				synchFrames.push_back(tmpV[1] - tmpV[0] - tmpV[2]);
			}
			//printROI(Parameters::globalROI);
			if (Parameters::synchMethod == SYNCH_GREEN_CHANNEL || Parameters::synchMethod == SYNCH_COMBINED)
			{
				/*tmpBGR[0] /= ROIsSize;
				tmpBGR[1] /= ROIsSize;
				tmpBGR[2] /= ROIsSize;*/
				vector<float> tmpBGR;
				//getDiffInBGR(prev, frame, Rect(0,0,frame.cols, frame.rows), tmpBGR, 10);
				getDiffInBGR(prev, frame, Parameters::globalROI, tmpBGR, 10);
				
				interGreenSynch.push_back(tmpBGR[1] - tmpBGR[0] - tmpBGR[2]);
			}

			//float tmpBGR[3] = { 0, 0, 0 };
			//Mat diffMat = frame - prev;
#pragma omp parallel for
			for (int i = 0; i < ROIsSize; i++)
			{
				cv::Scalar means = cv::mean(frame(ROIs[i])) - mean(prev(ROIs[i]));
				frames[i].push_back(means[0]);
				frames[i].push_back(means[1]);
				frames[i].push_back(means[2]);
				//extractOneFrameLuminance(0, ROIs, frames, prev, frame, i);
				
			}
			
			prev = frame.clone();
		}
		// then update based on the inter synchronization
		if (Parameters::synchMethod == SYNCH_GREEN_CHANNEL || Parameters::synchMethod == SYNCH_COMBINED)
		{
			int frames_per_symbol = Parameters::fps * Parameters::symbolTime / 1000;
			int testingStart = 0;
			vector<vector<float> > signals;
			vector<float> wave = Utilities::createInterSynchWave();
			signals.push_back(wave);
			while (testingStart + frames_per_symbol < interGreenSynch.size())
			{
				vector<int> best_start(signals.size(), 0);
				vector<int> best_end(signals.size(), 0);
				vector<int> test_start(signals.size(), 0);
				testingStart += (Parameters::numSynchDataSymbols - 1) * frames_per_symbol;
				vector<double> res = calcCrossCorrelate(signals, interGreenSynch, testingStart, testingStart + 2 * frames_per_symbol, best_start, best_end, test_start);
				cout << "best_end[0] = " << best_end[0] << endl;
				testingStart += best_end[0];
				//testingStart += (Parameters::numSynchDataSymbols) * frames_per_symbol + wave.size();
				Parameters::luminancesDivisionStarts.push_back(testingStart);
			}
			/*for (int i = 0; i < Parameters::luminancesDivisionStarts.size(); i++)
			{
				cout << Parameters::luminancesDivisionStarts[i] << endl;
			}*/
		}
		Parameters::endingIndex = cap.get(CV_CAP_PROP_POS_FRAMES);
		cout << "last index = " << Parameters::endingIndex << endl;
		return frames;
	}

	// divide a frame with certain number of divisions with removing percentage from the boundaries
	// divisions: number of divisions
	// frame_width: frame width
	// frame_height: frame Height
	// percent: percentage to crop from the image
	// cropInclusive: means crop this percentage from each section after dividing while false means crop this percentage from the whole frame then divide 
	static vector<cv::Rect> getDivisions(int sideWidth,int sideHeight,double percent,bool cropInclusive,cv::Rect globalROI,
		bool translateToOriginal,int cellWidthDivisions,int cellHeightDivisions)
	{
		int frame_width, frame_height;
		frame_width = globalROI.width;
		frame_height = globalROI.height;
		vector<cv::Rect> ROIs;
		int sectionWidth = 0, sectionHeight = 0;
		if (cropInclusive)
		{
			sectionWidth = frame_width / sideWidth;
			sectionHeight = frame_height / sideHeight;
			
			for (int y = 0; y < sideHeight; y++)
			{
				for (int x = 0; x < sideWidth; x++)
				{
					// i is the base, j is the symbol index starting from the base, k is the index of the frameinside the symbol
					ROIs.push_back(cv::Rect(
						x * sectionWidth + (1 - percent)*sectionWidth + 1,
						y * sectionHeight + (1 - percent)*sectionHeight + 1,
						percent * sectionWidth - 1,
						percent * sectionHeight - 1));
				}
			}

		}
		else
		{
			sectionWidth = (frame_width * percent) / sideWidth;
			sectionHeight = (frame_height * percent) / sideHeight;
			
			int widthStart = frame_width * (1 - percent);
			int heightStart = frame_height * (1 - percent);
			for (int y = 0; y < sideHeight; y++)
			{
				for (int x = 0; x < sideWidth; x++)
				{
					// i is the base, j is the symbol index starting from the base, k is the index of the frameinside the symbol
					ROIs.push_back(cv::Rect(
						x * sectionWidth + widthStart + 1,
						y * sectionHeight + heightStart + 1,
						sectionWidth - 1,
						sectionHeight - 1));
				}
			}
		}
		if (translateToOriginal)
		{
			// shift to the global ROI
			for (int i = 0; i < ROIs.size(); i++)
			{
				ROIs[i].x += globalROI.x;
				ROIs[i].y += globalROI.y;
			}
		}
		if (cellWidthDivisions * cellHeightDivisions > 1)
		{
			vector<Rect> tempROIs;
			for (int i = 0; i < ROIs.size(); i++)
			{
				int newWidth = ROIs[i].width / cellWidthDivisions;
				int newHeight = ROIs[i].height / cellHeightDivisions;
				for (int y = 0, yy = 0; yy < cellHeightDivisions; yy++, y+=newHeight)
				{
					for (int x = 0, xx = 0; xx < cellWidthDivisions;xx++, x+=newWidth)
					{
						tempROIs.push_back(cv::Rect(ROIs[i].x + x, ROIs[i].y + y, newWidth, newHeight));
					}
				}
			}
			return tempROIs;
		}
		return ROIs;
	}

	static int getMaxSum(int *arr, int startIncInput, int endExcInput, int &startOuput, int &endOutput)
	{
		int sum_here, sum_total;
		sum_here = sum_total = arr[startIncInput];
		int start = startOuput = startIncInput, end = endOutput = startIncInput + 1;
		for (int i = startIncInput + 1; i < endExcInput; i++)
		{
			if (arr[i] > sum_here + arr[i])
			{
				sum_here = arr[i];
				start = i;
				end = i + 1;
			}
			else if (sum_here + arr[i] > arr[i])
			{
				sum_here = sum_here + arr[i];
				end = i + 1;
			}
			
			if (sum_here > sum_total)
			{
				sum_total = sum_here;
				startOuput = start;
				endOutput = end;
			}
		}
		return sum_total;
	}

	// should be int 
	static cv::Rect getMaxSum(Mat &inp, float &maxSum)
	{
		int cols = inp.cols;
		int rows = inp.rows;
		int sz = rows * cols;
		int* data = (int*)inp.data;
		// Variables to store the final output
		maxSum = INT_MIN;
		int finalLeft, finalRight, finalTop, finalBottom;

		int top, bottom, i;
		float sum;
		int start, finish;
		int * temp = new int[cols];
		// Set the left column
		for (top = 0; top < rows; ++top)
		{
			// Initialize all elements of temp as 0
			memset(temp, 0, cols*sizeof(int));
			// Set the right column for the left column set by outer loop
			for (bottom = top; bottom < rows; ++bottom)
			{
				// Calculate sum between current left and right for every row 'i'
				for (i = 0; i < cols; ++i)
				{
					temp[i] += data[i + bottom * cols];
				}
				// Find the maximum sum subarray in temp[]. The kadane() function
				// also sets values of start and finish.  So 'sum' is sum of
				// rectangle between (start, left) and (finish, right) which is the
				//  maximum sum with boundary columns strictly as left and right.
				sum = getMaxSum(temp, 0, cols,  start, finish);

				// Compare sum with maximum sum so far. If sum is more, then update
				// maxSum and other output values
				if (sum > maxSum)
				{
					maxSum = sum;
					finalLeft = start;
					finalRight = finish;
					finalTop = top;
					finalBottom = bottom + 1;
				}
			}
		}
		delete[]temp;
		//cout << "maxSum = " << maxSum << endl;
		//printf("(%d\t%d)\t(%d\t%d)\n", finalLeft, finalTop, finalRight, finalBottom);
		return cv::Rect(finalLeft, finalTop, finalRight - finalLeft, finalBottom - finalTop);
	}
	static void accumelateSum(Mat &inp, float *dp)
	{
		int cols = inp.cols;
		int rows = inp.rows;
		int sz = rows * cols;
		float* data = (float*)inp.data;
		memset(dp, 0, sz * sizeof(float));
		dp[0] = data[0];
		for (int c = 1; c < cols; c++)
		{
			dp[c] = data[c] + dp[c - 1];
		}
		for (int r = 1; r < rows; r++)
		{
			dp[r * cols] = data[r * cols] + dp[(r - 1) * cols];
			for (int c = 1; c < cols; c++)
			{
				dp[c + r * cols] = data[c + r * cols] + dp[c - 1 + r * cols] + dp[c + (r - 1) * cols] - dp[c - 1 + (r - 1) * cols];
			}
		}
	}

	static float getRectSum(float *dp,int cols,int rows,int c1,int r1,int c2,int r2)
	{
		float a = dp[r2 * cols + c2];
		float b = ((r1 > 0) ? dp[(r1 - 1) * cols + c2] : 0);
		float c = ((c1 > 0) ? dp[r2 * cols + c1 - 1] : 0);
		float d = (((c1 > 0) && (r1 > 0)) ? dp[(r1 - 1) * cols + c1 - 1] : 0);
		float sum = a - b - c + d;
		return sum;
	}

	// should be float 
	static cv::Rect getMaxSumDP(Mat &inp, float &maxSum)
	{
		int cols = inp.cols;
		int rows = inp.rows;
		int sz = rows * cols;
		float* data = (float*)inp.data;
		maxSum = FLT_MIN;
		// Variables to store the final output
		float*dp = new float[sz];
		accumelateSum(inp, dp);
		int finalLeft, finalRight, finalTop, finalBottom;
		for (int r1 = 0; r1 < rows; r1++)
		{
			for (int c1 = 0; c1 < cols; c1++)
			{
				for (int r2 = r1; r2 < rows; r2++)
				{
					for (int c2 = c1; c2 < cols; c2++)
					{
						float sum = getRectSum(dp, cols, rows, c1, r1, c2, r2);
						if (sum > maxSum)
						{
							maxSum = sum;
							finalLeft = c1;
							finalRight = c2 + 1;
							finalTop = r1;
							finalBottom = r2 + 1;
						}
					}
				}
			}
		}
		delete[]dp;
		//cout << "DP maxSum = " << maxSum << endl;
		//printf("(%d\t%d)\t(%d\t%d)\n", finalLeft, finalTop, finalRight, finalBottom);
		return cv::Rect(finalLeft, finalTop, finalRight - finalLeft, finalBottom - finalTop);
	}
	static void DetectGreenScreen(int frame_width, int frame_height, VideoCapture &cap, cv::Rect &globalROI, double &framerate, int &starting_index)
	{
		int width = frame_width;// getFrameSize().width;
		int height = frame_height;// getFrameSize().height;
		cv::Size size(width, height);
		vector<double> diffMaxSumBox;
		vector<double> accMaxSumBox;
		vector<cv::Rect> maxSumBox;
		vector<int> candidateIndex;
		cout << "GREEN\n";
		Mat prev, frame, accumelation = Mat::zeros(height, width, CV_32SC1);
		cv::Rect tempROI = cv::Rect(0, 0, width, height);
		Mat temp = Mat::zeros(height, width, CV_32SC1);
		if (cap.read(prev))
		{
			//cv::resize(prev,prev,size);
			Mat tmp_mask = Mat::zeros(prev.size(), CV_8UC1) + 255;
			int index = 1;
			while (cap.read(frame))
			{
				index++;
				vector<float> val;

				getDiffBetweenFramesBR_G(prev, frame, tempROI, val, tmp_mask, temp);
				if (val[0] < 0)
				{
					temp = -temp;
				}
			
				accumelation = accumelation + temp - 0.5;
				// for testing only
				{
					Mat bkg;// = Mat::zeros(accumelation.size(), CV_8UC1);
					accumelation.convertTo(bkg, CV_32FC1);
					cv::threshold(bkg, bkg, 0, 1, THRESH_BINARY);
					imshow("bkg", bkg);
					cv::waitKey(10);
				}
			
				float maxSumVal;
				maxSumBox.push_back(getMaxSum(accumelation, maxSumVal));
				cout << "Index = " << index << endl;
				cout << maxSumVal << endl;
				globalROI = *maxSumBox.rbegin();
				printf("(%d\t%d)\t(%d\t%d)\n", globalROI.x, globalROI.y, globalROI.width, globalROI.height);
				accMaxSumBox.push_back(maxSumVal);
				if (index > framerate / 2)
				{
					int compare = std::min((int)framerate, index);
					int errors = 0;
					for (int i = index - compare + 1; i < accMaxSumBox.size(); i++)
					{
						if (accMaxSumBox[i] < accMaxSumBox[i - 1] && accMaxSumBox[i - 1] < accMaxSumBox[i - 2])
						{
							errors++;
						}
					}
					cout << "Errors = " << errors << endl;
					if (errors < 2 && accMaxSumBox[index - 2] > accMaxSumBox[index - 3])
					{
						candidateIndex.push_back(index);
					}
					else if (candidateIndex.size() > 0)
					{
						starting_index = *candidateIndex.rbegin() - 3;
						globalROI = maxSumBox[starting_index - 2]; // because it is zeros based index and index is one more than size
						if (globalROI.width >= width / 2 && globalROI.height >= height / 2)
						{
							// and move it back to its original location
							float colScale = ((float)frame_width) / width;
							float rowScale = ((float)frame_height) / height;

							globalROI.x = globalROI.x * colScale;
							globalROI.y = globalROI.y * rowScale;
							globalROI.width = globalROI.width * colScale;
							globalROI.height = globalROI.height * rowScale;
							// and break
							break;
						}
					}
				}

				cv::rectangle(prev, *maxSumBox.rbegin(), cv::Scalar(255, 0, 0), 4);
				//cv::resize(prev, prev, Parameters::DefaultFrameSize);
				imshow("frame", prev);
				//cv::resize(ret, ret, Parameters::DefaultFrameSize);
				//imshow("acc", ret);
				cv::waitKey(10);
				//
				prev = frame.clone();
			}
		}
	}
	// calculate the best fit between two signals based on cross-correlation and return the peek value
	static vector<double> calcCrossCorrelate(vector< vector<float> > &signals, vector<float> &test, int start, int end, 
		vector<int> &best_start, vector<int> &best_end, vector<int> &test_start, int absolute = 0, int display = 0)
	{
		vector<double> bestVal(signals.size(), 0);
		if (end >= test.size())
		{
			end = test.size();
			end--;
		}
		int tsz = end - start + 1;
		int ssz = signals[0].size();
		for (int shift = -ssz + 1; shift < tsz - 1; shift++)
		{
			vector<double> sum(signals.size(), 0);
			vector<int> cnt(signals.size(), 0);
			int start_test = std::max(0, shift);
			int end_test = std::min(shift + ssz, tsz); // exclusive
			for (int j = start_test; j < end_test; j++)
			{
				for (int k = 0; k < signals.size(); k++)
				{
					sum[k] += signals[k][j - shift] * test[j + start];
					cnt[k]++;
				}
			}

			for (int k = 0; k < sum.size(); k++)
			{
				if (sum[k] > bestVal[k])
				{
					bestVal[k] = sum[k];
					best_start[k] = start_test;
					best_end[k] = end_test;
					test_start[k] = start_test - shift;
				}
			}
		}
		return bestVal;
	}
	// Finds the intersection of two lines, or returns false.
	// The lines are defined by (o1, p1) and (o2, p2).
	static bool intersection(Vec4i l1, Vec4i l2, Point2f &r)
	{
		Point2f o1(l1[0], l1[1]);
		Point2f p1(l1[2], l1[3]); 
		Point2f o2(l2[0], l2[1]);
		Point2f p2(l2[2], l2[3]);

		Point2f x = o2 - o1;
		Point2f d1 = p1 - o1;
		Point2f d2 = p2 - o2;

		float cross = d1.x*d2.y - d1.y*d2.x;
		if (abs(cross) < /*EPS*/1e-8)
			return false;

		double t1 = (x.x * d2.y - x.y * d2.x) / cross;
		r = o1 + d1 * t1;
		return true;
	}
	static bool isPointOutside(Point2f&p, int cols,int rows)
	{
		if (p.x >= 0 && p.y < cols && p.y >= 0 && p.y < rows)
		{
			return false;
		}
		return true;
	}
	// input image needs to be transformed to gray
	static cv::Rect detectanddrawhough(Mat src,Mat img)
	{
		//
		src.convertTo(src, CV_32FC1);
		Mat thr_Mat;
		threshold(src, thr_Mat, 0, 1, THRESH_BINARY);
		float *dp_src = new float[src.cols * src.rows];
		Utilities::accumelateSum(src, dp_src);
		float *dp = new float[src.cols * src.rows];
		Utilities::accumelateSum(thr_Mat, dp);
		////src *= 255;
		//src.convertTo(src, CV_8UC1);
		Mat tmp;
		cv::cvtColor(img, tmp, CV_BGR2GRAY);
		//tmp.copyTo(tmp, src);
		//src = tmp;
		Mat dst;// , cdst;
		Canny(tmp, dst, 50, 200, 3);
		//cvtColor(dst, cdst, CV_GRAY2BGR);

		// detect lines
		vector<Vec4i> lines;
		int W = std::max(img.cols, img.rows);
		cout << "Hough\n";
		HoughLinesP(dst, lines, 1, CV_PI / 180, W / 32, W / 32, W);
		int cnt = 0;
		cout << "Display\n";
		float tan_theta = 0.05;
		float alpha = 0.02;
		vector<Vec4i> dir_lines[4]; // left, right, top, bottom
		for (size_t i = 0; i < lines.size(); i++)
		{
			double a = abs(lines[i][0] - lines[i][2]); // x or columns
			double b = abs(lines[i][1] - lines[i][3]); // y or rows
			if (a == 0 || b == 0 || a < b * tan_theta || b < a * tan_theta)
			{
				if (a < b)
				{
					// vertical
					float left = Utilities::getRectSum(dp, src.cols, src.rows, 0, 0, std::min(lines[i][0], lines[i][2]), src.rows - 1);
					float right = Utilities::getRectSum(dp, src.cols, src.rows, std::max(lines[i][0], lines[i][2]), 0, src.cols - 1, src.rows - 1);
					if (left < right *alpha) 
					{
						dir_lines[0].push_back(lines[i]);
						/*cout << "cnt = " << ++cnt << endl;
						line(img, Point(lines[i][0], lines[i][1]),
							Point(lines[i][2], lines[i][3]), Scalar(0, 0, 255), 3, 8);*/
					}
					else if (right < left *alpha)
					{
						dir_lines[1].push_back(lines[i]);
						/*cout << "cnt = " << ++cnt << endl;
						line(img, Point(lines[i][0], lines[i][1]),
							Point(lines[i][2], lines[i][3]), Scalar(0, 255, 255), 3, 8);*/
					}
				}
				else
				{
					
					// horizontal
					float top = Utilities::getRectSum(dp, src.cols, src.rows, 0, 0, src.cols - 1, std::min(lines[i][1], lines[i][3]));
					float down = Utilities::getRectSum(dp, src.cols, src.rows, 0, std::max(lines[i][1], lines[i][3]), src.cols - 1, src.rows - 1);
					if (top < down * alpha)
					{
						dir_lines[2].push_back(lines[i]);
						/*cout << "cnt = " << ++cnt << endl;
						line(img, Point(lines[i][0], lines[i][1]),
							Point(lines[i][2], lines[i][3]), Scalar(255, 0, 0), 3, 8);*/
					}
					else if (down < top * alpha)
					{
						dir_lines[3].push_back(lines[i]);
						/*cout << "cnt = " << ++cnt << endl;
						line(img, Point(lines[i][0], lines[i][1]),
							Point(lines[i][2], lines[i][3]), Scalar(255, 255, 0), 3, 8);*/
					}
				}
			}
		}
		for (int i = 0; i < 4; i++)
		{
			cout << dir_lines[i].size() << endl; 
		}
		double bestRes = 0;
		vector<Point2f> bestPts;
		for (int l = 0; l < dir_lines[0].size(); l++)
		{
			for (int r = 0; r < dir_lines[1].size(); r++)
			{
				for (int u = 0; u < dir_lines[2].size(); u++)
				{
					for (int d = 0; d < dir_lines[3].size(); d++)
					{
						vector<Point2f> p(4);
						intersection(dir_lines[0][l], dir_lines[2][u], p[0]);
						if (isPointOutside(p[0], src.cols, src.rows)) continue;
						intersection(dir_lines[1][r], dir_lines[2][u], p[1]);
						if (isPointOutside(p[1], src.cols, src.rows)) continue;
						intersection(dir_lines[1][r], dir_lines[3][d], p[2]);
						if (isPointOutside(p[2], src.cols, src.rows)) continue;
						intersection(dir_lines[0][l], dir_lines[3][d], p[3]);
						if (isPointOutside(p[3], src.cols, src.rows)) continue;
						int c1 = 10000000, c2 = 0, r1 = 100000000, r2 = 0;
						for (int i = 0; i < 4; i++)
						{
							if (p[i].x < c1)
							{
								c1 = p[i].x;
							}
							if (p[i].x > c2)
							{
								c2 = p[i].x;
							}
							if (p[i].y < r1)
							{
								r1 = p[i].y;
							}
							if (p[i].y > r2)
							{
								r2 = p[i].y;
							}
						}
						double tmpRes = getRectSum(dp, src.cols, src.rows, c1, r1, c2, r2) / ((c2 - c1) * (r2 - r1));
						if (tmpRes > bestRes)
						{
							bestRes = tmpRes;
							bestPts = p;
						}
					}
				}
			}
		}
		for (int i = 0; i < 4; i++)
		{
			line(img, bestPts[i],
				bestPts[(i + 1)%4], Scalar(255, 0, 0), 3, 8);
		}
		//imshow("source", tmp);
		imshow("detected lines", img);

		waitKey(0);
		int c1 = 10000000, c2 = 0, r1 = 100000000, r2 = 0;
		for (int i = 0; i < 4; i++)
		{
			if (bestPts[i].x < c1)
			{
				c1 = bestPts[i].x;
			}
			if (bestPts[i].x > c2)
			{
				c2 = bestPts[i].x;
			}
			if (bestPts[i].y < r1)
			{
				r1 = bestPts[i].y;
			}
			if (bestPts[i].y > r2)
			{
				r2 = bestPts[i].y;
			}
		}
		return cv::Rect(c1, r1, c2 - c1 + 1, r2 - r1 + 1);
	}
	static void DetectGreenScreenCrossCorrelation(int frame_width, int frame_height, VideoCapture &cap, const int &framerate, int &starting_index)
	{
		//int sz = frame_height * frame_width;
		//int* accData = (int*)accumelation.data;

		//cv::Size size = getFrameSize();
		int width = frame_width;// getFrameSize().width;
		int height = frame_height;// getFrameSize().height;
		cv::Size size(width, height);
		cout << "GREEN Cross\n";
		Mat prev, frame, accumelation = Mat::zeros(height, width, CV_32SC1);
		cv::Rect tempROI = cv::Rect(0, 0, width, height);
		vector<float> tmpVal(3, 0); // 3 channels
		//vector<Mat> frames(1, Mat::zeros(height, width, CV_32SC1));
		//vector<Mat> test_frames;
		if (ReadNextFrame(cap, prev,0))
		{
			cv::resize(prev, prev, size);
			Mat tmp_mask = Mat::zeros(prev.size(), CV_8UC1) + 255;
			int index = 0;
			while (ReadNextFrame(cap, frame,0) && index < framerate * 6)
			{
				//test_frames.push_back(prev);
				index++;
				cv::resize(frame, frame, size);
				//Mat tmpRet[3] = { Mat::zeros(frame.size(), CV_32SC1), Mat::zeros(frame.size(), CV_32SC1), Mat::zeros(frame.size(), CV_32SC1) };
				//vector<Mat*> ret(3, 0);
				//ret[0] = &tmpRet[0];
				//ret[1] = &tmpRet[1];
				//ret[2] = &tmpRet[2];
				getDiffInBGR(prev, frame, tempROI, tmpVal, 10);
				//frames.push_back(tmpRet[1] - tmpRet[0] - tmpRet[2]);
				prev = frame.clone();
			}
			vector<float> val;
			for (int i = 0; i < tmpVal.size(); i += 3)
			{
				val.push_back(tmpVal[i + 1] - tmpVal[i] - tmpVal[i + 2]);
			}
			vector<vector<float> > signals;
			vector<float> wave = Utilities::createPreambleWave();
			signals.push_back(wave);

			vector<int> best_start(signals.size(), 0);
			vector<int> best_end(signals.size(), 0);
			vector<int> test_start(signals.size(), 0);
			vector<double> res = calcCrossCorrelate(signals, val, 0, val.size() - 1, best_start, best_end, test_start);
			//cout << best_start[0] << "\t" << best_end[0] << "\t" << test_start[0] << endl;
			int bestInd = 0;
			for (int i = 1; i < signals.size(); i++)
			{
				if (res[i] > res[bestInd])
				{
					bestInd = i;
				}
			}
			cout << best_start[bestInd] << "\t" << best_end[bestInd] << "\t" << test_start[bestInd] << "\t" << res[bestInd] << endl;
			
			//for (int i = best_start[bestInd], j = 0; i < best_end[bestInd], j < signals[bestInd].size(); i++, j++)
			//{
			//	if (val[i] > 0)
			//	{
			//		accumelation = accumelation + frames[i] - 1;
			//	}
			//	else
			//	{
			//		accumelation = accumelation - frames[i] - 1;
			//	}
			//	//cout << i << "\t" << signals[bestInd][j] << "\t" << val[i] << endl;
			//}
			starting_index += best_end[bestInd] - 2;
			//float maxSumVal = 0;
			//globalROI = detectanddrawhough(accumelation.clone(), test_frames[starting_index]);
			//globalROI = getMaxSum(accumelation, maxSumVal);
			// and move it back to its original location
			//float colScale = ((float)frame_width) / width;
			//float rowScale = ((float)frame_height) / height;

			//globalROI.x = globalROI.x * colScale;
			//globalROI.y = globalROI.y * rowScale;
			//globalROI.width = globalROI.width * colScale;
			//globalROI.height = globalROI.height * rowScale;

			//cv::rectangle(test_frames[starting_index], globalROI, cv::Scalar(0, 0, 255), 2);
			//imshow("rect", test_frames[starting_index]);
			//cv::waitKey(100);
		}
	}

	static void DetectGreenScreenFFT(int frame_width, int frame_height, VideoCapture &cap, cv::Rect &globalROI, double &framerate, int &starting_index)
	{
		//int sz = frame_height * frame_width;
		//int* accData = (int*)accumelation.data;

		//cv::Size size = getFrameSize();
		//int width = frame_width;// getFrameSize().width;
		//int height = frame_height;// getFrameSize().height;
		//cv::Size size(width,height);
		cout << "GREEN Cross\n";
		//int firstIndex = cap.get(CV_CAP_PROP_POS_FRAMES);
		//for (int l = 0; l < 3; l++)
		{
			//cap.set(CV_CAP_PROP_POS_FRAMES, firstIndex);
			Mat prev, frame, accumelation = Mat::zeros(globalROI.height, globalROI.width, CV_32SC1);
			vector<float> tmpVal(3, 0); // 3 channels
			vector<Mat> frames(1, Mat::zeros(globalROI.height, globalROI.width, CV_32SC1));
			vector<Mat> test_frames;
			if (cap.read(prev))
			{
				//cv::resize(prev, prev, size);
				int index = 0;
				while (cap.read(frame) && index < framerate * 3)
				{
					test_frames.push_back(prev);
					index++;
					//cv::resize(frame, frame, size);
					Mat tmpRet[3] = { 
						Mat::zeros(accumelation.size(), CV_32SC1), 
						Mat::zeros(accumelation.size(), CV_32SC1), 
						Mat::zeros(accumelation.size(), CV_32SC1) };
					vector<Mat*> ret(3, 0);
					ret[0] = &tmpRet[0];
					ret[1] = &tmpRet[1];
					ret[2] = &tmpRet[2];
					getDiffInBGR(prev, frame, globalROI, tmpVal, 10, ret);
					frames.push_back(tmpRet[1] - tmpRet[0] - tmpRet[2]);
					prev = frame.clone();
				}
				vector<float> val;
				for (int i = 0; i < tmpVal.size(); i += 3)
				{
					val.push_back(tmpVal[i + 1] - tmpVal[i] - tmpVal[i + 2]);
				}
				int valSize = val.size();
				cout << "Values = " << valSize << endl;
				int best_start = 0, best_end = 0;
				double bestRes = 0;
				int window_size = framerate * 5 / 6;
				for (int i = 0; i < valSize - window_size; i++)
				{
					//cout << i << endl;
					vector<Frequency> temp = Utilities::myft(val, framerate, i, window_size);
					int bestInd = 0;
					for (int j = 1; j < temp.size(); j++)
					{
						if (abs(temp[j].freq - 14) < abs(temp[bestInd].freq - 14))
						{
							if (temp[j].percent > bestRes)
							{
								cout << i << " -> " << i + window_size << " = " << temp[j].percent << endl;
								bestInd = j;
								bestRes = temp[j].percent;
								best_start = i;
								best_end = i + window_size;
							}
						}
					}
				}

				cout << best_start << "\t" << best_end << "\t" << bestRes << endl;
				for (int i = best_start; i < best_end; i++)
				{
					if (val[i] > 0)
					{
						accumelation = accumelation + frames[i] - 1;
					}
					else
					{
						accumelation = accumelation - frames[i] - 1;
					}
					//cout << i << "\t" << signals[bestInd][j] << "\t" << val[i] << endl;
				}
				starting_index = best_end;
				float maxSumVal = 0;

				cv::Rect tmpROI = getMaxSum(accumelation, maxSumVal);
				globalROI.x += tmpROI.x;
				globalROI.y += tmpROI.y;
				globalROI.width = tmpROI.width;
				globalROI.height = tmpROI.height;
				// and move it back to its original location
				/*float colScale = ((float)frame_width) / width;
				float rowScale = ((float)frame_height) / height;

				globalROI.x = globalROI.x * colScale;
				globalROI.y = globalROI.y * rowScale;
				globalROI.width = globalROI.width * colScale;
				globalROI.height = globalROI.height * rowScale;*/

				cv::rectangle(test_frames[starting_index], globalROI, cv::Scalar(0, 0, 255), 2);
				imshow("rect", test_frames[starting_index]);
				cv::waitKey(10);
			}
		}
	}

	static cv::Rect detectGlobalROIChessBoard(VideoCapture &cap, int &starting_index, int framerate)
	{
		cv::Rect globalROI(0, 0, 1, 1);
		Mat frame;
		bool detected = false;
		int countChess = 0;
		int countErrors = 0;
		cv::Size testSize(1920, 1080);
		bool shrink = false;
		for (; ReadNextFrame(cap,frame,0,false) && countErrors < framerate; starting_index++, countChess++)
		{
			//frame(cv::Rect(100, 100, frame.cols - 200, frame.rows - 200));
			// this loop to detect the first chess board
			if (!detected)
			{
				testSize = cv::Size(160, 120);
				while (!detected && testSize.width < 2600)
				{
					detected = canDetectMyBoard(frame, Parameters::patternsize, testSize);
					if (!detected)
					{
						testSize.width *= 2;
						testSize.height *= 2;
					}
				}
				if (!detected)
				{
					for (int i = 0; i < framerate - 1&& ReadNextFrame(cap, frame, 0, false); starting_index++, i++)
					{
						// Do nothing
					}
				}
			}
			if (detected)
			{
				//starting_index++;
				cv::Rect tmp = detectMyBoard(frame, testSize);
				if (tmp.width == 0)
				{
					countErrors++;
				}
				else
				{
					globalROI = tmp;
					if (Parameters::synchMethod == SYNCH_COMBINED && countChess == 1)
					{
						break;
					}
					countErrors = 0;
				}
			}
		}
		//cout << "first index = " << starting_index << endl;
		
		//for (; detected && countErrors < framerate; starting_index++, countChess++)
		//{
		//	//cout << "found chess at index = " << starting_index << endl;
		//	// this loop to detect the last chess board
		//	cv::Rect tmp = detectMyBoard(frame);
		//	if (tmp.width == 0)
		//	{
		//		countErrors++;
		//	}
		//	else
		//	{
		//		globalROI = tmp;
		//		if (Parameters::synchMethod == SYNCH_COMBINED && countChess == 1)
		//		{
		//			break;
		//		}
		//		countErrors = 0;
		//	}
		//	if (!cap.read(frame))
		//		break;
		//}
		if (Parameters::synchMethod == SYNCH_COMBINED)
		{
			starting_index -= countErrors;
		}
		else if (framerate < 45)
		{
			starting_index--;
		}
		cap.set(CV_CAP_PROP_POS_FRAMES, starting_index);
		cout << starting_index << endl;

		return globalROI;
	}

	// this function is trying to find athe chess board and detect the last frame with the chess board and to detect the global ROI as well
	static cv::Rect getGlobalROI(VideoCapture &cap, int &starting_index)
	{
		cv::Rect globalROI(0, 0, 1, 1);
		if (!cap.isOpened())  // check if we succeeded
			return globalROI;
		int framerate;
#ifdef __unix__         
		framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
		framerate = 0.5 + cap.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
		double count = cap.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
		int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
		int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
		cap.set(CV_CAP_PROP_POS_FRAMES, starting_index); //Set index to last frame
		// try to detect the chess board
		
		globalROI.width = frame_width;
		globalROI.height = frame_height;
		Parameters::DefaultFrameSize.width = frame_width;
		Parameters::DefaultFrameSize.height = frame_height;
		cout << "index before start = " << starting_index << endl;;
		if (Parameters::synchMethod == SYNCH_CHESS || Parameters::synchMethod == SYNCH_COMBINED)
		{
			globalROI = detectGlobalROIChessBoard(cap, starting_index, framerate);
		}
		
		if (Parameters::synchMethod == SYNCH_GREEN_CHANNEL || Parameters::synchMethod == SYNCH_COMBINED)
		{
			DetectGreenScreenCrossCorrelation(frame_width, frame_height, cap, framerate, starting_index);
		}
		//printROI(globalROI);
		return globalROI;
	}
	static void printROI(const Rect &globalROI)
	{
		printf("(%d\t%d)\t(%d\t%d)\n", globalROI.x, globalROI.y, globalROI.width, globalROI.height);
	}
	/// get video frames luminance (this is the split version which splits the image into two)
	// video name as input
	// percentage of the frame as input (used to get this percentage from the center of the image) and takes value from (0,1]
	// int &framerate: is output parameter
	// divisions: supports 2 and 4 only for now
	// ROI: is the area of interest only for the whole image
	static vector<vector<float> > getVideoFrameLuminancesSplitted(string videoName, double percent, int &framerate, 
		int sideA,int sideB,bool useGlobalROI,
		int cellWidthDiv,int cellHeightDiv)
	{
		vector<vector<float> > frames;
		VideoCapture cap(videoName); // open the default camera
		if (!cap.isOpened())  // check if we succeeded
			return frames;
#ifdef __unix__         
		framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
		framerate = 0.5 + cap.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
		cout << "Rounded fps = " << framerate << endl;
		Parameters::startingIndex = Parameters::start_second * framerate;
		// try to detect the chess board
		Parameters::globalROI = cv::Rect(0, 0, cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		if (useGlobalROI)
		{
			std::chrono::system_clock::time_point transmissionStartTime = std::chrono::system_clock::now();
			Parameters::globalROI = getGlobalROI(cap, Parameters::startingIndex);
			long long milli;
			milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - transmissionStartTime).count();
			cout << "Calibration Time = " << milli << " ms" << endl;
			printROI(Parameters::globalROI);
		}
		cap.set(CV_CAP_PROP_POS_FRAMES, Parameters::startingIndex);
		cout << "Index = " << Parameters::startingIndex << endl;
		// the ROI		
		vector<cv::Rect> ROIs = getDivisions(sideA,sideB, percent, false, Parameters::globalROI, true, cellWidthDiv, cellHeightDiv);
		
		frames = getVideoFrameLuminances(cap, ROIs, framerate, Parameters::globalROI);
		
		return frames;
	}

	static vector<float> getVideoFrameLuminancesOLd(string videoName, int &framerate, bool useGlobalROI,bool useAlpha)
	{
		cout << "Old method" << endl;
		VideoCapture cap(videoName); // open the default camera
		if (!cap.isOpened())  // check if we succeeded
			return vector<float>();
#ifdef __unix__         
		framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
		framerate = 0.5 + cap.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
		// try to detect the chess board
		int index = 0;
		cv::Rect globalROI(0, 0, cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		vector<cv::Rect> ROIs;
		
		if (useGlobalROI)
		{
			globalROI = getGlobalROI(cap, index);
			cap.set(CV_CAP_PROP_POS_FRAMES, index);
		}
		ROIs.push_back(globalROI);
		ROIs[0].x = ROIs[0].y = 0;
		cout << "Index = " << index << endl;
		if (useAlpha)
		{
			cout << "Alpha" << endl;
		}
		else
		{
			cout << "V-channel" << endl;
		}
		return getVideoFrameLuminances(cap, ROIs, framerate, globalROI, true, useAlpha)[0];
	}

	float getLuminanceWithMaskFromGray(Mat& frame, cv::Rect& ROI)
	{
		// get the ROI
		Mat tmp = frame(ROI);
		// convert the color for the ROI only
		cv::cvtColor(tmp, tmp, CV_BGR2GRAY);
		// get pixels with values above 127
		cv::Mat mask = (tmp > 10) / 255;
		//imshow("test", tmp.mul(mask));
		//cv::waitKey(0);
		return cv::sum(tmp.mul(mask)).val[0] / cv::sum(mask).val[0];
	}

	float getLuminanceWithMaskFromHSV(Mat& frame, cv::Rect& ROI, double percent)
	{
		// get the ROI
		Mat tmp = frame(ROI);
		// convert the color for the ROI only
		// convert the color for the ROI only
		cv::cvtColor(tmp, tmp, CV_BGR2HSV);
		vector<Mat> HSV(3);
		split(tmp, HSV);
		// get pixels with values above 127
		Mat tmp1 = HSV[2].clone();
		cv::Mat mask = (tmp1 > (255 - 255 * percent)) / 255;
		//imshow("test", tmp.mul(mask));
		//cv::waitKey(0);
		return cv::sum(tmp1.mul(mask)).val[0] / cv::sum(mask).val[0];
	}

	float getLuminanceFromGray(Mat& frame, cv::Rect& ROI)
	{
		// get the ROI
		Mat tmp = frame(ROI);
		// convert the color for the ROI only
		cv::cvtColor(tmp, tmp, CV_BGR2GRAY);
		cv::Scalar tempVal = mean(tmp);
		float myMAtMean = tempVal.val[0];

		return myMAtMean;
	}

	float getLuminanceFromHSV(Mat& frame, cv::Rect& ROI)
	{
		// get the ROI
		Mat tmp = frame(ROI);
		// convert the color for the ROI only
		cv::cvtColor(tmp, tmp, CV_BGR2HSV);
		vector<Mat> HSV(3);
		split(tmp, HSV);
		cv::Scalar tempVal = mean(HSV[2]);
		float myMAtMean = tempVal.val[0];

		return myMAtMean;
	}

	float getLuminance(Mat& frame, cv::Rect& ROI)
	{
		return getLuminanceWithMaskFromHSV(frame, ROI, 1);
	}

	/// get video frames luminance
	// video name as input
	// percentage of the frame as input (used to get this percentage from the center of the image) and takes value from (0,1]
	// int &framerate: is out parameter
	static vector<float> getVideoFrameLuminances(string videoName, int &framerate)
	{
		vector<float> frames;
		VideoCapture cap(videoName); // open the default camera
		if (!cap.isOpened())  // check if we succeeded
			return frames;
		double count = cap.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
#ifdef __unix__         
		framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
		framerate = 0.5 + cap.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
		int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
		int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
		cout << count << endl;
		//Mat edges;
		//namedWindow("edges", 1);
		cout << "Processing Frames..." << endl;
		cap.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
		int ind = 0;
		Mat frame;
		bool success = cap.read(frame);
		vector<Point> points;
		if (success)
		{
			points = SelectByMouse::getROI(frame);
		}
		int lx = 10000000, ly = 10000000, hx = 0, hy = 0;
		for (int i = 0; i < points.size(); i++)
		{
			lx = min(lx, points[i].x);
			ly = min(ly, points[i].y);
			hx = max(hx, points[i].x);
			hy = max(hy, points[i].y);
		}
		cv::Rect ROI(lx, ly, hx - lx + 1, hy - ly + 1);
		vector<cv::Rect> ROIs;
		ROIs.push_back(ROI);
		return Utilities::getVideoFrameLuminances(cap, ROIs, framerate, cv::Rect(0, 0, frame_width, frame_height))[0];
	}

	/// get video frames luminance
	// video name as input
	// percentage of the frame as input (used to get this percentage from the center of the image) and takes value from (0,1]
	// int &framerate: is output parameter
	static vector<float> getVideoFrameLuminances(string videoName, double percent, int &framerate)
	{
		vector<float> frames;
		VideoCapture cap(videoName); // open the default camera
		if (!cap.isOpened())  // check if we succeeded
			return frames;
		double count = cap.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
#ifdef __unix__         
		framerate = Parameters::fps;
#elif defined(_WIN32) || defined(WIN32) 
		framerate = 0.5 + cap.get(CV_CAP_PROP_FPS); //get the frame rate
#endif
		int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
		int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
		// the ROI
		int width = frame_width * percent;
		int height = frame_height * percent;
		int lowerX = (frame_width - width) / 2;
		int lowerY = (frame_height - height) / 2;
		VideoWriter vidWriter;
		vidWriter.open(std::to_string(percent) + ".avi", CV_FOURCC('D', 'I', 'V', 'X'), framerate, cv::Size(width, height));
		cap.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
		vector<cv::Rect> ROI;
		ROI.push_back(cv::Rect(lowerX, lowerY, width, height));
		return Utilities::getVideoFrameLuminances(cap, ROI, framerate, cv::Rect(0, 0, frame_width, frame_height))[0];
	}

	// open a video writer to use the same codec with every one
	static VideoWriter getVideoWriter(string vidName,cv::Size frameSize)
	{
		// 0 -> no compression, DIVX, H264
		unsigned long milliseconds_since_epoch =
			std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count();
		ostringstream name;
		name << milliseconds_since_epoch << "_" << vidName;
		string codec = Parameters::getCodec();
		VideoWriter vidWriter(name.str(),
			CV_FOURCC(codec[0], codec[1], codec[2], codec[3])
			, Parameters::fps, frameSize);
		return vidWriter;
	}
	static cv::Size getFrameSize()
	{
		return Parameters::DefaultFrameSize;
	}

	// calculate longest common subsequence between two strings
	static void LCS(vector<short> orig_msg, vector<short> test_msg)
	{
		int **l = new int*[orig_msg.size() + 1];
		//int lcs = -1;

		for (int i = 0; i <= orig_msg.size(); i++)
		{
			l[i] = new int[test_msg.size() + 1];
			for (int j = 0; j <= test_msg.size(); j++)
			{
				l[i][j] = 0;
			}
		}
		
		printf("%d and %d\r\n", orig_msg.size(), test_msg.size());
		for (int i = 1; i <= orig_msg.size(); i++)
		{
			for (int j = 1; j <= test_msg.size(); j++)
			{
				l[i][j] = std::max(l[i - 1][j - 1], std::max(l[i - 1][j], l[i][j - 1]));
				if (orig_msg[i - 1] == test_msg[j - 1])
				{
					l[i][j] = max(l[i - 1][j - 1] + 1, max(l[i - 1][j], l[i][j - 1]));
				}
			}
		}
		int lcs = l[orig_msg.size()][test_msg.size()];
		double percent = lcs;
		percent /= orig_msg.size();
		printf("Longest Common SubString Length = %d = %0.2f%%\r\n", lcs, 100*percent);
		// deallocate
		for (int i = 0; i <= orig_msg.size(); i++)
		{
			delete []l[i];
		}
		delete []l;
	}

	// calculate best greedy match
	static void LCS_greedy(vector<short> orig_msg, vector<short> test_msg, string videoName = "")
	{
		printf("%d and %d\r\n", orig_msg.size(), test_msg.size());
		int lcs = 0;
		int t_sz = test_msg.size();
		int o_sz = orig_msg.size();
		int best_i;
		vector<int> errors;
		vector<int> accErrors(10, 0);
		vector<int> rowErrors(Parameters::sideB, 0);
		vector<int> colErrors(Parameters::sideA, 0);
		vector<int> cellErrors(Parameters::sideA * Parameters::sideB, 0);
		vector<int> bitErrors(Parameters::getSymbolLength(), 0);
		vector<int> symbolErrors(o_sz / Parameters::getSymbolLength(), 0);
		//for (int i = 1 - t_sz; i < o_sz; i++)
		for (int i = 0; i < 1; i++)
		{
			int sum = 0;
			vector<int> tempErrors;
			for (int j = std::max(0, i); j < std::min(o_sz, t_sz + i); j++)
			{
				if (orig_msg[j] ^ test_msg[j - i])
				{
					tempErrors.push_back(j - i);
					accErrors[(j * accErrors.size()) / o_sz]++;
					rowErrors[((j / Parameters::getSymbolLength()) % (Parameters::sideA*Parameters::sideB)) / Parameters::sideA]++;
					colErrors[((j / Parameters::getSymbolLength()) % (Parameters::sideA*Parameters::sideB)) % Parameters::sideA]++;
					cellErrors[((j / Parameters::getSymbolLength()) % (Parameters::sideA*Parameters::sideB))]++;
					bitErrors[j % Parameters::getSymbolLength()]++;
					symbolErrors[j / Parameters::getSymbolLength()] = 1;
				}
				else
				{
					++sum;
				}
			}
			//cout << i << "\t" << sum << endl;
			if (sum > lcs)
			{
				lcs = sum;
				best_i = i;
				errors = tempErrors;
			}
		}
		double percent = lcs;
		percent /= orig_msg.size();
		printf("Longest Common SubString at index = %d Length = %d = %0.2f%%\r\n", best_i, lcs, 100 * percent);
		// then print error percentages
		// assume 10 slices
		for (int i = 0; i < accErrors.size(); i++)
		{
			printf("Error Percentage in range of frames from %d%% to %d%% = %0.2f%%\n", i * 100 / accErrors.size(), (i + 1) * 100 / accErrors.size(), (accErrors[i] * 100.0) / (orig_msg.size() / accErrors.size()));
		}
		puts("Row Errors");
		for (int i = 0; i < rowErrors.size(); i++)
		{
			printf("Error Percentage in row %d = %0.2f%%\n", i + 1, (rowErrors[i] * 100.0) / (orig_msg.size() / rowErrors.size()));
		}
		puts("Column Errors");
		for (int i = 0; i < colErrors.size(); i++)
		{
			printf("Error Percentage in column %d = %0.2f%%\n", i + 1, (colErrors[i] * 100.0) / (orig_msg.size() / colErrors.size()));
		}
		puts("Grid Errors");
		for (int i = 0; i < cellErrors.size(); i++)
		{
			printf("Error Percentage in Cell %d = %0.2f%%\n", i + 1, (cellErrors[i] * 100.0) / (orig_msg.size() / cellErrors.size()));
		}
		puts("Bit Errors");
		for (int i = 0; i < bitErrors.size(); i++)
		{
			printf("Error Percentage in bit %d = %0.2f%%\n", i + 1, (bitErrors[i] * 100.0) / (orig_msg.size() / bitErrors.size()));
		}
		puts("symbol errors");
		int cntErrors = 0;
		for (int i = 0; i < symbolErrors.size(); i++)
		{
			if (symbolErrors[i])
			{
				cntErrors++;
			}
		}
		printf("Symbols accuracy = %0.2f%%\n", 100*(1.0 - (cntErrors * 1.0 / symbolErrors.size())));
	}

	// create binary message from string message
	static vector<short> getBinaryMessage(string msg)
	{
		vector<short> result;
		bool convert = false;
		for (int i = 0; i < msg.length(); i++)
		{
			if (msg[i] != '0' && msg[i] != '1')
			{
				convert = true;
				break;
			}
		}
		if (convert)
		{
			for (int i = 0; i < msg.length(); i++)
			{
				for (int j = 7; j >= 0; j--)
				{
					result.push_back((msg[i] >> (7 - j)) & 1);
				}
			}
		}
		else
		{
			for (int i = 0; i < msg.length(); i++)
			{
				result.push_back(msg[i] - '0');
			}
		}
		return result;
	}
	// Get teh size from utilities
	// boarderPercentage that will be white
	// patternsize is the number of interior points
	// return BGR image
	static Mat createChessBoard(cv::Size patternSize)
	{
		double boarderPercentage = 0.95;
		Mat board = 255 * Mat::ones(Utilities::getFrameSize(), CV_8UC1);
		cv::cvtColor(board, board, CV_GRAY2BGR);
		int xStep = (board.cols * boarderPercentage) / (patternSize.width + 1);
		int yStep = (board.rows * boarderPercentage) / (patternSize.height + 1);
		int xStart = (board.cols * (1 - boarderPercentage)) / 2;
		int yStart = (board.rows * (1 - boarderPercentage)) / 2;
		for (int x = 0; x <= patternSize.width; x += 2)
		{
			for (int y = 0; y <= patternSize.height; y += 2)
			{
				cv::rectangle(board,
					cv::Point(xStart + x*xStep, yStart + y*yStep),
					cv::Point(xStart + (x + 1)*xStep, yStart + (y + 1)*yStep),
					cv::Scalar(0, 0, 0),
					CV_FILLED);

			}
		}
		for (int x = 1; x <= patternSize.width; x += 2)
		{
			for (int y = 1; y <= patternSize.height; y += 2)
			{
				cv::rectangle(board,
					cv::Point(xStart + x*xStep, yStart + y*yStep),
					cv::Point(xStart + (x + 1)*xStep, yStart + (y + 1)*yStep),
					cv::Scalar(0, 0, 0),
					CV_FILLED);

			}
		}

		return board;
	}

	static void displayChessBoardFullScreen()
	{
		namedWindow(Parameters::displayName, WINDOW_NORMAL);
		setWindowProperty(Parameters::displayName, CV_WND_PROP_FULLSCREEN, 1); //( on or off)
		Mat chess = createChessBoard(Parameters::patternsize);
		imshow(Parameters::displayName, chess);
		cv::waitKey(0);
	}

	static Rect createChessBoardDataRect(cv::Size frameSize = Parameters::DefaultFrameSize)
	{
		double boarderPercentage = 0.95;
		int xStart = (frameSize.width * (1 - boarderPercentage)) / 2;
		int yStart = (frameSize.height * (1 - boarderPercentage)) / 2;
		int w = frameSize.width * boarderPercentage;
		int h = frameSize.height * boarderPercentage;

		cv::Rect res(xStart, yStart, w, h);
		return res;
	}

	static vector<vector<Rect> > createTopBottomLayers(int num, cv::Size frameSize = Parameters::DefaultFrameSize)
	{
		Rect roi = createChessBoardDataRect(frameSize);
		vector<vector<Rect> > ret;
		Rect top(0, 0, frameSize.width, roi.y);
		ret.push_back(getDivisions(num, 1, 1, false, top, true, 1, 1));
		
		Rect bottom(0, roi.y + roi.height, frameSize.width, frameSize.height - roi.y - roi.height);
		ret.push_back(getDivisions(num, 1, 1, false, bottom, true, 1, 1));

		return ret;
	}

	static vector<Point2f> getChessBoardInternalCorners()
	{
		double boarderPercentage = 0.95;
		int cols = Utilities::getFrameSize().width;
		int rows = Utilities::getFrameSize().height;

		int xStep = (cols * boarderPercentage) / (Parameters::patternsize.width + 1);
		int yStep = (rows * boarderPercentage) / (Parameters::patternsize.height + 1);
		int xStart = (cols * (1 - boarderPercentage)) / 2;
		int yStart = (rows * (1 - boarderPercentage)) / 2;
		vector<Point2f> res;
		for (int y = 1; y <= Parameters::patternsize.height; y++)
		{
			for (int x = 1; x <= Parameters::patternsize.width; x++)
			{
				res.push_back(cv::Point2f(xStart + x*xStep, yStart + y*yStep));
			}
		}

		return res;
	}


	static void convertImgtoGray(Mat &img, Mat &gray)
	{
		if (img.cols > 640 && img.rows > 480)
		{
			gray = cv::Mat::zeros(cv::Size(640, 480), CV_8UC1); //source image
		}
		else
		{
			gray = cv::Mat::zeros(img.size(), CV_8UC1); //source image	
		}

		int channels = img.channels();
		int cols = gray.cols;
		int rows = gray.rows;
		int imgCols = img.cols;
		int imgRows = img.rows;
		int index = 0;
		unsigned char* imgData = (unsigned char*)img.data;
		unsigned char* grayData = (unsigned char*)gray.data;
		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < cols; c++)
			{
				grayData[index++] = imgData[(imgCols * ((r * imgRows) / rows) + ((c * imgCols) / cols)) * channels];
			}
		}
	}

	static double getLength(float x1, float y1, float x2, float y2)
	{
		double dx = x1 - x2;
		double dy = y1 - y2;
		return dx*dx + dy * dy;
	}

	static vector<Point2f> sortConvexHullClockWiseStartNorthWest(vector<Point2f> inp)
	{
		vector<Point2f> oup;
		convexHull(inp, oup, true, true);
		// get first point
		multimap<float, int> yMap;
		double xc = 0, yc = 0;
		for (int i = 0; i < oup.size(); i++)
		{
			xc += oup[i].x;
			yc += oup[i].y;
		}
		xc /= oup.size();
		yc /= oup.size();
		vector<Point2f> res(4,Point2f());
		vector<double> length(4, 0);
		for (int i = 0; i < oup.size(); i++)
		{
			if (oup[i].x < xc && oup[i].y < yc)
			{
				 double l = getLength(oup[i].x,oup[i].y, xc, yc);
				 if (l > length[0])
				 {
					 length[0] = l;
					 res[0] = oup[i];
				 }
			}
			if (oup[i].x > xc && oup[i].y < yc)
			{
				double l = getLength(oup[i].x, oup[i].y, xc, yc);
				if (l > length[1])
				{
					length[1] = l;
					res[1] = oup[i];
				}
			}
			if (oup[i].x > xc && oup[i].y > yc)
			{
				double l = getLength(oup[i].x, oup[i].y, xc, yc);
				if (l > length[2])
				{
					length[2] = l;
					res[2] = oup[i];
				}
			}
			if (oup[i].x < xc && oup[i].y > yc)
			{
				double l = getLength(oup[i].x, oup[i].y, xc, yc);
				if (l > length[3])
				{
					length[3] = l;
					res[3] = oup[i];
				}
			}
		}

		return res;
	}
	// img: input image in BGR
	// patternSize: interior number of corners
	// return rectangle around the calibration points
	static cv::Rect detectMyBoard(Mat &img, cv::Size testSize = cv::Size(0,0))
	{
		// create the gray image
		Mat gray; //source image
		//convertImgtoGray(img, gray);
		if (img.channels() == 3)
		{
			cv::cvtColor(img, gray, CV_BGR2GRAY);
		}
		else
		{
			gray = img.clone();
		}

		//cv::resize(gray, gray, cv::Size(640, 480));
		if (testSize.width > 1)
		{
			cv::resize(gray, gray, testSize);
		}
		vector<Point2f> corners; //this will be filled by the detected corners

		//CALIB_CB_FAST_CHECK saves a lot of time on images
		//that do not contain any chessboard corners
		bool patternfound = findChessboardCorners(gray, Parameters::patternsize, corners,
			CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
			+ CALIB_CB_FAST_CHECK);

		cv::Rect result(0, 0, 0, 0);
		if (patternfound)
		{
			cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1),
				TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
		}
		else
		{
			return result;
		}
		
		
		corners = sortConvexHullClockWiseStartNorthWest(corners);
		// translate the corners to the correct location
		float colScale = ((float)img.cols) / gray.cols;
		float rowScale = ((float)img.rows) / gray.rows;
		for (int i = 0; i < corners.size(); i++)
		{
			corners[i].x *= colScale;
			corners[i].y *= rowScale;
		}
		vector<Point2f> orig = sortConvexHullClockWiseStartNorthWest(Utilities::getChessBoardInternalCorners());
		
		//float xl = 100000, yl = 1000000, xh = 0, yh = 0;
		vector<float> dist(corners.size(), 0);
		int firstIndex = 0;
		
		Parameters::homography = findHomography(corners, orig);

		result = Utilities::createChessBoardDataRect(img.size());
		
		/*Mat frame;
		cv::warpPerspective(img, frame, Parameters::homography, img.size());
		cv::rectangle(frame, result, cv::Scalar(255, 0, 0), 2);
		cv::resize(img, img, cv::Size(640, 480));
		cv::resize(frame, frame, cv::Size(640, 480));
		imshow("orig", img);
		imshow("transform", frame);
		cv::waitKey(0);*/
		return result;
	}


	// img: input image in BGR
	// return true if board detected
	static bool canDetectMyBoard(Mat &img, cv::Size patternSize, cv::Size testSize = cv::Size(0, 0))
	{
		// create the gray image
		Mat gray;//source image
		//convertImgtoGray(img, gray);
		//imshow("chess", gray);
		//cv::waitKey(0);

		if (img.channels() == 3)
		{
			cv::cvtColor(img, gray, CV_BGR2GRAY);
		}
		else
		{
			gray = img.clone();
		}
		if (testSize.width > 1)
		{
			cv::resize(gray, gray, testSize);
		}

		vector<Point2f> corners; //this will be filled by the detected corners

		//CALIB_CB_FAST_CHECK saves a lot of time on images
		//that do not contain any chessboard corners
		return findChessboardCorners(gray, patternSize, corners,
			CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
			+ CALIB_CB_FAST_CHECK);
	}

	/*
	* I am using this function for testing
	* trying to explore the video frames and see what are the parts with the major source of error
	*/
	static void exploreVideo(string videoName)
	{
		cout << "explore : " << videoName << endl;
		VideoCapture cap(videoName);
		
		if (cap.isOpened())
		{
			Mat frame;
			int index = 0;
			bool flag = true;
			while (true)
			{
				cap.set(CV_CAP_PROP_POS_FRAMES, index);
				cap >> frame;
				cv::resize(frame, frame, getFrameSize());
				imshow("frame", frame);
				// check the color mask as well
				MaskFactory::getColorMask(frame, cv::Scalar(0, 0, 230));
				cout << index << endl;
				int key = cv::waitKey(0);
				int new_index = 0;
				switch (key)
				{
				case 27:
					flag = false;
					break;
				case 2555904:
					index++;
					break;
				case 2424832:
					index--;
					break;
				default:
					while (key >= '0' && key <= '9')
					{
						new_index = (new_index * 10) + (key - '0');
						key = cv::waitKey(0);
					}
					index = new_index;
					break;
				}
			}
		}
		else
		{
			puts("Can not open the video file!");
		}
	}
	// this method is used to get the video frames as if it was at some required frame rate like 30fps
	static vector<int> getFrameIndeces(double original_fps, double fps, int number_frames)
	{
		vector<int> result;
		double diff = original_fps / fps;
		double index = 0;
		for (int i = 0; i < number_frames; i++, index += diff)
		{
			result.push_back(index);
		}
		return result;
	}
	static Mat calcHueHistogram(Mat &src, Mat &hueVal,int hbins = 30)
	{
		Mat hsv;

		cvtColor(src, hsv, CV_BGR2HSV);

		// Quantize the hue to 30 levels
		// and the saturation to 32 levels
		
		int histSize[] = { hbins };
		// hue varies from 0 to 179, see cvtColor
		float hranges[] = { 0, 180 };

		const float* ranges[] = { hranges };
		MatND hist;
		// we compute the histogram from the 0-th and 1-st channels
		int channels[] = { 0 };
		Mat HSV[3];
		cv::split(hsv, HSV);
		hueVal = HSV[0].clone();
		calcHist(&hsv, 1, channels, Mat(), // do not use mask
			hist, 1, histSize, ranges,
			true, // the histogram is uniform
			false);
		return hist;
	}

	static	void writeFrame(VideoWriter &vidWriter, Mat &frame)
	{
		if (Parameters::liveTranmitter || Parameters::liveTranmitterCV)
		{
			while (Parameters::transmitterQueue.size() > 10)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
			Parameters::transmitterQueue.push(frame);
			//std::thread startTrans(Utilities::displayFrame);
		}
		else
		{
			vidWriter << frame;
			Parameters::outputFrameIndex++;
		}
	} 

	static void createWhiteNoiseVideo(string outputName)
	{
		std::mt19937 mt(19937);
		std::uniform_int_distribution<int> dist(0, 1);
		int length = Parameters::fps * Parameters::totalTime;
		int size = Parameters::DefaultFrameSize.width * Parameters::DefaultFrameSize.height;
		string codec = Parameters::getCodec();
		VideoWriter vidWriter(outputName, CV_FOURCC(codec[0], codec[1], codec[2], codec[3])
			, Parameters::fps, Parameters::DefaultFrameSize);
		for (int i = 0; i < length; i++)
		{
			Mat newFrame = Mat::zeros(Parameters::DefaultFrameSize, CV_8UC3);
			unsigned char* frameData = (unsigned char*)newFrame.data;
			for (int j = 0; j < size; j++)
			{
				if (dist(mt))
				{
					frameData[j * 3] = frameData[j * 3 + 1] = frameData[j * 3 + 2] = 255;
				}
			}
			vidWriter << newFrame;
		}
	}
};
