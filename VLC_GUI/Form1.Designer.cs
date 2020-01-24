namespace VLC_GUI
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.CalibrateButton = new System.Windows.Forms.Button();
            this.SendButton = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.GridsComboBox = new System.Windows.Forms.ComboBox();
            this.SymbolTimeComboBox = new System.Windows.Forms.ComboBox();
            this.ResolutionComboBox = new System.Windows.Forms.ComboBox();
            this.MsgTextBox = new System.Windows.Forms.TextBox();
            this.VideoRadioButton = new System.Windows.Forms.RadioButton();
            this.ImageRadioButton = new System.Windows.Forms.RadioButton();
            this.FileTextBox = new System.Windows.Forms.TextBox();
            this.BrowseButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // CalibrateButton
            // 
            this.CalibrateButton.Location = new System.Drawing.Point(375, 299);
            this.CalibrateButton.Name = "CalibrateButton";
            this.CalibrateButton.Size = new System.Drawing.Size(104, 25);
            this.CalibrateButton.TabIndex = 0;
            this.CalibrateButton.Text = "Calibrate";
            this.CalibrateButton.UseVisualStyleBackColor = true;
            this.CalibrateButton.Click += new System.EventHandler(this.CalibrateButton_Click);
            // 
            // SendButton
            // 
            this.SendButton.Location = new System.Drawing.Point(502, 299);
            this.SendButton.Name = "SendButton";
            this.SendButton.Size = new System.Drawing.Size(104, 25);
            this.SendButton.TabIndex = 1;
            this.SendButton.Text = "Transmit";
            this.SendButton.UseVisualStyleBackColor = true;
            this.SendButton.Click += new System.EventHandler(this.SendButton_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 50);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(50, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Input File";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 78);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Random Seed";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(13, 107);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(31, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Grids";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(13, 138);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(67, 13);
            this.label4.TabIndex = 5;
            this.label4.Text = "Symbol Time";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(13, 169);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(57, 13);
            this.label5.TabIndex = 6;
            this.label5.Text = "Resolution";
            // 
            // GridsComboBox
            // 
            this.GridsComboBox.FormattingEnabled = true;
            this.GridsComboBox.Location = new System.Drawing.Point(120, 104);
            this.GridsComboBox.Name = "GridsComboBox";
            this.GridsComboBox.Size = new System.Drawing.Size(150, 21);
            this.GridsComboBox.TabIndex = 7;
            this.GridsComboBox.Text = " ";
            // 
            // SymbolTimeComboBox
            // 
            this.SymbolTimeComboBox.FormattingEnabled = true;
            this.SymbolTimeComboBox.Location = new System.Drawing.Point(120, 136);
            this.SymbolTimeComboBox.Name = "SymbolTimeComboBox";
            this.SymbolTimeComboBox.Size = new System.Drawing.Size(150, 21);
            this.SymbolTimeComboBox.TabIndex = 8;
            this.SymbolTimeComboBox.Text = " ";
            // 
            // ResolutionComboBox
            // 
            this.ResolutionComboBox.FormattingEnabled = true;
            this.ResolutionComboBox.Location = new System.Drawing.Point(120, 167);
            this.ResolutionComboBox.Name = "ResolutionComboBox";
            this.ResolutionComboBox.Size = new System.Drawing.Size(150, 21);
            this.ResolutionComboBox.TabIndex = 9;
            this.ResolutionComboBox.Text = " ";
            // 
            // MsgTextBox
            // 
            this.MsgTextBox.Location = new System.Drawing.Point(120, 73);
            this.MsgTextBox.Name = "MsgTextBox";
            this.MsgTextBox.Size = new System.Drawing.Size(150, 20);
            this.MsgTextBox.TabIndex = 10;
            this.MsgTextBox.Text = "1";
            // 
            // VideoRadioButton
            // 
            this.VideoRadioButton.AutoSize = true;
            this.VideoRadioButton.Checked = true;
            this.VideoRadioButton.Location = new System.Drawing.Point(120, 13);
            this.VideoRadioButton.Name = "VideoRadioButton";
            this.VideoRadioButton.Size = new System.Drawing.Size(52, 17);
            this.VideoRadioButton.TabIndex = 11;
            this.VideoRadioButton.TabStop = true;
            this.VideoRadioButton.Text = "Video";
            this.VideoRadioButton.UseVisualStyleBackColor = true;
            // 
            // ImageRadioButton
            // 
            this.ImageRadioButton.AutoSize = true;
            this.ImageRadioButton.Location = new System.Drawing.Point(207, 12);
            this.ImageRadioButton.Name = "ImageRadioButton";
            this.ImageRadioButton.Size = new System.Drawing.Size(54, 17);
            this.ImageRadioButton.TabIndex = 12;
            this.ImageRadioButton.TabStop = true;
            this.ImageRadioButton.Text = "Image";
            this.ImageRadioButton.UseVisualStyleBackColor = true;
            // 
            // FileTextBox
            // 
            this.FileTextBox.Location = new System.Drawing.Point(120, 47);
            this.FileTextBox.Name = "FileTextBox";
            this.FileTextBox.Size = new System.Drawing.Size(324, 20);
            this.FileTextBox.TabIndex = 13;
            this.FileTextBox.MouseEnter += new System.EventHandler(this.FileTextBox_MouseHover);
            // 
            // BrowseButton
            // 
            this.BrowseButton.Location = new System.Drawing.Point(464, 47);
            this.BrowseButton.Name = "BrowseButton";
            this.BrowseButton.Size = new System.Drawing.Size(142, 25);
            this.BrowseButton.TabIndex = 14;
            this.BrowseButton.Text = "Browse";
            this.BrowseButton.UseVisualStyleBackColor = true;
            this.BrowseButton.Click += new System.EventHandler(this.BrowseButton_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(620, 336);
            this.Controls.Add(this.BrowseButton);
            this.Controls.Add(this.FileTextBox);
            this.Controls.Add(this.ImageRadioButton);
            this.Controls.Add(this.VideoRadioButton);
            this.Controls.Add(this.MsgTextBox);
            this.Controls.Add(this.ResolutionComboBox);
            this.Controls.Add(this.SymbolTimeComboBox);
            this.Controls.Add(this.GridsComboBox);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.SendButton);
            this.Controls.Add(this.CalibrateButton);
            this.Name = "Form1";
            this.Text = "VLC Transmitter";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button CalibrateButton;
        private System.Windows.Forms.Button SendButton;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox GridsComboBox;
        private System.Windows.Forms.ComboBox SymbolTimeComboBox;
        private System.Windows.Forms.ComboBox ResolutionComboBox;
        private System.Windows.Forms.TextBox MsgTextBox;
        private System.Windows.Forms.RadioButton VideoRadioButton;
        private System.Windows.Forms.RadioButton ImageRadioButton;
        private System.Windows.Forms.TextBox FileTextBox;
        private System.Windows.Forms.Button BrowseButton;
    }
}

