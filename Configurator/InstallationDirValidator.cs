using System.IO;

namespace Configurator
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
            "msvcr71.dll",
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

            return new InstallationDirStatus {Valid = true, Status = dllVersion.Description};
        }

        private static InstallationDirStatus CreateInvalid(string reason)
        {
            return new InstallationDirStatus
            {
                Valid = false,
                Status = reason
            };
        }
    }

    public class InstallationDirStatus
    {
        public bool Valid { get; set; }

        public string Status { get; set; }
    }
}