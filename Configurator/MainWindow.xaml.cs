using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using IniParser;
using IniParser.Model;
using Microsoft.WindowsAPICodePack.Shell;
using System.Reflection;
using System.Diagnostics;

namespace TemplePlusConfig
{
    /// <summary>
    ///     Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        // INI file is encoded in UTF-8 without a byte order mark
        private static readonly Encoding IniEncoding = new UTF8Encoding(false);

        private readonly string _iniPath;

        private readonly IniViewModel _iniViewModel = new IniViewModel();

        private readonly FileIniDataParser _iniParser;

        public MainWindow()
        {
            _iniParser = new FileIniDataParser();
            _iniParser.Parser.Configuration.AssigmentSpacer = "";

            InitializeComponent();
            
            if (App.LaunchAfterSave)
            {
                OkButton.Content = "Launch";
            }

            DataContext = _iniViewModel;

            var saveGameFolder = KnownFolders.SavedGames.Path;
            _iniPath = Path.Combine(saveGameFolder, "TemplePlus", "TemplePlus.ini");

            if (File.Exists(_iniPath))
            {
                var iniData = _iniParser.ReadFile(_iniPath, IniEncoding);
                _iniViewModel.LoadFromIni(iniData);
            }

            // Auto detect an installation if the INI didnt exist, or if 
            // the path is not set in the ini
            if (string.IsNullOrEmpty(InstallationDir.InstallationPath))
            {
                InstallationDir.AutoDetectInstallation();
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            IniData iniData;

            if (File.Exists(_iniPath))
            {
                iniData = _iniParser.ReadFile(_iniPath, Encoding.UTF8);
            }
            else
            {
                // Copy the INI configuration from our parser
                iniData = new IniData {Configuration = _iniParser.Parser.Configuration};
            }

            var iniDir = Path.GetDirectoryName(_iniPath);
            if (iniDir != null)
            {
                Directory.CreateDirectory(iniDir);
            }

            _iniViewModel.SaveToIni(iniData);
            
            _iniParser.WriteFile(_iniPath, iniData, IniEncoding);

            Close();

            if (App.LaunchAfterSave)
            {
                var path = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                var tpExe = Path.Combine(path, "TemplePlus.exe");
                Process.Start(tpExe);
            }
        }

    }
}