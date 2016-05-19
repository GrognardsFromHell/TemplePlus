using System;
using System.Runtime.InteropServices;

namespace ParticleEditor
{
    internal class AnimatedModel : IDisposable
    {
        private IntPtr _handle;

        public AnimatedModel(IntPtr handle)
        {
            _handle = handle;
        }

        public void Dispose()
        {
            if (_handle != IntPtr.Zero)
            {
                AnimatedModel_Free(_handle);
            }
            _handle = IntPtr.Zero;
        }

        public void Render(TempleDll templeDll)
        {
            if (_handle != IntPtr.Zero)
            {
                AnimatedModel_Render(templeDll.Handle, _handle);
            }
        }

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void AnimatedModel_Render(IntPtr templeDll, IntPtr handle);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr AnimatedModel_FromFiles(IntPtr templeDll, string skmFilename, string skaFilename);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void AnimatedModel_Free(IntPtr handle);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void AnimatedModel_AdvanceTime(IntPtr handle, float time);

        public static AnimatedModel FromFiles(TempleDll templeDll, string skmPath, string skaPath)
        {
            return new AnimatedModel(AnimatedModel_FromFiles(templeDll.Handle, skmPath, skaPath));
        }

        public void AdvanceTime(float simulTime)
        {
            if (_handle != IntPtr.Zero)
            {
                AnimatedModel_AdvanceTime(_handle, simulTime);
            }
        }
    }
}