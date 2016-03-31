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

        /// <summary>
        /// Tries to find an installation directory based on common locations and the Windows registry.
        /// </summary>
        public void AutoDetectInstallation()
        {
            var gogKey = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\\GOG.com\\Games\\1207658889");
            var gogPath = gogKey?.GetValue("PATH", null) as string;

            if (gogPath != null)
            {
                var gogStatus = InstallationDirValidator.Validate(gogPath);
                if (gogStatus.Valid)
                {
                    InstallationPath = gogPath;
                }
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