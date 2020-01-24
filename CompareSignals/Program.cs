using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CompareSignals
{
    class Program
    {
        static void writeToFileHighCrossCorrelation(double[] GT, double[] captured, string fileName)
        {
            double bestVal = 0;
            int best_i = 0;
            for (int i = 1 - captured.Length; i < GT.Length - 1; i++)
		    {
                double sum = 0;
			    for (int j = Math.Max(0, i); j < Math.Min(GT.Length, captured.Length + i); j++)
			    {
                    sum += GT[j] * captured[j - i];
			    }
			    //cout << i << "\t" << sum << endl;
                if (sum > bestVal)
                {
                    bestVal = sum;
                    best_i = i;
                }
		    }
            // write to file
            StreamWriter sw = new StreamWriter(fileName + ".csv");
            sw.WriteLine("GT,test");
            for (int j = Math.Max(0, best_i); j < Math.Min(GT.Length, captured.Length + best_i); j++)
            {
                sw.WriteLine("{0},{1}", GT[j], captured[j - best_i]);
            }
            sw.Close();
        }
        static void Main(string[] args)
        {
            string[] files = Directory.GetFiles
                (@"C:\Users\mostafaizz\Documents\Visual Studio 2013\Projects\ConsoleApplication1\Release\errors_helicopter_recorded_spatial_redundancy",
                "*.frames", SearchOption.AllDirectories);
            Dictionary<string, List<string>> dic = new Dictionary<string, List<string>>();
            // get the same files together
            for(int i = 0;i < files.Length;i++)
            {
                FileInfo finf = new FileInfo(files[i]);
                string key = finf.Name.Substring(0, 10);
                if (!dic.ContainsKey(key))
                {
                    dic.Add(key, new List<string>());
                }
                dic[key].Add(files[i]);
            }
            // 
            foreach(KeyValuePair<string,List<string>> entry in dic)
            {
                StreamReader GT, test;

                if(entry.Value[0].Length > entry.Value[1].Length)
                {
                    GT = new StreamReader(entry.Value[0]);
                    test = new StreamReader(entry.Value[1]);
                }
                else 
                {
                    GT = new StreamReader(entry.Value[1]);
                    test = new StreamReader(entry.Value[0]);
                }
                // then read the values and write the correlated signals
                List<double> GTList = new List<double>(), testList = new List<double>();
                while(!GT.EndOfStream)
                {
                    string tmp = GT.ReadLine();
                    double data;
                    if (double.TryParse(tmp, out data))
                    {
                        GTList.Add(data);
                    }
                }
                GT.Close();
                while (!test.EndOfStream)
                {
                    string tmp = test.ReadLine();
                    double data;
                    if (double.TryParse(tmp, out data))
                    {
                        testList.Add(data);
                    }
                }
                test.Close();
                // write to file
                writeToFileHighCrossCorrelation(GTList.ToArray(), testList.ToArray(), entry.Key);
            }
        }
    }
}
