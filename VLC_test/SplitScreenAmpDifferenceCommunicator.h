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

#include "SplitScreenCommunicator.h"

class SplitScreenAmpDifferenceCommunicator :
	public SplitScreenCommunicator
{
public:
	////////////////////////////// Split to quarters ///////////////////////////
	virtual string getVideoName(string outputVideoFile)
	{
		ostringstream ostr;
		ostr << "_SplitAmp_" << outputVideoFile;
		return ostr.str();
	}
	
	virtual void initCommunication()
	{
		////double lumin1[] = { LUMINANCE[0], LUMINANCE[1] };
		//amplitudes.push_back(WaveGenerator::createWaveGivenFPS(msg, Parameters::fps, Parameters::symbolTime));
		////double lumin2[] = { LUMINANCE[1], LUMINANCE[0] };
		//for (int i = 0; i < msg.size(); i++)
		//{
		//	msg[i].amplitude = -msg[i].amplitude;
		//}
		//amplitudes.push_back(WaveGenerator::createWaveGivenFPS(msg, Parameters::fps, Parameters::symbolTime));
		//
		//framesForSymbol = (Parameters::fps * Parameters::symbolTime) / 1000;
		// 
		//ROIs = Utilities::getDivisions(Parameters::sideA, Parameters::sideB, 1, false, Parameters::globalROI, true, 2,1);
		//sections = Parameters::sideA* Parameters::sideB;
	}
	virtual void sendImageMainLoop()
	{
		for (int i = 0; i < amplitudes[0].size(); i += (sections * framesForSymbol))
		{
			for (int k = 0; k < framesForSymbol; k++)
			{
				Mat frame = img.clone();
				for (int j = 0; j < sections && (i + (j * framesForSymbol) + k) < amplitudes[0].size(); j++)
				{
					// i is the base, j is the symbol index starting from the base, k is the index of the frameinside the symbol
					Utilities::updateFrameWithVchannel(frame, ROIs[j * 2], amplitudes[0][i + (j * framesForSymbol) + k]);
					Utilities::updateFrameWithVchannel(frame, ROIs[j * 2 + 1], amplitudes[1][i + (j * framesForSymbol) + k]);
				}
				writeFrame(frame);
			}
		}
	}
	virtual void sendVideoMainLoop()
	{
		double frameIndex = 0;
		for (int i = 0; i < amplitudes[0].size(); i += (sections * framesForSymbol))
		{
			for (int k = 0; k < framesForSymbol; k++)
			{
				if ((i + k * sections) >= frameIndex)
				{
					frameIndex += inputFrameUsageFrames * sections;
					videoReader.read(img);
					cv::resize(img, img, Utilities::getFrameSize());
				}
				Mat frame = img.clone();
				for (int j = 0; j < sections && (i + (j * framesForSymbol) + k) < amplitudes[0].size(); j++)
				{
					// i is the base, j is the symbol index starting from the base, k is the index of the frameinside the symbol
					Utilities::updateFrameWithVchannel(frame, ROIs[j * 2], amplitudes[0][i + (j * framesForSymbol) + k]);
					Utilities::updateFrameWithVchannel(frame, ROIs[j * 2 + 1], amplitudes[1][i + (j * framesForSymbol) + k]);
				}
				writeFrame(frame);
			}
		}
	}

	// receive with a certain ROI ratio
	vector<short> receive(string fileName, double ROI_Ratio)
	{
		int fps = 0;
		vector<vector<float> > frames = Utilities::getVideoFrameLuminancesSplitted(fileName, ROI_Ratio, fps, 
			Parameters::sideA, Parameters::sideB, true, 2,1);
		vector<vector<float> > amplitude_difference;
		for (int j = 0; j < frames.size(); j += 2)
		{
			vector<float> tmp;
			for (int i = 0; i < frames[j].size(); i++)
			{
				tmp.push_back(frames[j][i] - frames[j + 1][i]);
			}
			amplitude_difference.push_back(tmp);
		}
		int frames_per_symbol = fps * Parameters::symbolTime / 1000;

		return receiveN(amplitude_difference, fps, frames_per_symbol);
	}
};

