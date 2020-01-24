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

class SplitScreenCommunicator :
	public Communicator
{
public:
	////////////////////////////// Split to quarters ///////////////////////////
	int sections;
	
	int framesForSymbol;
	virtual string getVideoName(string outputVideoFile)
	{
		ostringstream ostr;
		ostr << "_Split_" << outputVideoFile;
		return ostr.str();
	}

	virtual void initCommunication()
	{
		////double lumin1[] = { LUMINANCE[0], LUMINANCE[1] };
		//amplitudes.push_back(WaveGenerator::createWaveGivenFPS(msg, Parameters::fps, Parameters::symbolTime));
		//
		//framesForSymbol = (Parameters::fps * Parameters::symbolTime) / 1000;
		//
		//ROIs = Utilities::getDivisions(Parameters::sideA, Parameters::sideB, 1, false, Parameters::globalROI, true, 1,1);
		//sections = Parameters::sideA * Parameters::sideB;
	}
	virtual void sendImageMainLoop()
	{
		int amplitudes0_size = amplitudes[0].size();
		int i_increment = (sections * framesForSymbol);
		for (int i = 0; i < amplitudes0_size; i += i_increment)
		{
			for (int k = 0; k < framesForSymbol; k++)
			{
				Mat frame = img.clone();
				int innerLoopComparison = i + k;
				for (int j = 0; j < sections && innerLoopComparison < amplitudes0_size; j++, innerLoopComparison += framesForSymbol)
				{
					// i is the base, j is the symbol index starting from the base, k is the index of the frameinside the symbol
					Utilities::updateFrameWithVchannel(frame, ROIs[j], amplitudes[0][innerLoopComparison]);
				}
				writeFrame(frame);
			}
		}
	}
	virtual void sendVideoMainLoop()
	{
		double frameIndex = 0;
		double frameIndexIncrement = inputFrameUsageFrames*sections;
		int ampitudesSize = amplitudes[0].size();
		int i_increment = (sections * framesForSymbol);
		for (int i = 0; i < ampitudesSize; i += i_increment)
		{
			int frameIndexComparison = i;
			for (int k = 0; k < framesForSymbol; k++, frameIndexComparison += sections)
			{
				if (frameIndexComparison >= frameIndex)
				{
					frameIndex += frameIndexIncrement;
					videoReader.read(img);
					cv::resize(img, img, Utilities::getFrameSize());
				}
				Mat frame = img.clone();
				int innerLoopComparison = i + k;
				for (int j = 0; j < sections && innerLoopComparison < ampitudesSize; j++, innerLoopComparison += framesForSymbol)
				{
					// i is the base, j is the symbol index starting from the base, k is the index of the frameinside the symbol
					Utilities::updateFrameWithVchannel(frame, ROIs[j], amplitudes[0][innerLoopComparison]);
				}
				writeFrame(frame);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	///              //////////////      Receive     ///////////////                         ////
	/////////////////////////////////////////////////////////////////////////////////////////////
	vector<short> receiveNCombined(vector<vector<float> >& frames, int fps, int frames_per_symbol)
	{
		vector<short> results;
		if (Parameters::synchMethod == SYNCH_GREEN_CHANNEL || Parameters::synchMethod == SYNCH_COMBINED)
		{
			// divide the frames
			int divisionSize = frames_per_symbol * Parameters::numSynchDataSymbols;
			for (int i = 0; i < Parameters::luminancesDivisionStarts.size(); i++)
			{
				/*vector<vector<float> > tmpFrames(frames.size(), vector<float>());
				for (int j = 0; j < divisionSize && (j + Parameters::luminancesDivisionStarts[i]) < frames[0].size(); j++)
				{
					for (int k = 0; k < frames.size(); k++)
					{
						tmpFrames[k].push_back(frames[k][j + Parameters::luminancesDivisionStarts[i]]);
					}
				}
				vector<short> tmpRes = receiveN(tmpFrames, fps, frames_per_symbol);
				*/
				vector<short> tmpRes = receiveN(frames, fps, frames_per_symbol, Parameters::luminancesDivisionStarts[i], 
					std::min((int)(Parameters::luminancesDivisionStarts[i] + frames_per_symbol * Parameters::numSynchDataSymbols), 
					(int)frames[0].size()));
				results.insert(results.end(), tmpRes.begin(), tmpRes.end());
			}
		}
		return results;
	}

	vector<short> receiveN(vector<vector<float> >& frames, int fps,int frames_per_symbol, int start = 0, int end = 0)
	{
		sections = frames.size();// Parameters::sideA * Parameters::sideB;
		vector<short> result;
		if (frames.size() == 0)
			return result;

		vector<vector<short> > vt(sections, vector<short>());

#pragma omp parallel for
		for (int k = 0; k < sections; k++)
		{
			vt[k] = receive2(frames[k], fps, frames_per_symbol, start, end);
		}
		int symbolSize = Parameters::symbolsData.allData[0].symbol.size();
		for (int i = 0; i < vt[0].size(); i += symbolSize)
		{
			for (int j = 0; j < vt.size(); j++)
			{
				for (int k = 0; k < symbolSize; k++)
				{
					result.push_back(vt[j][i + k]);
				}
			}
		}
		return result;
	}

	// receive with a certain ROI ratio
	vector<short> receive(string fileName, double ROI_Ratio)
	{
		//Parameters::BKGMaskThr = 300;
		int fps = 0;
		vector<vector<float> > frames = Utilities::getVideoFrameLuminancesSplitted(fileName, ROI_Ratio, fps, 
			Parameters::sideA, Parameters::sideB, true, 1,1);
		int frames_per_symbol = fps * Parameters::symbolTime / 1000;

		return receiveN(frames, fps, frames_per_symbol);
	}
};

