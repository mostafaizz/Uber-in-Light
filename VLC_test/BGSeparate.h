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
#include "BGRCommunicator.h"
class BGSeparate :
	public BGRCommunicator
{
public:
	////////////////////////////// Red and Blue as separate channels ///////////////////////////
	virtual string getVideoName(string outputVideoFile)
	{
		return "_BGR0_" + outputVideoFile;
	}
	virtual void initCommunication()
	{
		// divide the message into 2 parts
		vector<SymbolData> DivMsg[2];
		for (int i = 0; i < msg.size(); i++)
		{
			DivMsg[i % 2].push_back(msg[i]);
		}
		for (int i = 1; i < 2; i++)
		{
			if (DivMsg[i].size() < DivMsg[0].size())
			{
				DivMsg[i].push_back(DivMsg[0][0]);
			}
		}
		// blue
		amplitudes.push_back(WaveGenerator::createWaveGivenFPS(DivMsg[0], Parameters::fps, Parameters::symbolTime,1));
		// green
		amplitudes.push_back(WaveGenerator::createWaveGivenFPS(DivMsg[1], Parameters::fps, Parameters::symbolTime,1));
		//red
		amplitudes.push_back(vector<float>());

		// then add the green channel as the inverse of the other two channels
		for (int i = 0; i < amplitudes[0].size(); i++)
		{
			amplitudes[2].push_back(0);// -amplitudes[0][i] - amplitudes[2][i]);
		}
		framesForSymbol = (Parameters::fps * Parameters::symbolTime) / 1000;

		ROIs = Utilities::getDivisions(Parameters::sideA, Parameters::sideB, 1, false, Parameters::globalROI, true, 1,1);
		sections = Parameters::sideA * Parameters::sideB;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	///              //////////////      Receive     ///////////////                         ////
	/////////////////////////////////////////////////////////////////////////////////////////////
	vector<short> receive(string fileName, double ROI_Ratio)
	{
		vector<vector<float> > frames = Utilities::getVideoFrameLuminancesSplitted(fileName, ROI_Ratio, Parameters::fps,
			Parameters::sideA, Parameters::sideB, true, 1,1);
		vector<vector<float> > frames_BG;
		for (int i = 0; i < frames.size(); i++)
		{
			vector<float> B;
			vector<float> G;
			for (int j = 0; j < frames[i].size(); j += 3)
			{
				// blue
				B.push_back(frames[i][j]);
				// red
				G.push_back(frames[i][j + 1]);
			}
			frames_BG.push_back(B);
			frames_BG.push_back(G);
		}
		int frames_per_symbol = Parameters::fps * Parameters::symbolTime / 1000;

		return receiveN(frames_BG, Parameters::fps, frames_per_symbol);
	}
};