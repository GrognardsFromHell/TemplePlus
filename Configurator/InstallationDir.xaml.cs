using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using Microsoft.Win32;
using Microsoft.WindowsAPICodePack.Dialogs;

namespace TemplePlusConfig
{
    /// <summary>
    ///     Interaction logic for InstallationDir.xaml
    /// </summary>
    public partial class InstallationDir : UserControl
    {
        public static readonly DependencyProperty UsingCo8Property = DependencyProperty.Register(
            "UsingCo8", typeof(bool), typeof(InstallationDir), new PropertyMetadata(default(bool))); 

        public static readonly DependencyProperty InstallationPathProperty = DependencyProperty.Register(
            "InstallationPath", typeof (string), typeof (InstallationDir), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty InstallationPathStatusProperty = DependencyProperty.Register(
            "InstallationPathStatus", typeof (InstallationDirStatus), typeof (InstallationDir),
            new PropertyMetadata(default(InstallationDirStatus)));




        public InstallationDir()
        {
            InitializeComponent();

            DependencyPropertyDescriptor.FromProperty(InstallationPathProperty, typeof (InstallationDir))
                .AddValueChanged(this, (sender, args) => RevalidateInstallationDir());
        }

        public string InstallationPath
        {
            get { return (string) GetValue(InstallationPathProperty); }
            set { SetValue(InstallationPathProperty, value); }
        }

        public InstallationDirStatus InstallationPathStatus
        {
            get { return (InstallationDirStatus) GetValue(InstallationPathStatusProperty); }
            set { SetValue(InstallationPathStatusProperty, value); }
        }

        public bool UsingCo8
        {
            get
            {
                return (bool)(InstallationPathStatus.IsCo8);
            }
            set { InstallationPathStatus.IsCo8= value; }
        }

        private void RevalidateInstallationDir()
        {
            InstallationPathStatus = InstallationDirValidator.Validate(InstallationPath);

            if (InstallationPathStatus.Valid)
            {
                OkIcon.Visibility = Visibility.Visible;
                NotOkIcon.Visibility = Visibility.Collapsed;
            }
            else
            {
                OkIcon.Visibility = Visibility.Collapsed;
                NotOkIcon.Visibility = Visibility.Visible;
            }
        }

        private void BrowseButton_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new CommonOpenFileDialog
            {
                Title = "Choose Temple of Elemental Evil Installation Folder",
                IsFolderPicker = true,
                InitialDirectory = InstallationPath,
                AllowNonFileSystemItems = false,
                EnsurePathExists = true,
                EnsureValidNames = true
            };

            if (dialog.ShowDialog(Application.Current.MainWindow) == CommonFileDialogResult.Ok)
            {
                InstallationPath = dialog.FileName;
            }
        }
    }
}