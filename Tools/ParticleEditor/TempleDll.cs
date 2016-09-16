using System;
using System.IO;
using System.Reflection;
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
            try
            {
                Handle = TempleDll_Load(path, tpData);
            }
            catch (Exception e){

            }
            
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

        public float Scale
        {
            set
            {
                TempleDll_SetScale(Handle, value);
            }
        }

        public void CenterOn(float x, float y, float z)
        {
            TempleDll_CenterOn(Handle, x, y, z);
        }

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

        public void SetRenderTarget(IntPtr surface)
        {
            TempleDll_SetRenderTarget(Handle, surface);
        }

        public void Flush()
        {
            TempleDll_Flush(Handle);
        }

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr TempleDll_Load(string filename, string tpDataPath);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr TempleDll_GetLastError();

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void TempleDll_Unload(IntPtr handle);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void TempleDll_SetRenderTarget(IntPtr handle, IntPtr surface);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void TempleDll_Flush(IntPtr handle);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void TempleDll_SetScale(IntPtr handle, float scale);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void TempleDll_CenterOn(IntPtr handle, float x, float y, float z);
    }
}