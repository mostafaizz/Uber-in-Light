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
#include "BGRCommunicator2.h"
class BGRSpatialCommunicator :
	public BGRCommunicator2
{
public:
	////////////////////////////// Split Amplitude ///////////////////////////
	virtual string getVideoName(string outputVideoFile)
	{
		return "_RGBSpatial_" + outputVideoFile;
	}
	//virtual void initCommunication()
	//{
	//	
	//	// we need the following
	//	// modulation rate 
	//	int modulationRate = 60;
	//	// sample width
	//	framesForSymbol = (modulationRate * Parameters::symbolTime) / 1000;
	//	
	//	ROIs = Utilities::getDivisions(Parameters::sideA, Parameters::sideB, 1, false, Parameters::globalROI, true, framesForSymbol, 1);
	//	sections = ROIs.size();
	//	vector<float> tmpWave = WaveGenerator::createWaveGivenFPS(msg, modulationRate, Parameters::symbolTime,1);
	//	int num = tmpWave.size() / ROIs.size();
	//	num *= ROIs.size();
	//	tmpWave.resize(num, 0);

	//	vector<float> finalWave(num * 2, 0);
	//	for (int i = 0; i < tmpWave.size(); i++)
	//	{
	//		finalWave[i * 2] = tmpWave[i];
	//		finalWave[i * 2 + 1] = -tmpWave[i];
	//	}

	//	amplitudes.push_back(finalWave);

	//	framesForSymbol = 2;// (Parameters::fps * Parameters::symbolTime) / 1000;
	//	amplitudes.push_back(vector<float>());
	//	amplitudes.push_back(vector<float>());
	//	for (int i = 0; i < amplitudes[0].size(); i++)
	//	{
	//		amplitudes[1].push_back(0);
	//		amplitudes[2].push_back(-amplitudes[0][i]);
	//	}
	//	
	//	
	//}

	/////////////////////////////////////////////////////////////////////////////////////////////
	///              //////////////      Receive     ///////////////                         ////
	/////////////////////////////////////////////////////////////////////////////////////////////
	vector<short> receive(string fileName, double ROI_Ratio)
	{
		int modulationFreq = 60;// (500 / Parameters::sideA);
		Parameters::BKGMaskThr = 300;
		int frames_per_symbol = modulationFreq * Parameters::symbolTime / 1000;

		vector<vector<float> > frames = Utilities::getVideoFrameLuminancesSplitted(fileName, ROI_Ratio, Parameters::fps,
			Parameters::sideA, Parameters::sideB, true, frames_per_symbol, 1);

		vector<vector<float> > BRDiff;
		for (int i = 0; i < frames.size(); i++)
		{
			vector<float> temp;
			for (int j = 0; j < frames[i].size(); j += 3)
			{
				temp.push_back(frames[i][j] - frames[i][j + 2]);
			}
			BRDiff.push_back(temp);
		}
		//return receiveN(BRDiff, Parameters::fps, frames_per_symbol);
		vector<short> res;
		for (int i = 1; i < BRDiff[0].size(); i += 2)
		{
			vector<float> tmpWave;
			for (int j = 0; j < BRDiff.size(); j++)
			{
				tmpWave.push_back(BRDiff[j][i]);
			}
			// then receive
			vector<short> tmpRes = receive2(tmpWave, modulationFreq, frames_per_symbol);
			res.insert(res.end(), tmpRes.begin(), tmpRes.end());
		}
		return res;
	}
};

