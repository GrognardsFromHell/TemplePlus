using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using SharpDX;
using SharpDX.Direct3D9;
using Color = System.Drawing.Color;

namespace ParticleEditor
{
    public class ParticleSystem : IDisposable
    {

        public Vector2 ScreenPosition { get; set; }

        public ParticleSystem()
        {
            ScreenPosition = Vector2.Zero;
        }

        public List<ParticleSystemEmitter> Emitters
        {
            get
            {
                var count = EmitterCount;
                var result = new List<ParticleSystemEmitter>(count);
                for (var i = 0; i < count; ++i)
                {
                    var emitterHandle = ParticleSystem_GetEmitter(TempleDll.Instance.Handle, i);
                    result.Add(new ParticleSystemEmitter(emitterHandle));
                }
                return result;
            }
        }

        public int EmitterCount => ParticleSystem_GetEmitterCount(TempleDll.Instance.Handle);

        public bool IsDead => ParticleSystem_IsDead(TempleDll.Instance.Handle);

        public void Dispose()
        {
            ParticleSystem_Free(TempleDll.Instance.Handle);
        }

        public static ParticleSystem FromSpec(string spec)
        {
            if (!ParticleSystem_FromSpec(TempleDll.Instance.Handle, spec))
            {
                throw new InvalidOperationException("Unable to parse particle system spec: " + spec + ".\n"
                    + TempleDll.LastError);
            }

            return new ParticleSystem();
        }

        public void Simulate(float elapsedSecs)
        {
            ParticleSystem_Simulate(TempleDll.Instance.Handle, elapsedSecs);
        }

        public void Render(float w, float h, float xTrans, float yTrans, float scale)
        {
            ParticleSystem_SetObjPos(TempleDll.Instance.Handle, ScreenPosition.X, ScreenPosition.Y);
            ParticleSystem_SetPos(TempleDll.Instance.Handle, ScreenPosition.X, ScreenPosition.Y);

            ParticleSystem_Render(TempleDll.Instance.Handle, w, h, xTrans, yTrans, scale);
        }

        public bool RenderVideo(Color background, string fileName)
        {
            ParticleSystem_SetObjPos(TempleDll.Instance.Handle, ScreenPosition.X, ScreenPosition.Y);
            ParticleSystem_SetPos(TempleDll.Instance.Handle, ScreenPosition.X, ScreenPosition.Y);
            
            return ParticleSystem_RenderVideo(TempleDll.Instance.Handle, background.ToArgb(), fileName, 60);
        }

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern bool ParticleSystem_FromSpec(IntPtr dllHandle, string spec);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        private static extern int ParticleSystem_GetEmitterCount(IntPtr dllHandle);

        [DllImport("ParticleEditorNative.dll", EntryPoint = "ParticleSystem_RenderVideo", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool ParticleSystem_RenderVideo(IntPtr dllHandle, int backgroundColor, string outputFile, int fps);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        private static extern IntPtr ParticleSystem_GetEmitter(IntPtr dllHandle, int idx);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ParticleSystem_Simulate(IntPtr dllHandle, float timeInSecs);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ParticleSystem_SetPos(IntPtr dllHandle, float x, float y);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ParticleSystem_SetObjPos(IntPtr dllHandle, float x, float y);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ParticleSystem_Resize(IntPtr dllHandle, float x, float y);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ParticleSystem_SetScale(IntPtr dllHandle, float scale);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool ParticleSystem_IsDead(IntPtr dllHandle);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ParticleSystem_Render(IntPtr dllHandle, float w, float h, float xTrans, float yTrans, float scale);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        private static extern void ParticleSystem_Free(IntPtr dllHandle);

    }
}