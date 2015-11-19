using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace MdfPreview
{

    public class MdfPreviewNative : IDisposable
    {

        public MdfPreviewNative(string dllPath)
        {
            if (!dllPath.EndsWith("\\"))
            {
                dllPath += "\\";
            }

            // Try to guess where TPData is 
            var assemblyLoc = Assembly.GetAssembly(typeof(MdfPreviewNative)).Location;
            var tpDataPath = Path.Combine(Path.GetDirectoryName(assemblyLoc), "tpdata");

            // Try one level up
            if (!Directory.Exists(tpDataPath))
            {
                tpDataPath = Path.Combine(Path.GetDirectoryName(assemblyLoc), "..", "tpdata");
            }

            if (!Directory.Exists(tpDataPath))
            {
                MessageBox.Show("The tpdata folder next to this executable does not seem to exist.");
            }

            if (!tpDataPath.EndsWith("\\"))
            {
            }

            _handle = MdfPreviewNative_Load(dllPath, tpDataPath);
        }

        public void InitDevice(IntPtr windowHandle, int renderWidth, int renderHeight)
        {
            _windowHandle = windowHandle;
            MdfPreviewNative_InitDevice(_handle, windowHandle, renderWidth, renderHeight);
        }

        public void Render()
        {
            MdfPreviewNative_Render(_handle);
        }

        public void SetSize(int w, int h)
        {
            MdfPreviewNative_SetRenderSize(_handle, w, h);
        }

        public void SetCameraPos(float x, float y)
        {
            MdfPreviewNative_SetCameraPos(_handle, x, y);
        }
        
        public void GetCameraPos(out float x, out float y)
        {
            MdfPreviewNative_GetCameraPos(_handle, out x, out y);
        }

        public bool LoadMaterial(string name)
        {
            return MdfPreviewNative_SetMaterial(_handle, name);
        }
        public bool LoadModel(string skmFile, string skaFile)
        {
            return MdfPreviewNative_SetModel(_handle, skmFile, skaFile);
        }

        public string Error
        {
            get
            {
                return MdfPreviewNative_GetError(_handle);
            }
        }

        public int Rotation
        {
            set
            {
                var rad = (float)(Math.PI * value / 180.0f);
                MdfPreviewNative_SetRotation(_handle, rad);
            }
        }

        public float Scale
        {
            set
            {
                MdfPreviewNative_SetScale(_handle, value);
            }
        }

        public string GetAndClearLog()
        {
            return MdfPreviewNative_GetAndClearLog(_handle);
        }

        public void ScreenToWorld(float x, float y, out float worldX, out float worldY, out float worldZ)
        {
            MdfPreviewNative_ScreenToWorld(_handle, x, y, out worldX, out worldY, out worldZ);
        }

        private IntPtr _handle;
        private IntPtr _windowHandle;

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private extern static IntPtr MdfPreviewNative_Load(string dllPath, string tpDataPath);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_Unload(IntPtr handle);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_SetCameraPos(IntPtr handle, float x, float y);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_GetCameraPos(IntPtr handle, out float x, out float y);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_ScreenToWorld(IntPtr handle, float x, float y, out float worldX, out float worldY, out float worldZ);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_SetRenderSize(IntPtr handle, int x, int y);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_InitDevice(IntPtr native,
                                            IntPtr windowHandle,
                                            int renderWidth, int renderHeight);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_FreeDevice(IntPtr native);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private extern static bool MdfPreviewNative_SetMaterial(IntPtr native, string materialName);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private extern static bool MdfPreviewNative_SetModel(IntPtr native, string skmFile, string skaFile);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private extern static string MdfPreviewNative_GetError(IntPtr native);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private extern static string MdfPreviewNative_GetAndClearLog(IntPtr native);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_Render(IntPtr native);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_SetRotation(IntPtr native, float rotation);

        [DllImport("MdfPreviewNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private extern static void MdfPreviewNative_SetScale(IntPtr native, float scale);

        public void Dispose()
        {
            if (_handle != IntPtr.Zero)
            {
                MdfPreviewNative_Unload(_handle);
                _handle = IntPtr.Zero;
            }
        }
    }

}
