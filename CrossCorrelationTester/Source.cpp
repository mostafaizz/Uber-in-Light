#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <algorithm>

using namespace std;

const double MM_PI = 3.14159265359;

// try to read this from file
struct SymbolData
{
	string symbol;
	double amplitude;
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
	// assume the fiel is sorted
	void readData(string fileName)
	{
		vector<SymbolData> tmp;
		ifstream ifstr(fileName);
		if (ifstr.is_open())
		{
			SymbolData obj;
			while (ifstr >> obj.symbol >> obj.frequency >> obj.amplitude >> obj.phase)
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
	void addSymbol(string symbol, double frequency, double amplitude = 0.004, double phase = 0)
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
	}
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
		for (int i = 0; i < msg.size(); i++)
		{
			string ostr;
			for (int j = 0; i < msg.size(); j++, i++)
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
	static AllSymbolsData symbolsData;
	static int getSymbolLength()
	{
		return symbolsData.allData[0].symbol.size();
	}
};
AllSymbolsData Parameters::symbolsData;

// this class will be generating waves
class WaveGenerator
{
public:
	
	// create sampled sine wave
	static vector<float> createSampledSineWave(int fps, int frames_per_symbol, float freq, double phase)
	{
		vector<float> result;
		for (int i = 0; i < frames_per_symbol; i++)
		{
			double x = 2 * MM_PI * freq * i / fps + phase;
			result.push_back(sin(x));
		}

		return result;
	}

	// create sampled square wave
	static vector<float> createSampledSquareWave(int fps, int frames_per_symbol, float freq, float pos, float neg, double phase = MM_PI)
	{
		vector<float> result;
		for (int i = 0; i < frames_per_symbol; i++)
		{
			float val = sin(2 * MM_PI * freq * i / fps + phase);
			if (val > 0)
			{
				result.push_back(pos);
			}
			else if (val < 0)
			{
				result.push_back(neg);
			}
			else
			{
				result.push_back(0);
			}
		}

		return result;
	}
	// create sampled square wave
	static vector<float> createSampledTriangleWave(int fps, int frames_per_symbol, float freq, float amp)
	{
		vector<float> result;
		for (int i = 0; i < frames_per_symbol; i++)
		{
			float pos = fmod((freq * i) / fps, 1.0);
			result.push_back((1 - fabs(pos - 0.5) * 4)*amp);
		}

		return result;
	}
};

class Communicator
{

public:
	// calculate the best fit between two signals based on cross-correlation and return the peek value
	static vector<double> calcCrossCorrelate(vector< vector<float> > &signals, vector<float> &test, int start, int end,
		vector<int> &best_start, vector<int> &best_end, vector<int> &test_start, int absolute = 0, int display = 0)
	{
		vector<double> bestVal(signals.size(), 0);
		int tsz = end - start + 1;
		int ssz = signals[0].size();
		for (int shift = -ssz + 1; shift < tsz - 1; shift++)
		{
			vector<double> sum(signals.size(), 0);
			vector<int> cnt(signals.size(), 0);
			int start_test = std::max(0, shift);
			int end_test = std::min(shift + ssz, tsz); // exclusive
			for (int j = start_test; j < end_test; j++)
			{
				for (int k = 0; k < signals.size(); k++)
				{
					sum[k] += signals[k][j - shift] * test[j + start];
					cnt[k]++;
				}
			}

			for (int k = 0; k < sum.size(); k++)
			{
				if (sum[k] > bestVal[k])
				{
					bestVal[k] = sum[k];
					best_start[k] = start_test;
					best_end[k] = end_test;
					test_start[k] = start_test - shift;
				}
			}
		}
		return bestVal;
	}
	// receive using cross-correlation as classifier
	vector<short> receiveCrossCorrelation(vector<float> frames, int fps, int frames_per_symbol, vector<double> & retSymbols)
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
		int end = frames.size();// -((Parameters::synchMethod == SYNCH_CHESS) ? fps : 0);
		int start = 0;// (Parameters::synchMethod == SYNCH_CHESS) ? fps : 0;
		vector<int> best_start(signals.size(), 0);
		vector<int> best_end(signals.size(), 0);
		vector<int> test_start(signals.size(), 0);
		static int cellNumber = 0;
		
		for (int i = start; i < end; i += window_size)
		{
			vector<double> Detected;
			for (int j = 0; j < signals.size(); j++)
			{
				vector<double> bestOfSignal = calcCrossCorrelate(signals[j], frames, i, i + window_size, best_start, best_end, test_start);
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
		return result;
	}
};

int main(int argc, char** argv)
{
	int fps = 30;
	if (argc != 3)
	{
		puts("Usage: TestCrossCorrelation <file_name> <fps>");
		puts("Exiting");
		return 0;
	}
	fps = stoi(argv[2]);
	Parameters::symbolsData.readData("8Freq8org.symbol");
	ifstream inpf(argv[1]);
	vector<vector<float> > data;
	while (!inpf.eof())
	{
		string line;
		getline(inpf, line);
		istringstream istr(line);
		string val;
		vector<float> tmp;
		while (getline(istr, val, ','))
		{
			tmp.push_back(stof(val));
		}
		data.push_back(tmp);
	}
	// then reshape
	int frame_per_symbol = data[0].size();
	
	Communicator comm;
	vector<float> frames;
	for (int i = 0; i < data.size(); i++)
	{
		frames.insert(frames.end(), data[i].begin(), data[i].end());
	}
	frames.push_back(0);
	vector<double> retSymbols;
	comm.receiveCrossCorrelation(frames, fps, frame_per_symbol, retSymbols);
	for (int i = 0; i < retSymbols.size(); i++)
	{
		cout << retSymbols[i] << " ";
	}
	return 0;
}