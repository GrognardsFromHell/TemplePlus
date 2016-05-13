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
        

        public MainWindow()
        {
            App._iniParser = new FileIniDataParser();
            App._iniParser.Parser.Configuration.AssigmentSpacer = "";

            InitializeComponent();
            
            if (App.LaunchAfterSave)
            {
                OkButton.Content = "Launch";
            }

            DataContext = App._iniViewModel;

            var saveGameFolder = KnownFolders.SavedGames.Path;
            App._iniPath = Path.Combine(saveGameFolder, "TemplePlus", "TemplePlus.ini");

            if (File.Exists(App._iniPath))
            {
                var iniData = App._iniParser.ReadFile(App._iniPath, App.IniEncoding);
                App._iniViewModel.LoadFromIni(iniData);
            }

            // Auto detect an installation if the INI didnt exist, or if 
            // the path is not set in the ini
            if (string.IsNullOrEmpty(App._iniViewModel.InstallationPath))
            {
                App._iniViewModel.AutoDetectInstallation();
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            IniData iniData;

            if (File.Exists(App._iniPath))
            {
                iniData = App._iniParser.ReadFile(App._iniPath, Encoding.UTF8);
            }
            else
            {
                // Copy the INI configuration from our parser
                iniData = new IniData {Configuration = App._iniParser.Parser.Configuration};
            }

            var iniDir = Path.GetDirectoryName(App._iniPath);
            if (iniDir != null)
            {
                Directory.CreateDirectory(iniDir);
            }

            App._iniViewModel.SaveToIni(iniData);

            App._iniParser.WriteFile(App._iniPath, iniData, App.IniEncoding);

            Close();

            if (App.LaunchAfterSave)
            {
                var path = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                var tpExe = Path.Combine(path, "TemplePlus.exe");
                Process.Start(tpExe);
            }
        }

        private void HousRulesBtnClick(object sender, RoutedEventArgs e)
        {
            var newWindow = new HouseRulesWindow();
            newWindow.ShowDialog();

        }
    }
}