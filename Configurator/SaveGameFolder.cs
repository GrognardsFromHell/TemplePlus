using System;
using System.Runtime.InteropServices;

namespace TemplePlusConfig
{
    class SaveGameFolder
    {

        private static readonly Guid SavedGames = new Guid("4C5C32FF-BB9D-43b0-B5B4-2D72E54EAAA4");

        [DllImport("shell32.dll")]
        private static extern int SHGetKnownFolderPath(
            [MarshalAs(UnmanagedType.LPStruct)] Guid rfid,
            uint dwFlags,
            IntPtr hToken,
            out IntPtr pszPath
        );


        static SaveGameFolder()
        {
            int hresult;
            if ((hresult = SHGetKnownFolderPath(SavedGames, 0, IntPtr.Zero, out IntPtr pszPath)) == 0)
            {
                Path = Marshal.PtrToStringUni(pszPath);
                Marshal.FreeCoTaskMem(pszPath);
            } else
            {
                throw new COMException("Unable to determine location of save game folder.", hresult);
            }
        }

        public static string Path { get; internal set; }

    }
}
