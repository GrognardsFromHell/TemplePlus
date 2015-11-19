using SharpDX.Direct3D9;
using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;

namespace ParticleEditor
{
    public class TempleDll : IDisposable
    {
        public TempleDll(string path, Device device)
        {
            if (Instance != null)
            {
                throw new InvalidOperationException("Cannot instantiate more than one TempleDll at once");
            }

            var editorDir = Path.GetDirectoryName(Assembly.GetAssembly(typeof(TempleDll)).Location);
            var tpData = Path.Combine(editorDir, "tpdata");
            if (!Directory.Exists(tpData))
            {
                tpData = Path.Combine(editorDir, "..", "tpdata");
            }
            if (!Directory.Exists(tpData))
            {
                throw new InvalidOperationException(
                    "Unable to find TemplePlus data in the installation directory of the editor."
                );
            }

            Handle = TempleDll_Load(path, tpData, device.NativePointer);
            if (Handle == IntPtr.Zero)
            {
                throw new InvalidOperationException(
                    Marshal.PtrToStringAnsi(TempleDll_GetLastError())
                );
            }
            Instance = this;
        }

        public IntPtr Handle { get; private set; }

        public static TempleDll Instance { get; private set; }
        public static string LastError { get; internal set; }

        public void Dispose()
        {
            if (Instance == this)
            {
                Instance = null;
            }
            if (Handle != IntPtr.Zero)
            {
                TempleDll_Unload(Handle);
                Handle = IntPtr.Zero;
            }
        }

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr TempleDll_Load(string filename, string tpDataPath, IntPtr device);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr TempleDll_GetLastError();

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void TempleDll_Unload(IntPtr handle);
    }
}