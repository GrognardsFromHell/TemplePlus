using System.IO;

namespace Configurator
{
    public enum VanillaDllVersion
    {
        Patch2,
        Patch3,
        Unknown
    }

    internal struct VanillaDllSignature
    {
        public uint Offset;

        public byte[] Pattern;

        public VanillaDllVersion Version;

        public static readonly VanillaDllSignature Patch2Signature =
            new VanillaDllSignature
            {
                Offset = 0x13c8,
                Pattern = new byte[]
                {
                    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                    0x81, 0xEC, 0x8C, 0x00, 0x00, 0x00, 0x55, 0x56, 0x68, 0xF0, 0xC7, 0x26, 0x10, 0x33, 0xED, 0x55,
                    0x55, 0xFF, 0x15, 0x34, 0xC0, 0x26, 0x10, 0x8B, 0xF0, 0x89, 0x74, 0x24, 0x08, 0xFF, 0x15, 0x38
                },
                Version = VanillaDllVersion.Patch2
            };

        public static readonly VanillaDllSignature Patch3Signature =
            new VanillaDllSignature
            {
                Offset = 0x2f28,
                Pattern = new byte[]
                {
                    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
                    0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, 0x40, 0x00, 0x28, 0x10, 0x68, 0xE8, 0x86, 0x21, 0x10, 0x64,
                    0xA1, 0x00, 0x00, 0x00, 0x00, 0x50, 0x64, 0x89, 0x25, 0x00, 0x00, 0x00, 0x00, 0x83, 0xEC, 0x08
                },
                Version = VanillaDllVersion.Patch3
            };
    }

    public class TempleDllVersion
    {
        public VanillaDllVersion VanillaVersion { get; set; }

        public string Description
        {
            get
            {
                var result = "DLL " + VanillaVersion.ToString();
                if (Co8)
                {
                    result += " (Co8)";
                }
                else if (MoebiusFixes)
                {
                    result += " (GOG)";
                }
                return result;
            }
        }

        public bool Co8 { get; set; }

        public bool MoebiusFixes { get; set; }

        public bool Supported { get; set; }

        public static TempleDllVersion Identify(string dllPath)
        {
            // Slurp the file into memory
            var dllData = File.ReadAllBytes(dllPath);

            var result = new TempleDllVersion();

            if (CheckSignature(dllData, VanillaDllSignature.Patch3Signature.Offset,
                VanillaDllSignature.Patch3Signature.Pattern))
            {
                result.VanillaVersion = VanillaDllVersion.Patch3;
                // No known mods of patch 3
                return result;
            }

            if (!CheckSignature(dllData, VanillaDllSignature.Patch2Signature.Offset,
                VanillaDllSignature.Patch2Signature.Pattern))
            {
                // Unknown version
                result.VanillaVersion = VanillaDllVersion.Unknown;
                return result;
            }

            // Anything Patch2 should be supported, really
            result.Supported = true;

            if (IsCo8SavehookPresent(dllData))
            {
                result.Co8 = true;
            }

            if (CheckSignature(dllData, 0x15c0, new byte[]
            {
                0xE8, 0xFB, 0xA1, 0x0F, 0x00
            }))
            {
                result.MoebiusFixes = true;
            }

            return result;
        }

        private static bool IsCo8SavehookPresent(byte[] dllData)
        {
            // Check for the Co8 save/load hook, because that means it's a Co8 Dll and includes moebius/spellslinger fixes
            var loadCallPresent = CheckSignature(dllData, 0x2d29, new byte[]
            {
                0xE8, 0x38, 0x3C, 0xEB, 0x01
            });
            var saveCallPresent = CheckSignature(dllData, 0x4865, new byte[]
            {
                0xE8, 0xD6, 0x20, 0xEB, 0x01
            });
            // Check for _co8init, load, save strings in relocation segment
            var co8Strings = CheckSignature(dllData, 0x346920, new byte[]
            {
                0x5F, 0x63, 0x6F, 0x38, 0x69, 0x6E, 0x69, 0x74, 0x00,
                0x6C, 0x6F, 0x61, 0x64, 0x00,
                0x73, 0x61, 0x76, 0x65, 0x00
            });

            return loadCallPresent && saveCallPresent && co8Strings;
        }

        private static bool CheckSignature(byte[] dllData, uint offset, byte[] data)
        {
            if (dllData.Length < offset + data.Length)
            {
                return false; // Not enough data
            }

            for (var i = 0; i < data.Length; ++i)
            {
                if (dllData[offset + i] != data[i])
                {
                    return false;
                }
            }
            return true;
        }
    }
}