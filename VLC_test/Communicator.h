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

#include "Utilities.h"
#include "WaveGenerator.h"

class Communicator
{
private:
	VideoWriter vidWriter;
	
protected:
	bool remainingFrames = true;
	double frameIndex = 0;
	int frameIndexComparison = 0;
	double inputFrameUsageFrames; // used for videos
	Mat img;
	vector<vector<float> > amplitudes;
	vector<cv::Rect> ROIs;
	vector<vector<cv::Rect> > additionalSynchRect[2];
	vector<short> shortMsg;
	vector<SymbolData> msg;
	VideoCapture videoReader;
	void writeFrame(Mat &frame)
	{
		Utilities::writeFrame(vidWriter,frame);
	}
public:
	virtual void setCommonParameters(vector<short> &msg, string outputVideoFile)
	{
		this->shortMsg = msg;
		this->msg = Parameters::symbolsData.getMsgSymbols(msg);
		for (int i = 0; i < (Parameters::sideA * Parameters::sideB); i++)
		{
			ostringstream ostr;
			ostr << "freq" << setw(5) << setfill('0') << i << ".txt";
			ofstream frequencies(ostr.str());
			for (int j = i; j < this->msg.size(); j += (Parameters::sideA * Parameters::sideB))
			{
				frequencies << this->msg[j].frequency << ",";
			}
			frequencies.close();
		}
		vidWriter = Utilities::getVideoWriter(getVideoName(outputVideoFile), Utilities::getFrameSize());
		
		Parameters::globalROI = Utilities::createChessBoardDataRect();
		// create the additional data rectangles
		additionalSynchRect[0] = Utilities::createTopBottomLayers(4);
		additionalSynchRect[1] = Utilities::createTopBottomLayers(8);
	}
	bool initImage(string inputImage, vector<short> &msg, string outputVideoFile)
	{
		
		img = imread(inputImage);
		cv::resize(img, img, Utilities::getFrameSize());
		//imshow("img", img);
		//cv::waitKey(0);
		//frame_width = img.cols;
		//frame_height = img.rows;
		
		setCommonParameters(msg,outputVideoFile);
		return true;
	}
	bool initVideo(string inputVideoFile, vector<short> msg, string outputVideoFile)
	{
		videoReader.open(inputVideoFile);
		if (videoReader.isOpened())
		{
			videoReader.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
			double framerate;
			if (Parameters::ifps > 1)
			{
				framerate = Parameters::ifps;
			}
			else
			{
				framerate = videoReader.get(CV_CAP_PROP_FPS); //get the frame rate
			}

			//frame_width = videoReader.get(CV_CAP_PROP_FRAME_WIDTH);
			//frame_height = videoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
			Parameters::fps = Utilities::getOuputVideoFrameRate((int)framerate);

			inputFrameUsageFrames = Parameters::fps / framerate;

			setCommonParameters(msg, outputVideoFile);
			return true;
		}
		return false;
	}
	void addChessSynchFrame(bool end)
	{
		if (end)
		{
			//Utilities::addDummyFramesToVideo(vidWriter, Parameters::fps);
			cv::Size endPatternSize = Parameters::patternsize;
			endPatternSize.width--; endPatternSize.height--;

			Utilities::addDummyFramesToVideo(vidWriter, Parameters::fps, Utilities::createChessBoard(endPatternSize));
			//vidWriter.release();
		}
		else
		{
			Utilities::addDummyFramesToVideo(vidWriter, Parameters::fps, Utilities::createChessBoard(Parameters::patternsize));
			//Utilities::addDummyFramesToVideo(vidWriter, Parameters::fps);
			if (Parameters::synchMethod == SYNCH_CHESS)
			{
				addNonModulatedFrames(Parameters::fps);
			}
		}
	}
	void addGreenChannelSynchFrames(bool end)
	{
		double frameIndex = 0;

		vector<float> wave = Utilities::createPreambleWave();
		cv::Rect rect(0, 0, Parameters::DefaultFrameSize.width, Parameters::DefaultFrameSize.height);
		int i = 0;
		for (int j = 0; j < wave.size(); j++, i++)
		{
			if (Parameters::realVideo)
			{
				if (i >= frameIndex)
				{
					frameIndex += inputFrameUsageFrames;
					if (remainingFrames)
					{
						Mat tmp;
						remainingFrames = videoReader.read(tmp);
						if (remainingFrames)
						{
							cv::resize(tmp, img, Utilities::getFrameSize());
						}
					}
				}
			}
			vector<Mat> BGR;
			cv::split(img, BGR);

			
			Utilities::updateFrameWithVchannel(BGR[0], rect , wave[j]);
			Utilities::updateFrameWithVchannel(BGR[0], Parameters::globalROI, -2 * wave[j]);

			Utilities::updateFrameWithVchannel(BGR[1], rect, -wave[j]);
			Utilities::updateFrameWithVchannel(BGR[1], Parameters::globalROI, 2 * wave[j]);
			
			Utilities::updateFrameWithVchannel(BGR[2], rect, wave[j]);
			Utilities::updateFrameWithVchannel(BGR[2], Parameters::globalROI, -2 * wave[j]);

			Mat frame;
			cv::merge(BGR, frame);
			writeFrame(frame);
		}
	}
	void addSynchFrames(bool end)
	{
		switch (Parameters::synchMethod)
		{
		case SYNCH_GREEN_CHANNEL:
		
			addGreenChannelSynchFrames(end);
		
		break;
		case SYNCH_CHESS:
			addChessSynchFrame(end);
		break;
		case SYNCH_COMBINED:
			addChessSynchFrame(end);
			if (!end)
			{
				addGreenChannelSynchFrames(end);
			}
			break;
		}
		if (end)
		{
			Parameters::done = true;
			vidWriter.release();
		}
	}

	void addNonModulatedFrames(int len)
	{
		for (int i = 0; i < len; i++, frameIndexComparison++)
		{
			if (Parameters::realVideo && frameIndexComparison >= frameIndex)
			{
				frameIndex += inputFrameUsageFrames;
				videoReader >> img;
			}
			Mat frame;
			resize(img, frame, Parameters::DefaultFrameSize);
			writeFrame(frame);
		}
	}

	virtual string getVideoName(string outputVideoFile)
	{
		return outputVideoFile;
	}
	virtual void initCommunication()
	{
		//amplitudes.push_back(WaveGenerator::createWaveGivenFPS(msg, Parameters::fps, Parameters::symbolTime));
	}
	virtual void sendImageMainLoop()
	{
		for (int i = 0; i < amplitudes[0].size(); i++)
		{
			//cout << i << endl;
			Mat frame = img.clone();
			//cv::resize(img, frame, Utilities::getFrameSize());
			Utilities::updateFrameWithVchannel(frame, Parameters::globalROI, amplitudes[0][i]);
			//frame.convertTo(frame, CV_32F);
			writeFrame(frame);
		}
	}
	virtual vector<Mat> getSplittedImages(Mat &frame)
	{
		return vector<Mat>();
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
				//cout << (amplitudes[k]) << endl;
			}
			Mat tmp;
			cv::resize(frame, tmp, Utilities::getFrameSize());
			Utilities::updateFrameWithVchannel(tmp, Parameters::globalROI, amplitudes[0][k]);
			writeFrame(tmp);
		}
	}
	// symbol_time: how many milliseconds will the symbol last
	void sendImage()
	{
		initCommunication();
		addSynchFrames(false);
		sendImageMainLoop();
		addSynchFrames(true);
	}

	// symbol_time: how many milliseconds will the symbol last
	void sendVideo()
	{
		if (videoReader.isOpened())
		{
			initCommunication();
			addSynchFrames(false);
			sendVideoMainLoop();
			addSynchFrames(true);
		}
	}

	
	vector<short> receive2(vector<float> frames, int fps,int frames_per_symbol,int start = 0, int end = 0)
	{
		/*for (int i = 0; i < frames.size(); i++)
		{
			printf("%f\n", frames[i]);
		}
		*/
		if (Parameters::DecodingMethod == CROSS_CORRELATION)
		{
			vector<double> symbolDataVec;
			return receiveCrossCorrelation(frames, fps, frames_per_symbol, symbolDataVec,start,end);
		}
		return receiveFFT(frames, fps, frames_per_symbol);
	}
	
	// receive using cross-correlation as classifier
	vector<short> receiveCrossCorrelation(vector<float> frames, int fps, int frames_per_symbol, vector<double> & retSymbols,int start, int end)
	{
		// return array
		vector<short> result;
		// create the signals to use in correlation
		vector<vector<vector<float> > > signals;
		//signals.push_back(WaveGenerator::createSampledSineWave(fps, frames_per_symbol, Parameters::FREQ[0]));
		//signals.push_back(WaveGenerator::createSampledSineWave(fps, frames_per_symbol, Parameters::FREQ[1]));
		for (int i = 0; i < Parameters::symbolsData.allData.size(); i++)
		{
			vector<vector<float> > signal;
			double phase = MM_PI;
			for (int j = 0; j < 7; j++, phase += MM_PI / 4)
				//double phase = MM_PI;
			{
				//vector<float> tmp = WaveGenerator::createSampledSquareWave(fps, frames_per_symbol, Parameters::symbolsData.allData[i].frequency,1,-1, phase);
				vector<float> tmp1 = WaveGenerator::createSampledSineWave(fps, frames_per_symbol, Parameters::symbolsData.allData[i].frequency, phase);
				//tmp1[0] = tmp1[tmp1.size() - 1] = 0;
				//signal.push_back(tmp);
				signal.push_back(tmp1);
			}
			signals.push_back(signal);
		}
		int window_size = frames_per_symbol;
		if (!end)
		{
			end = frames.size();// -((Parameters::synchMethod == SYNCH_CHESS) ? fps : 0);
		}
		//int start = 0;// (Parameters::synchMethod == SYNCH_CHESS) ? fps : 0;
		vector<int> best_start(signals.size(), 0);
		vector<int> best_end(signals.size(), 0);
		vector<int> test_start(signals.size(), 0);
		static int cellNumber = 0;
		ostringstream outputframes;
		outputframes << "outputframes" << setw(5) << setfill('0') << cellNumber++ << ".txt";
		ofstream outputFrames(outputframes.str());
		for (int i = start; i < end; i += window_size)
		{
			for (int j = 0; j < window_size; j++)
			{
				outputFrames << frames[i + j] << ",";
			}
			outputFrames << endl;
			vector<double> Detected;
			for (int j = 0; j < signals.size(); j++)
			{
				vector<double> bestOfSignal = Utilities::calcCrossCorrelate(signals[j], frames, i, i + window_size, best_start, best_end, test_start);
				Detected.push_back(*max_element(bestOfSignal.begin(), bestOfSignal.end()));
			}
			// get maximum response
			int maxIdx = 0;
			for (int j = 1; j < signals.size(); j++)
			{
				if (Detected[j] > Detected[maxIdx])
				{
					maxIdx = j;
				}
			}
			retSymbols.push_back(Parameters::symbolsData.allData[maxIdx].frequency);
			vector<short> maxSymbol = Parameters::symbolsData.allData[maxIdx].getSymbol();
			result.insert(result.end(), maxSymbol.begin(), maxSymbol.end());
		}
		outputFrames.close();
		return result;
	}
	vector<short> receiveFFT(vector<float> frames, int fps, int frames_per_symbol)
	{
		Parameters::amplitudes = frames;
		vector<short> result;
		vector<vector<int> > detection(Parameters::symbolsData.allData.size(), vector<int>(frames.size(), 0));
		vector<int> other_detected(frames.size(), 0);
		int window_size = frames_per_symbol;
		for (int i = 0; i < frames.size() - fps; i++)
		{
			//cout << frames[i] << endl;
			vector<Frequency> temp = Utilities::myft(frames, fps, i, window_size);
			// get the maximum frequency for this set of frames
			int maxi = 0;
			for (int j = 1; j < temp.size(); j++)
			{
				if (temp[j].percent > temp[maxi].percent)
				{
					maxi = j;
				}
			}
			bool foundFreq = false;
			for (int j = 0; j < Parameters::symbolsData.allData.size(); j++)
			{
				if (abs(temp[maxi].freq - Parameters::symbolsData.allData[j].frequency) < EPSILON)
				{
					for (int k = 0; k < window_size; k++)
					{
						detection[j][i + k]++;
					}
					foundFreq = true;
					break;
				}
			}
			if (!foundFreq)
			{
				for (int j = 0; j < window_size; j++) other_detected[i + j]++;
			}
			
		}
		// then check for the first frame that has 60% or more with one of the two symbols (0,1), 
		// and the symbol should have enough time (at least after the first FRAMES_PER_SYMBOL have been passed)
		int starting_index = fps + frames_per_symbol / 2; // to be in the middle of the first symbol as we have the first symbol time empty
		//for (int i = frames_per_symbol; i < frames.size() - frames_per_symbol; i++)
		//{
		//	if ((zero_detected[i] + one_detected[i]) * 10 >= (zero_detected[i] + one_detected[i] + other_detected[i]) * 6)
		//	{
		//		// this first frame
		//		starting_index = i;
		//		break;
		//	}
		//}
		cout << "Starting Index Inside loaded frames = " << starting_index << endl;
		// for the rest of the symbols
		// just follow the same rule
		for (int i = starting_index; i < frames.size() - fps; i += frames_per_symbol)
		{
			//if (zero_detected[i] > one_detected[i])
			//{
			//	// this first frame and zero
			//	result.push_back(0);
			//}
			//else if (one_detected[i] > zero_detected[i])
			//{
			//	// this first frame and one
			//	result.push_back(1);
			//}
			int maxIdx = 0;
			for (int j = 1; j < Parameters::symbolsData.allData.size(); j++)
			{
				if (detection[j][i] > detection[maxIdx][i])
				{
					maxIdx = j;
				}
			}
			bool foundSymbol = true;
			for (int j = 1; j < Parameters::symbolsData.allData.size(); j++)
			{
				if (j != maxIdx && detection[maxIdx][i] > detection[j][i])
				{
					foundSymbol = false;
					break;
				}
			}
			if (foundSymbol)
			{
				vector<short> maxSymbol = Parameters::symbolsData.allData[maxIdx].getSymbol();
				result.insert(result.end(), maxSymbol.begin(), maxSymbol.end());
			}
			else
			{
				if (Parameters::DecodingMethod == FFT_NO_RANDOM_GUESS)
				{
					for (int j = 0; j < Parameters::symbolsData.allData[0].symbol.size(); j++)
					{
						result.push_back(2);
					}
				}
				else if (Parameters::DecodingMethod == FFT_RANDOM_GUESS)
				{
					for (int j = 0; j < Parameters::symbolsData.allData[0].symbol.size(); j++)
					{
						if (result.size() == 0)
						{
							result.push_back(1);
						}
						else
						{
							result.push_back((~result[result.size() - 1]) & 1);
						}
					}
				}
				
			}
		}
		return result;
	}

	// receive with a certain ROI ratio
	virtual vector<short> receive(string fileName, double ROI_Ratio)
	{
		int fps = 0;
		int frames_per_symbol = fps * Parameters::symbolTime / 1000;
		vector<float> frames = Utilities::getVideoFrameLuminancesSplitted(fileName, ROI_Ratio, fps,1,1,true,1,1)[0];
		return receive2(frames, fps, frames_per_symbol);
	}



	// receive with a certain ROI ratio
	void receiveWithSelectionByHand(string fileName)
	{
		int fps = 0;
		int frames_per_symbol = fps * Parameters::symbolTime / 1000;

		vector<float> frames = Utilities::getVideoFrameLuminances(fileName, fps);
		receive2(frames, fps, frames_per_symbol);
	}

};