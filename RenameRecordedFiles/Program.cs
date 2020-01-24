using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using WMPLib;

namespace RenameRecordedFiles
{

    enum Decodeing
    {
        FFT,
        FFT_NO_RANDOM,
        CROSS_CORRELATION
    }
    class Program
    {
        static Double getDuration(string file)
        {
            WindowsMediaPlayer wmp = new WindowsMediaPlayer();
            IWMPMedia mediainfo = wmp.newMedia(file);
            return mediainfo.duration;
        }

        static int getMode(string file)
        {
            if (file.ToLower().Contains("_ampdiff_"))
            {
                return 1;
            }
            if (file.ToLower().Contains("_splitamp_"))
            {
                return 5;
            }
            if(file.ToLower().Contains("_split_"))
            {
                return 4;
            }
            if (file.ToLower().Contains("_rgb2_"))
            {
                return 9;
            }
            return 0;
        }
        static int getCorrectionCode(string new_name)
        {
            int correction_code = 0;
            if (new_name.ToLower().Contains("hamming"))
            {
                correction_code = 1;
            }
            else if (new_name.ToLower().Contains("solomon"))
            {
                correction_code = 2;
            }
            return correction_code;
        }
        static string getFreq(string new_name)
        {
            // check if symbols file used
            if (new_name.ToLower().Contains("symbol_"))
            {
                int last = new_name.ToLower().IndexOf("symbol_");
                int first = new_name.Substring(0,last - 1).ToLower().LastIndexOf("_") + 1;
                return "-symbols " + new_name.Substring(first, last - first) + ".symbol";
            }
            else
            {
                int zero = 12;
                int one = 8;
                string[] parts = new_name.ToLower().Split(new string[] { "_" }, StringSplitOptions.RemoveEmptyEntries);
                // get zero
                int j = 0;
                for (; j < parts.Length; j++)
                {
                    if (parts[j].EndsWith("hz"))
                    {
                        if (int.TryParse(parts[j].Substring(0, parts[j].Length - 2), out zero))
                        {
                            j++;
                            break;
                        }
                    }
                }
                // get one 
                for (; j < parts.Length; j++)
                {
                    if (parts[j].EndsWith("hz"))
                    {
                        if (int.TryParse(parts[j].Substring(0, parts[j].Length - 2), out one))
                        {
                            break;
                        }
                    }
                }
                return ("-zero " + zero.ToString() + "-one" + one.ToString());
            }
        }
        static string getSymbolTime(string new_name)
        {
            Regex reg = new Regex(@"_\d+ms_");
            Match m = reg.Match(new_name);
            if (m.Success)
            {
                return m.Value.Substring(1, m.Value.Length - 4);
            }
            return "1";
        }
        static string getSideLength(string new_name)
        {
            Regex reg = new Regex(@"_side\d+_");
            if (reg.IsMatch(new_name))
            {
                Match m = reg.Match(new_name);
                return "-side " + m.Value.Substring(5, m.Value.Length - 6);
            }
            string ret = "";
            reg = new Regex(@"_sideA\d+_");
        
            if(reg.IsMatch(new_name))
            { 
                Match m = reg.Match(new_name);
                ret = "-sideA " + m.Value.Substring(6, m.Value.Length - 7);
            }
            reg = new Regex(@"_sideB\d+_");

            if (reg.IsMatch(new_name))
            {
                Match m = reg.Match(new_name);
                ret += " -sideB " + m.Value.Substring(6, m.Value.Length - 7);
            }
            return ret;
        }
        static string getFullScreen(string new_name)
        {
            string ret = "";
            Regex reg = new Regex(@"_full\d_");
            if (reg.IsMatch(new_name))
            {
                Match m = reg.Match(new_name);
                ret = "-full " + m.Value.Substring(5, 1);
            }
            return ret;
        }
        static string getTotalTime(string new_name)
        {
            string ret = "";
            Regex reg = new Regex(@"_totaltime\d+_");
            if (reg.IsMatch(new_name))
            {
                Match m = reg.Match(new_name);
                ret = "-total " + m.Value.Substring(10, m.Value.Length - 11);
            }
            return ret;
        }
        static string getSeed(string new_name)
        {
            string ret = "";
            Regex reg = new Regex(@"_seed\d+_");
            if (reg.IsMatch(new_name))
            {
                Match m = reg.Match(new_name);
                ret = "-seed " + m.Value.Substring(5, m.Value.Length - 6);
            }
            return ret;
        }
        static string getSynch(string new_name)
        {
            string ret = "";
            Regex reg = new Regex(@"_synch\d+_");
            if (reg.IsMatch(new_name))
            {
                Match m = reg.Match(new_name);
                ret = "-synch " + m.Value.Substring(6, m.Value.Length - 7);
            }
            return ret;
        }
        static string getCommand(string new_name, string folder)
        {
            int mode = getMode(new_name); /// 0 -  normal, 1 -> AmpDifference
            int correction_code = getCorrectionCode(new_name);
            string symbols = getFreq(new_name);
            string time = getSymbolTime(new_name);
            string sideLength = getSideLength(new_name);
            string fullScreen = getFullScreen(new_name);
            return string.Format(@"VLC_tester.exe -decode {4} -r {3} {5} {7} {8} {9} {10} {11} -roi 1 -m {1} -ec {2} -time {6} -if {0}\\",
                folder, 
                mode, 
                correction_code, 
                symbols, 
                (int)Decodeing.CROSS_CORRELATION,
                "", 
                time,
                sideLength,
                fullScreen,
                getTotalTime(new_name),
                getSeed(new_name),
                getSynch(new_name));
        }

        static void RenameSeparateFiles(string[] aviFiles, string[] mp4Files, string[] movFiles, DirectoryInfo dinf)
        {
            StreamWriter sw = new StreamWriter("r_" + dinf.Name + ".bat");
            for (int i = 0; i < mp4Files.Length; i++)
            {
                FileInfo avif = new FileInfo(aviFiles[i % aviFiles.Length]);
                FileInfo mp4f = new FileInfo(mp4Files[i]);
                string new_name = Path.GetFileNameWithoutExtension(mp4f.Name);
                if (mp4f.Name.Contains("_output"))
                {
                    new_name = mp4f.Name;
                }
                else
                {
                    new_name += "_" + Path.GetFileNameWithoutExtension(avif.Name) + ".mp4"; ;
                }
              
                try
                {
                    File.Move(mp4Files[i], new_name);
                }
                catch(Exception)
                {

                }
                sw.WriteLine(@"{2} > {0}\\{1}.txt", dinf.Name, new_name, getCommand(new_name, dinf.Name) + new_name);
            }
            sw.Close();
            sw = new StreamWriter("r_" + dinf.Name + "mov.bat");
            for (int i = 0; i < movFiles.Length; i++)
            {
                FileInfo avif = new FileInfo(aviFiles[i % aviFiles.Length]);
                FileInfo mp4f = new FileInfo(movFiles[i]);
                string new_name = Path.GetFileNameWithoutExtension(mp4f.Name);
                if (!mp4f.Name.Contains(Path.GetFileNameWithoutExtension(avif.Name)))
                {
                    new_name += "_" + Path.GetFileNameWithoutExtension(avif.Name);
                }
                new_name += ".mp4";
                try
                {
                    File.Move(movFiles[i], new_name);
                }
                catch (Exception exc)
                {
                    Console.WriteLine(exc.Message);
                }
                sw.WriteLine(@"{2} > {0}\\{1}.txt", dinf.Name, new_name, getCommand(new_name, dinf.Name) + new_name);
            }
            sw.Close();
            //if (mp4Files.Length == 0)
            {
                sw = new StreamWriter("r_" + dinf.Name + "_gt.bat");
                // then use the original AVI files as the test
                for (int i = 0; i < aviFiles.Length; i++)
                {
                    FileInfo finf = new FileInfo(aviFiles[i]);
                    string new_name = finf.Name;
                    sw.WriteLine(@"{2} > {0}\\{1}.txt", dinf.Name, new_name, getCommand(new_name, dinf.Name) + new_name);
                }
            }
            sw.Close();
        }
        static void WriteCommandsforCombinedVideo(string[] aviFiles, string videoFile, DirectoryInfo dinf, double start,double increment,double oneVidSize)
        {
            StreamWriter sw = new StreamWriter("r_" + dinf.Name + "_" + videoFile.Replace("\\", "") + ".bat");
            FileInfo mp4f = new FileInfo(videoFile);

            for (int i = 0; i < aviFiles.Length; i++)
            {
                FileInfo avif = new FileInfo(aviFiles[i]);

                sw.WriteLine(@"{2} -start {4} > {0}\\{3}_{1}.mp4.txt",
                    dinf.Name, avif.Name.Substring(0, avif.Name.Length - 4),
                    getCommand(avif.Name, dinf.Name) + videoFile, mp4f.Name, start);
                if (oneVidSize < 1)
                {
                    start += (getDuration(aviFiles[i]) + increment);
                }
                else
                {
                    start += (oneVidSize + increment);
                }
            }
            sw.Close();
        }
        static void Main(string[] args)
        {
            string currentDirectory = Environment.CurrentDirectory;
            string[] aviFiles = Directory.GetFiles(currentDirectory, "*.avi");
            string[] mp4Files = Directory.GetFiles(currentDirectory, "*.mp4");
            string[] movFiles = Directory.GetFiles(currentDirectory, "*.MOV");
            Array.Sort(aviFiles);
            Array.Sort(mp4Files);
            Array.Sort(movFiles);
            DirectoryInfo dinf = new DirectoryInfo(currentDirectory);
            if (args.Length >= 2)
            {
                double incremenet = 0;
                double oneVidSize = 0;
                if(args.Length >= 3)
                {
                    incremenet = double.Parse(args[2]);
                }
                if (args.Length >= 4)
                {
                    oneVidSize = double.Parse(args[3]);
                }
                WriteCommandsforCombinedVideo(aviFiles, args[0], dinf, double.Parse(args[1]), incremenet, oneVidSize);
            }
            else
            {
                RenameSeparateFiles(aviFiles, mp4Files, movFiles, dinf);
            }
            // write play batch file
            StreamWriter sw = new StreamWriter(@"play.bat");
            sw.WriteLine("pause");
            foreach (string aviFile in aviFiles)
            {
                FileInfo finf = new FileInfo(aviFile);
                sw.WriteLine("\"C:\\Program Files (x86)\\K-Lite Codec Pack\\MPC-HC\\mpc-hc.exe\"  {0} /fullscreen /play /close", finf.Name);
                //sw.WriteLine("taskkill /F /IM mpc-hc.exe");
                sw.WriteLine("pause");
            }
            sw.Close();
        }   
    }
}
