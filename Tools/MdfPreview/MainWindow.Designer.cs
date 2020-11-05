namespace MdfPreview
{
    partial class MainWindow
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
            this.components = new System.ComponentModel.Container();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.labelAnimType = new System.Windows.Forms.Label();
            this.trackBar3 = new System.Windows.Forms.TrackBar();
            this.loopAnimation = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.trackBar2 = new System.Windows.Forms.TrackBar();
            this.worldPosZLabel = new System.Windows.Forms.Label();
            this.worldPosYLabel = new System.Windows.Forms.Label();
            this.worldPosXLabel = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.trackBar1 = new System.Windows.Forms.TrackBar();
            this.rotation = new System.Windows.Forms.TrackBar();
            this.button2 = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.logDisplay = new System.Windows.Forms.TextBox();
            this.openFileDialog2 = new System.Windows.Forms.OpenFileDialog();
            this.pauseAnimChkBox = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackBar3)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBar2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBar1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.rotation)).BeginInit();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Paint += new System.Windows.Forms.PaintEventHandler(this.splitContainer1_Panel1_Paint);
            this.splitContainer1.Panel1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.splitContainer1_Panel1_MouseDown);
            this.splitContainer1.Panel1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.splitContainer1_Panel1_MouseMove);
            this.splitContainer1.Panel1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.splitContainer1_Panel1_MouseUp);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.pauseAnimChkBox);
            this.splitContainer1.Panel2.Controls.Add(this.checkBox1);
            this.splitContainer1.Panel2.Controls.Add(this.labelAnimType);
            this.splitContainer1.Panel2.Controls.Add(this.trackBar3);
            this.splitContainer1.Panel2.Controls.Add(this.loopAnimation);
            this.splitContainer1.Panel2.Controls.Add(this.label3);
            this.splitContainer1.Panel2.Controls.Add(this.trackBar2);
            this.splitContainer1.Panel2.Controls.Add(this.worldPosZLabel);
            this.splitContainer1.Panel2.Controls.Add(this.worldPosYLabel);
            this.splitContainer1.Panel2.Controls.Add(this.worldPosXLabel);
            this.splitContainer1.Panel2.Controls.Add(this.label2);
            this.splitContainer1.Panel2.Controls.Add(this.label1);
            this.splitContainer1.Panel2.Controls.Add(this.trackBar1);
            this.splitContainer1.Panel2.Controls.Add(this.rotation);
            this.splitContainer1.Panel2.Controls.Add(this.button2);
            this.splitContainer1.Panel2.Controls.Add(this.button1);
            this.splitContainer1.Size = new System.Drawing.Size(765, 617);
            this.splitContainer1.SplitterDistance = 550;
            this.splitContainer1.TabIndex = 0;
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Location = new System.Drawing.Point(16, 452);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(111, 17);
            this.checkBox1.TabIndex = 14;
            this.checkBox1.Text = "Combat Animation";
            this.checkBox1.UseVisualStyleBackColor = true;
            this.checkBox1.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // labelAnimType
            // 
            this.labelAnimType.AutoSize = true;
            this.labelAnimType.Location = new System.Drawing.Point(16, 367);
            this.labelAnimType.Name = "labelAnimType";
            this.labelAnimType.Size = new System.Drawing.Size(80, 13);
            this.labelAnimType.TabIndex = 13;
            this.labelAnimType.Text = "Animation Type";
            // 
            // trackBar3
            // 
            this.trackBar3.Location = new System.Drawing.Point(3, 383);
            this.trackBar3.Maximum = 79;
            this.trackBar3.Name = "trackBar3";
            this.trackBar3.Size = new System.Drawing.Size(196, 45);
            this.trackBar3.TabIndex = 12;
            this.trackBar3.TickFrequency = 5;
            this.trackBar3.Scroll += new System.EventHandler(this.trackBar3_Scroll);
            // 
            // loopAnimation
            // 
            this.loopAnimation.AutoSize = true;
            this.loopAnimation.Location = new System.Drawing.Point(16, 434);
            this.loopAnimation.Name = "loopAnimation";
            this.loopAnimation.Size = new System.Drawing.Size(99, 17);
            this.loopAnimation.TabIndex = 11;
            this.loopAnimation.Text = "Loop Animation";
            this.loopAnimation.UseVisualStyleBackColor = true;
            this.loopAnimation.CheckedChanged += new System.EventHandler(this.loopAnimation_CheckedChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(16, 306);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(45, 13);
            this.label3.TabIndex = 10;
            this.label3.Text = "Offset Z";
            this.label3.Click += new System.EventHandler(this.label3_Click);
            // 
            // trackBar2
            // 
            this.trackBar2.LargeChange = 25;
            this.trackBar2.Location = new System.Drawing.Point(3, 322);
            this.trackBar2.Maximum = 500;
            this.trackBar2.Minimum = -500;
            this.trackBar2.Name = "trackBar2";
            this.trackBar2.Size = new System.Drawing.Size(196, 45);
            this.trackBar2.SmallChange = 5;
            this.trackBar2.TabIndex = 9;
            this.trackBar2.TickFrequency = 100;
            this.trackBar2.Scroll += new System.EventHandler(this.trackBar2_Scroll);
            // 
            // worldPosZLabel
            // 
            this.worldPosZLabel.AutoSize = true;
            this.worldPosZLabel.Location = new System.Drawing.Point(16, 276);
            this.worldPosZLabel.Name = "worldPosZLabel";
            this.worldPosZLabel.Size = new System.Drawing.Size(14, 13);
            this.worldPosZLabel.TabIndex = 8;
            this.worldPosZLabel.Text = "Z";
            // 
            // worldPosYLabel
            // 
            this.worldPosYLabel.AutoSize = true;
            this.worldPosYLabel.Location = new System.Drawing.Point(16, 263);
            this.worldPosYLabel.Name = "worldPosYLabel";
            this.worldPosYLabel.Size = new System.Drawing.Size(14, 13);
            this.worldPosYLabel.TabIndex = 7;
            this.worldPosYLabel.Text = "Y";
            // 
            // worldPosXLabel
            // 
            this.worldPosXLabel.AutoSize = true;
            this.worldPosXLabel.Location = new System.Drawing.Point(16, 250);
            this.worldPosXLabel.Name = "worldPosXLabel";
            this.worldPosXLabel.Size = new System.Drawing.Size(14, 13);
            this.worldPosXLabel.TabIndex = 6;
            this.worldPosXLabel.Text = "X";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 182);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(34, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Zoom";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 92);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(47, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "Rotation";
            this.label1.Click += new System.EventHandler(this.label1_Click);
            // 
            // trackBar1
            // 
            this.trackBar1.LargeChange = 50;
            this.trackBar1.Location = new System.Drawing.Point(13, 198);
            this.trackBar1.Maximum = 500;
            this.trackBar1.Minimum = 50;
            this.trackBar1.Name = "trackBar1";
            this.trackBar1.Size = new System.Drawing.Size(186, 45);
            this.trackBar1.SmallChange = 5;
            this.trackBar1.TabIndex = 3;
            this.trackBar1.TickFrequency = 50;
            this.trackBar1.Value = 50;
            this.trackBar1.Scroll += new System.EventHandler(this.trackBar1_Scroll);
            // 
            // rotation
            // 
            this.rotation.Location = new System.Drawing.Point(13, 111);
            this.rotation.Maximum = 360;
            this.rotation.Name = "rotation";
            this.rotation.Size = new System.Drawing.Size(186, 45);
            this.rotation.TabIndex = 2;
            this.rotation.TickFrequency = 60;
            this.rotation.Scroll += new System.EventHandler(this.rotation_Scroll);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(13, 41);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(186, 23);
            this.button2.TabIndex = 1;
            this.button2.Text = "Load MDF";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(13, 12);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(186, 23);
            this.button1.TabIndex = 0;
            this.button1.Text = "Load SKM";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // timer1
            // 
            this.timer1.Interval = 33;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.Filter = "SKM Files|*.skm|All Files|*.*";
            // 
            // logDisplay
            // 
            this.logDisplay.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.logDisplay.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.logDisplay.Location = new System.Drawing.Point(0, 541);
            this.logDisplay.Multiline = true;
            this.logDisplay.Name = "logDisplay";
            this.logDisplay.ReadOnly = true;
            this.logDisplay.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.logDisplay.Size = new System.Drawing.Size(765, 76);
            this.logDisplay.TabIndex = 1;
            // 
            // openFileDialog2
            // 
            this.openFileDialog2.Filter = "MDF Files|*.mdf|All Files|*.*";
            // 
            // pauseAnimChkBox
            // 
            this.pauseAnimChkBox.AutoSize = true;
            this.pauseAnimChkBox.Location = new System.Drawing.Point(16, 475);
            this.pauseAnimChkBox.Name = "pauseAnimChkBox";
            this.pauseAnimChkBox.Size = new System.Drawing.Size(105, 17);
            this.pauseAnimChkBox.TabIndex = 15;
            this.pauseAnimChkBox.Text = "Pause Animation";
            this.pauseAnimChkBox.UseVisualStyleBackColor = true;
            this.pauseAnimChkBox.CheckedChanged += new System.EventHandler(this.checkBox2_CheckedChanged);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(765, 617);
            this.Controls.Add(this.logDisplay);
            this.Controls.Add(this.splitContainer1);
            this.Name = "MainWindow";
            this.Text = "SKM Viewer";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainWindow_FormClosed);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.trackBar3)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBar2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBar1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.rotation)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.TextBox logDisplay;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.OpenFileDialog openFileDialog2;
        private System.Windows.Forms.TrackBar rotation;
        private System.Windows.Forms.TrackBar trackBar1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label worldPosXLabel;
        private System.Windows.Forms.Label worldPosZLabel;
        private System.Windows.Forms.Label worldPosYLabel;
        private System.Windows.Forms.TrackBar trackBar2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox loopAnimation;
        private System.Windows.Forms.Label labelAnimType;
        private System.Windows.Forms.TrackBar trackBar3;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.CheckBox pauseAnimChkBox;
    }
}

