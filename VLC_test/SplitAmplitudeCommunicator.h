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
#include "Communicator.h"
class SplitAmplitudeCommunicator :
	public Communicator
{
public:
	////////////////////////////// Split Amplitude ///////////////////////////
	virtual string getVideoName(string outputVideoFile)
	{
		return "_AmpDiff_" + outputVideoFile;
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

		//ROIs = Utilities::getDivisions(1, 1, 1, false, Parameters::globalROI, true, 2,1);
	}
	virtual void sendImageMainLoop()
	{
		for (int i = 0; i < amplitudes[0].size(); i++)
		{
			Mat frame = img.clone();
			Utilities::updateFrameWithVchannel(frame, ROIs[0], amplitudes[0][i]);
			Utilities::updateFrameWithVchannel(frame, ROIs[1], amplitudes[1][i]);
			writeFrame(frame);
		}
	}
	virtual void sendVideoMainLoop()
	{
		Mat frame;
		double frameIndex = 0;
		for (int k = 0; k < amplitudes[0].size(); k++)
		{
			if (k >= frameIndex)
			{
				frameIndex += inputFrameUsageFrames;
				videoReader.read(frame);
			}
			Mat tmp;
			cv::resize(frame, tmp, Utilities::getFrameSize());
			Utilities::updateFrameWithVchannel(tmp, ROIs[0], amplitudes[0][k]);
			Utilities::updateFrameWithVchannel(tmp, ROIs[1], amplitudes[1][k]);
			writeFrame(tmp);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	///              //////////////      Receive     ///////////////                         ////
	/////////////////////////////////////////////////////////////////////////////////////////////


	// receive with a certain ROI ratio
	vector<short> receive(string fileName, double ROI_Ratio)
	{
		int fps = 0;
		vector<vector<float> > frames = Utilities::getVideoFrameLuminancesSplitted(fileName, ROI_Ratio, fps, 1,1,true,2,1);
		vector<float> amplitude_difference;
		//double avg = 0;
		for (int i = 0; i < frames[0].size(); i++)
		{
			amplitude_difference.push_back(frames[0][i] - frames[1][i]);
			//avg = std::max((double)abs(amplitude_difference[i]),avg);
		}
		//cout << "Avg amplitude = " << avg << endl;
		int frames_per_symbol = fps * Parameters::symbolTime / 1000;

		return receive2(amplitude_difference, fps, frames_per_symbol);
	}
};

