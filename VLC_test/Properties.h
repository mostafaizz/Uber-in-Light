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
#include "Parameters.h"

class Communicator;

enum
{
	SEND = 1,
	RECV,
	CNVRT,
	EXTEND,
	CORRELEATION,
	WHITE_NOISE,
	EXPLORE
};

enum
{
	TEMPORAL_CORRELATION = 0,
	SPATIAL_REDUNDANCY,
	FREQ_DIFF,
	FREQ_AMP_DIFF,
	SPLIT_SCREEN_TEMPORAL_CORRELATION,
	SPLIT_SCREEN_SPATIAL_REDUNDANCY
};

enum
{
	NO_ERROR_CORRECTION = 0,
	HAMMING,
	REED_SOLOMON
};

class Properties
{
private:
	Communicator*communicator = 0;
	static Properties* inst;
	Properties()
	{
		mode = SEND;
		outputFileName = "output.avi";
		inputFileName = "";
		ROI = 1;
		type = 0;
		text = "";
		errorCorrection = 0;
		interleave = false;
		color = false;
		correlation = 0;
	}
public:
	static Properties* getInst()
	{
		if (inst == 0)
		{
			inst = new Properties();
		}
		return inst;
	}

	int mode; // 0 for send and 1 for receive, 2 for converting video
	string inputFileName; // input file name for processing in case of receive, and input video/image file name in case of send
	string outputFileName; // used in send only
	string msgFileName; // the message file Name
	float ROI; // <= 0 means in the receiver use selection by hand and positive value means percentage
	int type; // -1->the old HiLight work(no difference),0->normal(and default),1->split amplitude,2->split frequency,3->split amplitude and frequency, 4 -> split screen, 5-> split screen and amplitude
	string text; // text to send
	vector<short> msg; // the message after conversion to vector<short>
	int extendN;
	int errorCorrection;
	bool interleave;
	bool color;
	int correlation;
	
	int returnError();
	int testSendReceive(int argc, char** argv);
	vector<cv::Mat> getSplittedImages(cv::Mat &frame);
};

Properties* Properties::inst = Properties::getInst();
