using System;
using System.IO;

namespace LCS
{
    class Program
    {
        static void Main(string[] args)
        {
            StreamWriter sw = new StreamWriter("test3.rand");
            Random r = new Random(1000);
            for(int i = 0;i < 200;i++)
            {
                sw.Write(r.Next(0,2));
            }
            sw.Close();
        }
    }
}