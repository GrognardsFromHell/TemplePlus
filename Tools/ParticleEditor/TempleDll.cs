using System;
using System.Runtime.InteropServices;

namespace ParticleEditor
{
    public class TempleDll : IDisposable
    {
        public TempleDll(string path)
        {
            if (Instance != null)
            {
                throw new InvalidOperationException("Cannot instantiate more than one TempleDll at once");
            }

            Handle = TempleDll_Load(path);
            Instance = this;
        }

        public IntPtr Handle { get; private set; }

        public static TempleDll Instance { get; private set; }

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
        private static extern IntPtr TempleDll_Load(string filename);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void TempleDll_Unload(IntPtr handle);
    }
}