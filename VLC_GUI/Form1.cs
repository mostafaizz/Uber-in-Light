using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;

namespace VLC_GUI
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            // add grids
            for(int i = 0;i < sideA.Length;i++)
            {
                GridsComboBox.Items.Add(sideB[i].ToString() + "x" + sideA[i].ToString());
            }
            GridsComboBox.SelectedIndex = 0;
            // add symbol times
            foreach(int t in timePerSymbol)
            {
                SymbolTimeComboBox.Items.Add(t.ToString() + " ms");
            }
            SymbolTimeComboBox.SelectedIndex = 1;
            // add resolutions
            foreach(int r in res)
            {
                ResolutionComboBox.Items.Add(r.ToString() + "p");
            }
            ResolutionComboBox.SelectedIndex = 1;
        }
        int[] sideA = new int[] { 3, 6, 12, 15, 16, 20, 30 };
        int[] sideB = new int[] { 2, 5, 10, 12, 15, 18, 20 };
        int[] timePerSymbol = new int[] { 300, 600, 1000, 2000 };
        int[] res = new int[] { 480, 720, 1080 };
        int[] res1 = new int[] { 640, 1280, 1920 };
        private void CalibrateButton_Click(object sender, EventArgs e)
        {
            Process.Start("VLC_tester.exe", "-chess");
        }

        private void FileTextBox_MouseHover(object sender, EventArgs e)
        {
            ToolTip tip = new ToolTip();
            tip.Show(FileTextBox.Text, FileTextBox, 2000);
        }

        private void BrowseButton_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofdlg = new OpenFileDialog();
            if(VideoRadioButton.Checked)
            {
                // get video
                ofdlg.Filter = "All Video|*.AVI;*.MP4";
            }
            else if(ImageRadioButton.Checked)
            {
                // get image
                ofdlg.Filter = "All Images|*.BMP;*.JPG;*.JPEG;*.JPE;*.GIF;*.TIF;*.TIFF;*.PNG";
            }
            ofdlg.Multiselect = false;
            if(ofdlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                FileTextBox.Text = ofdlg.FileName;
            }
        }

        private void SendButton_Click(object sender, EventArgs e)
        {
            // check file
            if (!File.Exists(FileTextBox.Text))
            {
                MessageBox.Show("Please Select Input File First!", "Warning");
                return;
            }

            if (MsgTextBox.Text.Trim().Length == 0)
            {
                MessageBox.Show("Please Write a Message to Transmit", "Warning");
                return;
            }
            // prepare the command line
            string args = string.Format("-s -sideB {0} -sideA {1} -if \"{2}\" -symbols 8Freq8org.symbol " +
                "-m 9 -fps 60 -time {3} -res {4} {5}  -codec XVID -synch 1 -total 5 -seed {6} -live",
                sideB[GridsComboBox.SelectedIndex], sideA[GridsComboBox.SelectedIndex],
                FileTextBox.Text, 
                timePerSymbol[SymbolTimeComboBox.SelectedIndex], 
                res1[ResolutionComboBox.SelectedIndex], res[ResolutionComboBox.SelectedIndex],
                MsgTextBox.Text);

            if (VideoRadioButton.Checked)
            {
                // send video
                args += " -v";
            }
            // then send
            Process.Start("VLC_tester.exe", args);
        }

    }
}
