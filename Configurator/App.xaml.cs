using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using IniParser;

namespace TemplePlusConfig
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {

        public static bool LaunchAfterSave { get; set; }

        public static  Encoding IniEncoding = new UTF8Encoding(false);

        public static string _iniPath;

        public static  IniViewModel _iniViewModel = new IniViewModel();

        public static  FileIniDataParser _iniParser;

    }
}
