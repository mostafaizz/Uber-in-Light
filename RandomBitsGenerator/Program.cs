using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RandomBitsGenerator
{
    class Program
    {
        static void Main(string[] args)
        {
            int bitsPerSecond = 10; // bits
            int time = 120; // seconds
            int[] cellPerWidth = new int[] { 3, 6, 12, 15, 16, 20, 30 };
            int[] cellPerHeight = new int[] { 2, 5, 10, 12, 15, 18, 20 };
            for (int i = 0; i < cellPerHeight.Length; i++)
            {
                StreamWriter sw = new StreamWriter(string.Format("{0:00}x{1}x{2}x{3}.rand", cellPerHeight[i], cellPerWidth[i],bitsPerSecond,time));
                int length = cellPerHeight[i] * cellPerWidth[i] * bitsPerSecond * time; // 30 seconds
                Random r = new Random((int)DateTime.Now.Ticks);
                for (int j = 0; j < length; j++)
                {
                    sw.Write(r.Next(2));
                }
                sw.Close();
            }
        }
    }
}
