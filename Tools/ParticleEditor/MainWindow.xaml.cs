using System;
using System.IO;
using System.Windows;
using Microsoft.Win32;
using Microsoft.WindowsAPICodePack.Dialogs;
using ParticleEditor.Properties;
using ParticleModel;

namespace ParticleEditor
{
    /// <summary>
    ///     Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public static readonly DependencyProperty ModelProperty = DependencyProperty.Register(
            "Model", typeof (EditorViewModel), typeof (MainWindow), new PropertyMetadata(default(EditorViewModel)));

        public MainWindow() {
            InitializeComponent();
            PreviewControl.DataPath = Settings.Default.TemplePath;
            Model = new EditorViewModel();
            DataContext = Model;
        }

        public EditorViewModel Model
        {
            get { return (EditorViewModel) GetValue(ModelProperty); }
            set { SetValue(ModelProperty, value); }
        }

        private void SaveVideo_OnClick(object sender, RoutedEventArgs e)
        {
            if (Model.SelectedSystem == null)
            {
                MessageBox.Show("Please select a particle system before using this functionality.",
                    "Particle System Required");
                return;
            }

            var ofd = new SaveFileDialog
            {
                AddExtension = true,
                DefaultExt = "mp4",
                Filter = "MP4 Video|*.mp4|All Files|*.*"
            };
            var result = ofd.ShowDialog(this);
            if (result.Value)
            {
                VideoRenderer.RenderVideo(PreviewControl.Device, 
                    Settings.Default.TemplePath,
                    Model.SelectedSystem,
                    ofd.FileName);
            }
        }

        private void OpenPartSysFile(object sender, RoutedEventArgs e)
        {
            var ofd = new OpenFileDialog {Filter = "Particle System Files|*.tab|All Files|*.*"};
            if (!string.IsNullOrWhiteSpace(Settings.Default.TemplePath))
            {
                ofd.InitialDirectory = Path.Combine(Settings.Default.TemplePath, "data", "rules");
            }
            var result = ofd.ShowDialog(this);
            if (result.Value)
            {
                var file = new PartSysFile();
                file.Load(ofd.FileName);
                Model.Systems = file.Specs;
                Model.OpenedFileName = ofd.FileName;
            }
        }


        private void ChooseDataPath()
        {
            var dialog = new CommonOpenFileDialog
            {
                IsFolderPicker = true,
                InitialDirectory = Settings.Default.TemplePath
            };

            var result = dialog.ShowDialog();
            if (result == CommonFileDialogResult.Ok)
            {
                var templeDll = Path.Combine(dialog.FileName, "temple.dll");
                if (!File.Exists(templeDll))
                {
                    MessageBox.Show("The chosen ToEE installation directory does not seem to be valid.\n"
                                    + "Couldn't find temple.dll.",
                        "Invalid ToEE Directory");
                    return;
                }

                Settings.Default.TemplePath = dialog.FileName;
                Settings.Default.Save();
                PreviewControl.DataPath = dialog.FileName;
            }
        }

        private void PreviewControl_OnConfigureDataPath(object sender, EventArgs e)
        {
            ChooseDataPath();
        }

        private void ChooseDataPath_OnClick(object sender, RoutedEventArgs e)
        {
            ChooseDataPath();
        }

        private void DeleteEmitter_Click(object sender, RoutedEventArgs e)
        {
            if (Model.SelectedEmitter != null)
            {
                Model.SelectedSystem.Emitters.Remove(Model.SelectedEmitter);
            }
        }
    }
}