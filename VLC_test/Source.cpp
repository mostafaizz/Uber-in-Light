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

#include "Properties.h"
#include "SplitAmplitudeCommunicator.h"
#include "SplitScreenCommunicator.h"
#include "SplitScreenAmpDifferenceCommunicator.h"
#include "OldCommunicator.h"
#include "BGR0Communicator.h"
#include "RGBSpatialRedundancy.h"
#include "HUECommnunicator.h"
#include "BGRCommunicator2.h"
#include "BG0RCommunicator.h"
#include "BGRCommunicator4.h"
#include "B0GRCommunicator.h"
#include "BGR0Communicator.h"
#include "BGRSpatialCommunicator.h"
#include "BGRCommunicator2Spatial.h"
#include "BGR2CommunicatorPatterns.h"
#include "Hamming.h"
#include "ReedSolomon.h"
#include "mygl.h"
#include "livecv.h"

//Properties* Properties::inst = Properties::getInst();
int Properties::returnError()
{
	cout << "Usage: run.exe (-s <message>)|(-r <filename>)\n";
	cout << "One option must be selected\n";
	cout << "-s : create video file with the specified message where the message will be converted to binary\n";
	cout << "-r : receive message from video file and write the output to the screen\n";
	return -1;
}
int Properties::testSendReceive(int argc, char** argv)
{

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-s"))
		{
			mode = SEND;
		}
		else if (!strcmp(argv[i], "-r"))
		{
			mode = RECV;
		}
		else if (!strcmp(argv[i], "-c"))
		{
			mode = CNVRT;
		}
		else if (!strcmp(argv[i], "-e"))
		{
			mode = EXTEND;
			// get number of repitionis
			if (i < argc - 1)
			{
				extendN = stod(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-exp"))
		{
			mode = EXPLORE;
		}
		else if (!strcmp(argv[i], "-noise"))
		{
			mode = WHITE_NOISE;
		}
		else if (!strcmp(argv[i], "-if"))
		{
			// get the file name
			if (i < argc - 1)
			{
				inputFileName = argv[++i];
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-of"))
		{
			// get the file name
			if (i < argc - 1)
			{
				outputFileName = argv[++i];
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-m"))
		{
			// get the file name
			if (i < argc - 1)
			{
				type = stoi(argv[++i]);
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-t"))
		{
			// get the file name
			if (i < argc - 1)
			{
				msgFileName = argv[++i];
				ifstream ifs(msgFileName);
				if (ifs.is_open())
				{
					// assume the text inside
					ifs >> text;
					ifs.close();
				}
				else
				{
					// assume the file name is the text
					text = msgFileName;
				}
				// convert the message to vector of short
				msg = Utilities::getBinaryMessage(text);
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-roi"))
		{
			// get the file name
			if (i < argc - 1)
			{
				ROI = stod(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-fps"))
		{
			// get the file name
			if (i < argc - 1)
			{
				Parameters::fps = stod(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-ifps"))
		{
			// get the file name
			if (i < argc - 1)
			{
				Parameters::ifps = stod(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-v"))
		{
			Parameters::realVideo = true;
		}
		//else if (!strcmp(argv[i], "-zero"))
		//{
		//	// get the file name
		//	if (i < argc - 1)
		//	{
		//		Parameters::symbolsData.addSymbol("0", stod(string(argv[++i])));
		//	}
		//	else
		//	{
		//		return returnError();
		//	}
		//}
		//else if (!strcmp(argv[i], "-one"))
		//{
		//	// get the file name
		//	if (i < argc - 1)
		//	{
		//		Parameters::symbolsData.addSymbol("1", stod(string(argv[++i])));
		//	}
		//	else
		//	{
		//		return returnError();
		//	}
		//}
		else if (!strcmp(argv[i], "-ec"))
		{
			// error correction enabled
			// the starting second of the video
			// currntly in the conversion only
			if (i < argc - 1)
			{
				errorCorrection = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-interleave"))
		{
			// error correction enabled and interleave enabled
			// doesn't mean anything if error correction is not enabled
			interleave = true;
		}
		else if (!strcmp(argv[i], "-start"))
		{
			// the starting second of the video
			// currntly in the conversion only
			if (i < argc - 1)
			{
				string start = string(argv[++i]);
				ifstream startStream(start);
				if (startStream.is_open())
				{
					startStream >> Parameters::start_second;
					Parameters::endSecondFile = start;
				}
				else
				{
					Parameters::start_second = stod(start);
				}
				startStream.close();
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-end"))
		{
			// the starting second of the video
			// currntly in the conversion only
			if (i < argc - 1)
			{
				Parameters::end_second = stod(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-side"))
		{
			// the starting second of the video
			// currntly in the conversion only
			if (i < argc - 1)
			{
				Parameters::sideA = Parameters::sideB = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-sideA"))
		{
			// the starting second of the video
			// currntly in the conversion only
			if (i < argc - 1)
			{
				Parameters::sideA = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-sideB"))
		{
			// the starting second of the video
			// currntly in the conversion only
			if (i < argc - 1)
			{
				Parameters::sideB = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-color"))
		{
			// the starting second of the video
			// currntly in the conversion only
			color = true;
		}
		else if (!strcmp(argv[i], "-corr"))
		{
			// the starting second of the video
			// currntly in the conversion only
			if (i < argc - 1)
			{
				mode = CORRELEATION;
				correlation = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-decode"))
		{
			// the decoding method
			if (i < argc - 1)
			{
				Parameters::DecodingMethod = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-amp"))
		{
			// the amplitude
			if (i < argc - 1)
			{
				Parameters::LUMINANCE = stod(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-mfsk"))
		{
			// the amplitude
			if (i < argc - 1)
			{
				Parameters::mfsk = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-time"))
		{
			// the amplitude
			if (i < argc - 1)
			{
				Parameters::symbolTime = stod(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-symbols"))
		{
			if (i < argc - 1)
			{
				Parameters::symbolsData.readData(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-res"))
		{
			if (i < argc - 2)
			{
				Parameters::DefaultFrameSize.width = stoi(argv[++i]);
				Parameters::DefaultFrameSize.height = stoi(argv[++i]);
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-full"))
		{
			if (i < argc - 1)
			{
				Parameters::fullScreen = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-codec"))
		{
			if (i < argc - 1)
			{
				Parameters::setCodec(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-live"))
		{
			Parameters::liveTranmitter = 1;
		}
		else if (!strcmp(argv[i], "-livecv"))
		{
			Parameters::liveTranmitterCV = 1;
		}
		else if (!strcmp(argv[i], "-total"))
		{
			// the amplitude
			if (i < argc - 1)
			{
				Parameters::totalTime = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-seed"))
		{
			// the amplitude
			if (i < argc - 1)
			{
				Parameters::seed = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-synch"))
		{
			// the amplitude
			if (i < argc - 1)
			{
				Parameters::synchMethod = stoi(string(argv[++i]));
			}
			else
			{
				return returnError();
			}
		}
		else if (!strcmp(argv[i], "-chess"))
		{
			Utilities::displayChessBoardFullScreen();
			return 0;
		}
	}
	if (mode == SEND || mode == RECV)
	{
		if (Parameters::totalTime)
		{
			int totalLength = Parameters::getSymbolLength() *  Parameters::sideA * Parameters::sideB * 1000 * Parameters::totalTime / Parameters::symbolTime;
			//printf("length=%d\n", totalLength);
			msg.clear();
			std::mt19937 mt(Parameters::seed);
			//std::uniform_int_distribution<int> dist(0, 1);
			for (int i = 0; i < totalLength; i++)
			{
				msg.push_back(mt() & 1);
				//printf("%d", msg[i]);
			}
			ostringstream ostr;
			ostr << "totaltime" << Parameters::totalTime << "_seed" << Parameters::seed;
			msgFileName = ostr.str();
			//puts("");
		}
		else if (msgFileName.size())
		{
			ifstream ifs(msgFileName);
			if (ifs.is_open())
			{
				// assume the text inside
				ifs >> text;
				ifs.close();
			}
			else
			{
				// assume the file name is the text
				text = msgFileName;
			}
			// convert the message to vector of short
			msg = Utilities::getBinaryMessage(text);
		}
		else
		{
			return returnError();
		}
	}

	switch (type)
	{
	case SPATIAL_REDUNDANCY:
		communicator = new SplitAmplitudeCommunicator;
		break;
	case FREQ_DIFF:
		//communicator = new SplitFrequencyCommunicator;
		break;
	case FREQ_AMP_DIFF:
		//communicator = new SplitFrequencyAmplitudeCommunicator;
		break;
	case SPLIT_SCREEN_TEMPORAL_CORRELATION:
		communicator = new SplitScreenCommunicator;
		break;
	case SPLIT_SCREEN_SPATIAL_REDUNDANCY:
		communicator = new SplitScreenAmpDifferenceCommunicator;
		break;
	case 6:
		communicator = new BGRCommunicator;
		break;
	case 7:
		communicator = new RGBSpatialRedundancyCommunicator;
		break;
	case 8:
		communicator = new HUECommunicator;
		break;
	case 9:
		communicator = new BGRCommunicator2;
		break;
	case 10:
		communicator = new BG0RCommunicator;
		break;
	case 11:
		communicator = new BGRCommunicator4;
		break; 
	case 12:
		communicator = new B0GRCommunicator;
		break;
	case 13:
		communicator = new BGR0Communicator;
		break;
	case 14:
		communicator = new BGRSpatialCommunicator;
		break;
	case 15:
		communicator = new BGRCommunicator2Spatial;
		break;
	case 16:
		communicator = new BGRCommunicator2Patterns;
		break;
	case -1:
		communicator = new OldCommunicator;
		break;
	default:
		communicator = new Communicator;
	}

	switch (mode)
	{
	case SEND:
	{
		std::thread startTrans(displayGlut, argc, argv);
		std::thread startTransLiveCV(LiveCV::displayMainLoop);
		if (errorCorrection == HAMMING){
			MyHamming hamming;
			msg = hamming.EncodeMessage(msg, false);
			msgFileName = "Hamming_" + msgFileName;
			if (interleave)
			{
				msgFileName = "Interleave_" + msgFileName;
			}
		}
		else if (errorCorrection == REED_SOLOMON)
		{
			ReedSolomon reedSolomon;
			msg = reedSolomon.encode_bit_stream(msg);
			msgFileName = "ReedSolomon_" + msgFileName;
		}
		if (Parameters::realVideo)
		{
			if (communicator->initVideo(inputFileName, msg,
				Utilities::createOuputVideoName(msgFileName, inputFileName, outputFileName)))
			{
				communicator->sendVideo();
			}
		}
		else
		{
			if (communicator->initImage(inputFileName, msg,
				Utilities::createOuputVideoName(msgFileName, inputFileName, outputFileName)))
			{
				communicator->sendImage();
			}
		}
		startTransLiveCV.join();
		startTrans.join();
		break;
	}
	case RECV:

		if (ROI > 0 && ROI <= 1)
		{

			// then we have ROI
			vector<short> received = communicator->receive(inputFileName, ROI);
			
			for (int i = 0; i < msg.size(); i++)
			{
				cout << msg[i];
			}
			if (errorCorrection == HAMMING){
				MyHamming hamming;
				received = hamming.DecodeMessage(received, false);
			}
			else if (errorCorrection == REED_SOLOMON)
			{
				ReedSolomon reedSolomon;
				received = reedSolomon.decode_bit_stream(received);
			}

			cout << endl;
			for (int i = 0; i < received.size(); i++)
			{
				cout << received[i];
			}
			cout << endl;
			Utilities::LCS_greedy(msg, received);// , inputFileName);
		}
		else
		{
			communicator->receiveWithSelectionByHand(inputFileName);
		}
		break;
	case CNVRT:
		// convert argv2 video to argv3 as a video with the framerate in argv4
		// argv3 must end with .avi
		Utilities::convertVideo(inputFileName, outputFileName, Parameters::fps, Parameters::start_second, Parameters::end_second);
		break;
	case EXTEND:
		// extend the video by repeating
		Utilities::repeatVideo(inputFileName, outputFileName, Parameters::fps, extendN, Parameters::start_second, Parameters::end_second);
		break;
	case CORRELEATION:
		Utilities::correlateVideoDifference(inputFileName, Parameters::start_second, Parameters::end_second, correlation);
		break;
	case WHITE_NOISE:
		Utilities::createWhiteNoiseVideo(outputFileName);
		break;
	case EXPLORE:
		Utilities::exploreVideo(inputFileName);
		break;

	}

	return 0;
}
vector<Mat> Properties::getSplittedImages(Mat &frame)
{
	return communicator->getSplittedImages(frame);
}

#include <omp.h>

int main(int argc, char** argv)
{
	/*AllSymbolsData as;
	as.createGrayCodeSymbols(14, 1, 8, 0.008);
	for (int i = 0; i < as.allData.size(); i++)
	{
		cout << as.allData[i].symbol << " " << as.allData[i].frequency << endl;
	}
	return 0;*/
	/*Mat ones = Mat::ones(cv::Size(2, 2), CV_8UC1);
	Mat tmp;
	Mat arr[] = { ones, ones, ones };
	cv::merge(arr, 3, tmp);
	cv::Scalar sc = mean(tmp);
	cout << tmp.channels() << endl;
	cout << sc.val[0] << endl;
	cout << sc.val[1] << endl;
	cout << sc.val[2] << endl;

	return 0;*/
	/*string release = "C:\\VLC\\Release\\final_rotation_test\\all.txt";
	ifstream ifs(release);
	VideoWriter vidWriter("C:\\VLC\\Release\\final_rotation_test\\all.avi", CV_FOURCC('X', 'V', 'I', 'D'),60,cv::Size(1280,720));
	string vid;
	while (getline(ifs, vid))
	{
		VideoCapture cap("C:\\VLC\\Release\\final_rotation_test\\" + vid);
		Mat tmp;
		while (cap.read(tmp))
		{
			vidWriter << tmp;
		}
	}
	vidWriter.release();*/
	//Mat img = imread(release + "hilightnew.jpg");

	//cv::Scalar scalar = mean(img);
	//double meanBrightness = 0;
	//for (int i = 0; i < 4; i++)
	//{
	//	meanBrightness += scalar[i];
	//}
	//cout << meanBrightness << endl;

	int num = omp_get_num_procs();
	omp_set_num_threads(num / 2); // half of the processors
	std::chrono::system_clock::time_point transmissionStartTime = std::chrono::system_clock::now();
	Properties::getInst()->testSendReceive(argc, argv);
	long long milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - transmissionStartTime).count();
	cout << "Time = " << milli << "ms" << endl;
	
	return 0;
}