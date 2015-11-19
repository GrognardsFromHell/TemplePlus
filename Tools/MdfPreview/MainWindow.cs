using MdfPreview.Properties;
using Microsoft.WindowsAPICodePack.Dialogs;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MdfPreview
{

    public partial class MainWindow : Form
    {

        private MdfPreviewNative _mdfPreviewNative;

        private Size currentSize;

        private bool IsDataDirValid(string dir)
        {
            var dataFile = Path.Combine(dir, "ToEE1.dat");
            if (!File.Exists(dataFile)) {
                return false;
            }
            dataFile = Path.Combine(dir, "temple.dll");
            if (!File.Exists(dataFile))
            {
                return false;
            }
            return true;         
        }

        public MainWindow()
        {

            InitializeComponent();

            if (!IsDataDirValid(Settings.Default.ToEEDir))
            {
                ChooseDataPath();
            }

            _mdfPreviewNative = new MdfPreviewNative(Settings.Default.ToEEDir);

            currentSize = splitContainer1.Panel1.ClientSize;

            _mdfPreviewNative.InitDevice(
                splitContainer1.Panel1.Handle,
                currentSize.Width,
                currentSize.Height
                );

            logDisplay.AppendText(_mdfPreviewNative.GetAndClearLog());

            timer1.Enabled = true;

        }

        private void ChooseDataPath()
        {
            var dialog = new CommonOpenFileDialog
            {
                IsFolderPicker = true,
                InitialDirectory = Settings.Default.ToEEDir
            };

            var result = dialog.ShowDialog();
            if (result == CommonFileDialogResult.Ok)
            {
                if (!IsDataDirValid(dialog.FileName))
                {
                    MessageBox.Show("The chosen data directory does not seem to be valid.\n"
                                    + "Couldn't find ToEE1.dat.",
                        "Invalid Data Directory");
                    return;
                }

                Settings.Default.ToEEDir = dialog.FileName;
                Settings.Default.Save();
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (_mdfPreviewNative == null)
            {
                return;
            }

            if (!currentSize.Equals(splitContainer1.Panel1.ClientSize)) {
                currentSize = splitContainer1.Panel1.ClientSize;

                _mdfPreviewNative.SetSize(currentSize.Width, currentSize.Height);
            }

            _mdfPreviewNative.Render();
        }

        private void MainWindow_FormClosed(object sender, FormClosedEventArgs e)
        {
            _mdfPreviewNative.Dispose();
        }

        private bool dragging = false;
        private Point dragStart;
        private PointF cameraPos = new PointF(0, 0);

        private void splitContainer1_Panel1_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left) {
                dragging = true;
                splitContainer1.Panel1.Capture = true;
                dragStart = e.Location;
            }
        }

        private void splitContainer1_Panel1_MouseMove(object sender, MouseEventArgs e)
        {
            if (dragging && e.Button == MouseButtons.Left) {
                var diffX = e.Location.X - dragStart.X;
                var diffY = e.Location.Y - dragStart.Y;
                _mdfPreviewNative.SetCameraPos(cameraPos.X + diffX, cameraPos.Y + diffY);
            }

            // Calculate mouse position relative to this panel
            float worldX, worldY, worldZ;
            _mdfPreviewNative.ScreenToWorld(e.Location.X, e.Location.Y,
                out worldX, out worldY, out worldZ);
            worldPosXLabel.Text = worldX.ToString();
            worldPosYLabel.Text = worldY.ToString();
            worldPosZLabel.Text = worldZ.ToString();
        }

        private void splitContainer1_Panel1_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                if (dragging)
                {
                    var diffX = e.Location.X - dragStart.X;
                    var diffY = e.Location.Y - dragStart.Y;
                    cameraPos.X += diffX;
                    cameraPos.Y += diffY;
                    _mdfPreviewNative.SetCameraPos(cameraPos.X, cameraPos.Y);
                }
                splitContainer1.Panel1.Capture = false;
                dragging = false;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() != DialogResult.OK)
            {
                return;
            }

            var skmFile = openFileDialog1.FileName;
            var skaFile = Path.ChangeExtension(skmFile, ".ska");

            if (!_mdfPreviewNative.LoadModel(skmFile, skaFile))
            {
                MessageBox.Show(_mdfPreviewNative.Error);
            }
            
            float camX, camY;
            _mdfPreviewNative.GetCameraPos(out camX, out camY);
            cameraPos = new PointF(camX, camY);
            
            logDisplay.AppendText(_mdfPreviewNative.GetAndClearLog());
        }

        private void button2_Click(object sender, EventArgs e)
        {

            if (openFileDialog2.ShowDialog() != DialogResult.OK)
            {
                return;
            }

            var mdfFile = openFileDialog2.FileName;

            if (!_mdfPreviewNative.LoadMaterial(mdfFile))
            {
                MessageBox.Show(_mdfPreviewNative.Error);
            }

            logDisplay.AppendText(_mdfPreviewNative.GetAndClearLog());

        }

        private void rotation_Scroll(object sender, EventArgs e)
        {
            _mdfPreviewNative.Rotation = rotation.Value;
        }

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            _mdfPreviewNative.Scale = trackBar1.Value / 100.0f;
        }

    }

}
