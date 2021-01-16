using System.IO;
using System.Collections.Generic;

namespace TemplePlusConfig
{
    public class InstallationDirValidator
    {
        private static readonly string[] RequiredFiles =
        {
            "ToEE1.dat",
            "ToEE2.dat",
            "ToEE3.dat",
            "ToEE4.dat",
            "temple.dll",
            "modules\\ToEE.dat",
            "temple.dll",
            "mss32.dll",
            "tio.dll",
            "pytoee22.dll",
            "tig.dat",
            "zlib-1.2.1.dll",
            "msvcr70.dll",
            "binkw32.dll"
        };

        public static InstallationDirStatus Validate(string path)
        {
            if (!Directory.Exists(path))
            {
                return CreateInvalid("The directory does not exist.");
            }

            foreach (var requiredFile in RequiredFiles)
            {
                var fullPath = Path.Combine(path, requiredFile);
                if (!File.Exists(fullPath))
                {
                    return CreateInvalid("Required file " + requiredFile + " is missing.");
                }
            }

            var dllPath = Path.Combine(path, "temple.dll");
            var dllVersion = TempleDllVersion.Identify(dllPath);

            if (!dllVersion.Supported)
            {
                return CreateInvalid("Unsupported temple.dll Version found: "
                                     + dllVersion.Description);
            }

            
            // It's a Co8 DLL, check if the required script file exists
            if (dllVersion.Co8)
            {
                var co8Init = Path.Combine(path, "data", "scr", "_co8init.py");
                if (!File.Exists(co8Init))
                {
                    return CreateInvalid("A Co8 temple.dll is being used without the required Co8 data files.");
                }
                if (App._iniViewModel.NeedsCo8Defaults)
                {
                    App._iniViewModel.NumberOfPcs = NumberOfPcsType.Flexible;
                    App._iniViewModel.MaxLevel    = 30;
                    App._iniViewModel.MetamagicStacking = true; // this is true ever since Moebius patch...
                }
            }

            var result = new InstallationDirStatus { Valid = true, Status = dllVersion.Description, IsCo8 = dllVersion.Co8, ModuleNames = new List<string>() };
            DirectoryInfo di = new DirectoryInfo(Path.Combine(path, "modules"));
            FileInfo[] rgFiles = di.GetFiles("*.dat", SearchOption.TopDirectoryOnly);

            foreach (FileInfo datFile in rgFiles)
            {
                result.ModuleNames.Add(datFile.Name.Split('.')[0]);
            }
            return result;

        }

        private static InstallationDirStatus CreateInvalid(string reason)
        {
            return new InstallationDirStatus
            {
                Valid = false,
                Status = reason,
                IsCo8 = false,
                ModuleNames = new List<string>()
            };
        }
    }

    public class InstallationDirStatus
    {
        public bool Valid { get; set; }

        public string Status { get; set; }

        public bool IsCo8 { get; set; }

        public List<string> ModuleNames { get; set; }
    }
}