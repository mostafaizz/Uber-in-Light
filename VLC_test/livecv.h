#include "Header.h"
#include "Parameters.h"

class LiveCV
{
public:
	static void displayFunc() {
		if (Parameters::done && !Parameters::transmitterQueue.size())
		{
			long long milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - Parameters::transmissionStartTime).count();
			cout << "Time = " << milli << " ms" << endl;
			exit(0);
		}
		
		// start processing whin the queue has 10 frames
		while (Parameters::outputFrameIndex == 0 && Parameters::transmitterQueue.size() < 10)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(50));
		}
		while (!Parameters::transmitterQueue.size())
		{
			std::this_thread::sleep_for(std::chrono::microseconds(50));
		}
		Mat newImage;
		Parameters::transmitterQueue.pop(newImage);

		/*ostringstream ostr;
		ostr << "Frame No. = " << Parameters::outputFrameIndex;
		string txt = ostr.str();
		cv::Scalar white(255, 255, 255);
		rectangle(newImage, Rect(20, 10, 200, 30), white, -1);
		putText(newImage, txt, cvPoint(30, 30),
			FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0, 0, 0), 1, CV_AA);*/
		//countFrames();
		long long expectedMicroseconds = Parameters::outputFrameIndex;
		expectedMicroseconds *= 1000000 / Parameters::fps;
		long long currentMicroseonds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - Parameters::transmissionStartTime).count();
		//printf("exp = %lld\tcurr = %lld\tind = %d\tQueue = %d\n",
		//expectedMicroseconds, currentMicroseonds, Parameters::outputFrameIndex, Parameters::transmitterQueue.size());
		if (Parameters::outputFrameIndex && currentMicroseonds < expectedMicroseconds)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(expectedMicroseconds - currentMicroseonds));
		}
		
		imshow(Parameters::displayName, newImage);
		cv::waitKey(1);
		//currentMicroseonds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - Parameters::transmissionStartTime).count();
		//printf("%5d\tfps: %4.1f\n", Parameters::outputFrameIndex, Parameters::outputFrameIndex*1000000.0 / currentMicroseonds);
		if (!Parameters::outputFrameIndex)
		{
			/*while (pause)
			{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			}*/
			Parameters::transmissionStartTime = std::chrono::system_clock::now();
		}
		Parameters::outputFrameIndex++;
	}
	static void displayMainLoop() {
		if (Parameters::liveTranmitterCV)
		{
			namedWindow(Parameters::displayName, WINDOW_NORMAL);
			setWindowProperty(Parameters::displayName, CV_WND_PROP_FULLSCREEN, 1); //( on or off)

			while (!Parameters::done)
			{
				displayFunc();
			}
		}
	}
};