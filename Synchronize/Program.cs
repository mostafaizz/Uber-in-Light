using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Synchronize
{
    class Program
    {
        static List<string> ignoreTypes = new List<string>();
        static void recursiveCopy(string src,string dst)
        {
            Console.WriteLine("Copying directory {0} !", src);
            // copy files
            string[] files = Directory.GetFiles(src);
            foreach(string file in files)
            {
                FileInfo finf = new FileInfo(file);
                if (ignoreTypes.Contains(finf.Extension))
                {
                    Console.WriteLine("ignoring file {0} !", file);
                }
                else
                {
                    Console.WriteLine("copying file {0} !", file);
                    File.Copy(file, dst + "\\" + finf.Name);
                }
            }
            // copy directories
            string[] dirs = Directory.GetDirectories(src);
            foreach (string dir in dirs)
            {
                DirectoryInfo dinf = new DirectoryInfo(dir);
                // create a new empty directory
                string dstName = dst + "\\" + dinf.Name;
                Console.WriteLine("Creating directory {0} !", dstName);
                Directory.CreateDirectory(dstName);
                recursiveCopy(dir, dstName);
            }
        }
        static void Main(string[] args)
        {
            StreamReader setRead = new StreamReader("settings.synch");
            string output = setRead.ReadLine();
            while(!setRead.EndOfStream)
            {
                string tmp = setRead.ReadLine();
                ignoreTypes.Add(tmp);
            }
            setRead.Close();
            // first delete folder in the output location

            if (Directory.Exists(output))
            {
                try
                {
                    Console.WriteLine("Trying to delete directory {0}", output);
                    Directory.Delete(output, true);
                    Console.WriteLine("Delete directory {0} was successful!", output);
                }
                catch (Exception)
                {
                    Console.WriteLine("Delete directory {0} failed!", output);
                }
            }
            
            // create a new empty directory
            Console.WriteLine("Creating directory {0} !", output);
            Directory.CreateDirectory(output);
            // then start copying
            recursiveCopy(Environment.CurrentDirectory, output);
            // done
            Console.WriteLine("Done!");
        }
    }
}
