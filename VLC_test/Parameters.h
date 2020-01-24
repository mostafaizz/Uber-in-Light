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
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "concurrent_queue.h"

using namespace std;

enum
{
	FFT_RANDOM_GUESS,
	FFT_NO_RANDOM_GUESS,
	CROSS_CORRELATION,
};
enum
{
	ALPHA_CHANNEL,
	V_CHANNEL,
	V_CHANNEL_DIFF,
	BGR_CHANNELS,
};

enum
{
	SYNCH_CHESS,
	SYNCH_GREEN_CHANNEL,
	SYNCH_COMBINED,
};

// try to read this from file
struct SymbolData
{
	string symbol;
	double amplitudeRGB[3];
	double frequency;
	double phase; // currently zeros only
	vector<short> getSymbol()
	{
		vector<short> ret;
		for (int i = 0; i < symbol.size(); i++)
		{
			ret.push_back(symbol[i] - '0');
		}
		return ret;
	}
};

struct AllSymbolsData
{
	string fileName;
	double FREQ[2];
	vector<SymbolData> allData;
	//void createGrayCodeSymbols(int maxFrequency, int differenceBetweenFreq, int numberOfFreq, double amplitude)
	//{
	//	allData.clear();
	//	SymbolData s0, s1;
	//	s0.symbol = "0"; s0.amplitude = amplitude; s0.frequency = maxFrequency;
	//	s1.symbol = "1"; s0.amplitude = amplitude; s0.frequency = maxFrequency - differenceBetweenFreq;
	//	allData.push_back(s0);
	//	allData.push_back(s1);
	//	for (int n = 4; n <= numberOfFreq; n *= 2)
	//	{
	//		// add the new symbol
	//		for (int i = allData.size() - 1; i >= 0; i--)
	//		{
	//			SymbolData newSymbol = allData[i];
	//			allData[i].symbol = "0" + allData[i].symbol;
	//			newSymbol.symbol = "1" + newSymbol.symbol;
	//			allData.push_back(newSymbol);
	//		}
	//		// adjust frequencies
	//		allData[allData.size() - 1].frequency = maxFrequency;
	//		for (int i = allData.size() - 2; i >= 0; i--)
	//		{
	//			allData[i].frequency = allData[i + 1].frequency - differenceBetweenFreq;
	//		}
	//	}
	//}
	// assume the fiel is sorted
	void readData(string fileName)
	{
		vector<SymbolData> tmp;
		ifstream ifstr(fileName);
		if (ifstr.is_open())
		{
			SymbolData obj;
			while (ifstr >> obj.symbol >> obj.frequency >> obj.amplitudeRGB[2] >> obj.amplitudeRGB[1] >> obj.amplitudeRGB[0])
			{
				tmp.push_back(obj);
			}
		}
		if (tmp.size() >= 2)
		{
			this->fileName = fileName;
			allData = tmp;
		}
	}
	string toString()
	{
		if (fileName.size() > 0)
		{
			return fileName;
		}
		ostringstream ostr;
		ostr << FREQ[0] << "Hz_" << FREQ[1] << "Hz";
		return ostr.str();
	}
	/*void addSymbol(string symbol, double frequency, double amplitude = 0.004, double phase = 0)
	{
		for (int i = 0; i < allData.size(); i++)
		{
			if (symbol == allData[i].symbol)
			{
				cout << "symbol " << symbol << "already exists!" << endl;
				return;
			}
		}
		SymbolData obj;
		obj.symbol = symbol;
		obj.frequency = frequency;
		obj.amplitude = amplitude;
		obj.phase = phase;
		allData.push_back(obj);
		cout << "symbol " << symbol << " added successfully!" << endl;
	}*/
	SymbolData* getSymbol(string symbol)
	{
		for (int i = 0; i < allData.size(); i++)
		{
			if (symbol == allData[i].symbol)
			{
				return &allData[i];
			}
		}
		return 0;
	}
	vector<SymbolData> getMsgSymbols(vector<short>& msg)
	{
		vector<SymbolData> res;
		for (int i = 0; i < msg.size();i++)
		{
			string ostr;
			for (int j = 0; i < msg.size(); j++,i++)
			{
				ostr += '0' + msg[i];
				SymbolData* symbolPtr = getSymbol(ostr);
				if (symbolPtr != 0)
				{
					res.push_back(*symbolPtr);
					ostr.clear();
					//i += j;
					break;
				}
			}
			while (ostr.size() != 0)
			{
				// this should be in the last symbol only otherwise there is something wrong
				ostr += '0'; // pad zeros
				SymbolData* symbolPtr = getSymbol(ostr);
				if (symbolPtr != 0)
				{
					res.push_back(*symbolPtr);
					ostr.clear();
				}
			}
		}
		
		
		return res;
	}
};

struct Parameters
{
	static int startingIndex;
	static int endingIndex;
	static float symbolTime;
	static cv::Rect globalROI;
	static vector<float> amplitudes;
	static int fps;
	static double ifps;
	static int DecodingMethod;
	static double LUMINANCE; // percentage
	static int mfsk; // number of frequencies used in the mfsk
	//enum{ ZERO = 0, ONE };
	
	static cv::Size DefaultFrameSize;
	static cv::Size patternsize;
	static AllSymbolsData symbolsData;
	static int sideA;
	static int sideB;
	static int CommunicatorSpecificSplit; // split = 1, original = 0
	static map<long long, cv::Mat> vLayers;
	static double start_second;
	static double end_second;
	static string endSecondFile;
	static int BKGMaskThr;
	static int fullScreen;
	static int amplitudeExtraction;
	static bool realVideo;
	static int synchMethod;
	static cv::Mat homography;
	static int liveTranmitter;
	static int liveTranmitterCV;
	static std::chrono::system_clock::time_point transmissionStartTime;
	static int outputFrameIndex;
	static Queue<cv::Mat> transmitterQueue;
	static bool done;
	static string displayName;
	static int totalTime;
	static int seed;
	static int numSynchDataSymbols;
	static vector<int> luminancesDivisionStarts;
	static string getSide()
	{
		ostringstream ostr;
		if (sideA == sideB)
		{
			ostr << "side" << sideA;
		}
		else
		{
			ostr << "sideA" << sideA << "_sideB" << sideB;
		}
		return ostr.str();
	}
	static string getSynch()
	{
		ostringstream ostr;
		ostr << "synch" << synchMethod;
		return ostr.str();
	}
	static string getFull()
	{
		ostringstream ostr;
		ostr << "full" << fullScreen;
		return ostr.str();
	}
	static bool setCodec(string code)
	{
		vector<string> codecs({"I420", "DIB", "DIVX", "XVID"});
		//cout << code << endl;
		if (find(codecs.begin(), codecs.end(),code) != codecs.end())
		{
			codec = code;
			return true;
		}
		return false;
	}
	static string getCodec()
	{
		return codec;
	}
	static int getSymbolLength()
	{
		return symbolsData.allData[0].symbol.size();
	}
private:
	static string codec; //I420, DIB ,DIVX, XVID
};
int Parameters::startingIndex = 0;
int Parameters::endingIndex = 0;
cv::Rect Parameters::globalROI = cv::Rect(0,0,1,1);
vector<float> Parameters::amplitudes = vector<float>();
int Parameters::fps = 0;
double Parameters::ifps = 0;
float Parameters::symbolTime = 600;
int Parameters::DecodingMethod = CROSS_CORRELATION;
double Parameters::LUMINANCE = 0.8; // the amplitude
int Parameters::mfsk = 8; // number of mfsk
//enum{ ZERO = 0, ONE };
string Parameters::codec = "I420"; //I420, DIB ,DIVX, XVID
cv::Size Parameters::DefaultFrameSize = cv::Size(640, 480);
cv::Size Parameters::patternsize = cv::Size(6, 6);
AllSymbolsData Parameters::symbolsData;
map<long long, cv::Mat> Parameters::vLayers;
int Parameters::sideA = 15;
int Parameters::sideB = 12;
double Parameters::start_second = 0;
double Parameters::end_second = 0;
string Parameters::endSecondFile = "";
int Parameters::CommunicatorSpecificSplit = 0;
int Parameters::BKGMaskThr = 5;
int Parameters::fullScreen = false;
int Parameters::amplitudeExtraction = V_CHANNEL_DIFF;
bool Parameters::realVideo = false;
int Parameters::synchMethod = SYNCH_CHESS;
cv::Mat Parameters::homography = cv::Mat::zeros(cv::Size(0,0), CV_8UC1);
int Parameters::liveTranmitter = 0;
int Parameters::liveTranmitterCV = 0;
std::chrono::system_clock::time_point Parameters::transmissionStartTime;
int Parameters::outputFrameIndex = 0;
Queue<cv::Mat> Parameters::transmitterQueue;
bool Parameters::done = false;
string Parameters::displayName = "Video";
int Parameters::totalTime = 0;
int Parameters::seed = 1;
int Parameters::numSynchDataSymbols = 20;
vector<int> Parameters::luminancesDivisionStarts;