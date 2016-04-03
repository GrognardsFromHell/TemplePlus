using System;
using System.Runtime.InteropServices;
using System.Windows;

// ReSharper disable InconsistentNaming

namespace TemplePlusConfig
{

    internal struct POINTL
    {
        public int x;
        public int y;
    }

    [Flags]
    internal enum DM
    {
        Orientation = 0x1,
        PaperSize = 0x2,
        PaperLength = 0x4,
        PaperWidth = 0x8,
        Scale = 0x10,
        Position = 0x20,
        NUP = 0x40,
        DisplayOrientation = 0x80,
        Copies = 0x100,
        DefaultSource = 0x200,
        PrintQuality = 0x400,
        Color = 0x800,
        Duplex = 0x1000,
        YResolution = 0x2000,
        TTOption = 0x4000,
        Collate = 0x8000,
        FormName = 0x10000,
        LogPixels = 0x20000,
        BitsPerPixel = 0x40000,
        PelsWidth = 0x80000,
        PelsHeight = 0x100000,
        DisplayFlags = 0x200000,
        DisplayFrequency = 0x400000,
        ICMMethod = 0x800000,
        ICMIntent = 0x1000000,
        MediaType = 0x2000000,
        DitherType = 0x4000000,
        PanningWidth = 0x8000000,
        PanningHeight = 0x10000000,
        DisplayFixedOutput = 0x20000000
    }

    [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Ansi)]
    internal struct DEVMODE
    {
        public const int CCHDEVICENAME = 32;
        public const int CCHFORMNAME = 32;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = CCHDEVICENAME)]
        [FieldOffset(0)]
        public string dmDeviceName;
        [FieldOffset(32)]
        public short dmSpecVersion;
        [FieldOffset(34)]
        public short dmDriverVersion;
        [FieldOffset(36)]
        public short dmSize;
        [FieldOffset(38)]
        public short dmDriverExtra;
        [FieldOffset(40)]
        public DM dmFields;

        [FieldOffset(44)]
        public short dmOrientation;
        [FieldOffset(46)]
        public short dmPaperSize;
        [FieldOffset(48)]
        public short dmPaperLength;
        [FieldOffset(50)]
        public short dmPaperWidth;
        [FieldOffset(52)]
        public short dmScale;
        [FieldOffset(54)]
        public short dmCopies;
        [FieldOffset(56)]
        public short dmDefaultSource;
        [FieldOffset(58)]
        public short dmPrintQuality;

        [FieldOffset(44)]
        public POINTL dmPosition;
        [FieldOffset(52)]
        public int dmDisplayOrientation;
        [FieldOffset(56)]
        public int dmDisplayFixedOutput;

        [FieldOffset(60)]
        public short dmColor; // See note below!
        [FieldOffset(62)]
        public short dmDuplex; // See note below!
        [FieldOffset(64)]
        public short dmYResolution;
        [FieldOffset(66)]
        public short dmTTOption;
        [FieldOffset(68)]
        public short dmCollate; // See note below!
        [FieldOffset(72)]
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = CCHFORMNAME)]
        public string dmFormName;
        [FieldOffset(102)]
        public short dmLogPixels;
        [FieldOffset(104)]
        public int dmBitsPerPel;
        [FieldOffset(108)]
        public int dmPelsWidth;
        [FieldOffset(112)]
        public int dmPelsHeight;
        [FieldOffset(116)]
        public int dmDisplayFlags;
        [FieldOffset(116)]
        public int dmNup;
        [FieldOffset(120)]
        public int dmDisplayFrequency;
    }

    /// <summary>
    /// This class is needed to circumvent DPI awareness and to get the *real* 
    /// screen resolution used for 3d modes.
    /// </summary>
    public class ScreenResolution
    {

        const int ENUM_CURRENT_SETTINGS = -1;

        [DllImport("user32.dll")]
        private static extern bool EnumDisplaySettings(string deviceName, int modeNum, ref DEVMODE devMode);

        public static Size ScreenSize
        {
            get
            {
                var devMode = new DEVMODE {dmSize = (short) Marshal.SizeOf<DEVMODE>()};
                if (!EnumDisplaySettings(null, ENUM_CURRENT_SETTINGS, ref devMode))
                {
                    return new Size(1024, 768);
                }
                return new Size(devMode.dmPelsWidth, devMode.dmPelsHeight);
            }
        }

    }
}