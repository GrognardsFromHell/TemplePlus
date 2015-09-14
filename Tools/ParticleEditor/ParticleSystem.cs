using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using SharpDX.Direct3D9;

namespace ParticleEditor
{
    public class ParticleSystem : IDisposable
    {
        private IntPtr _handle;

        public ParticleSystem(IntPtr handle)
        {
            _handle = handle;
        }

        public List<ParticleSystemEmitter> Emitters
        {
            get
            {
                var count = EmitterCount;
                var result = new List<ParticleSystemEmitter>(count);
                for (var i = 0; i < count; ++i)
                {
                    var emitterHandle = ParticleSystem_GetEmitter(_handle, i);
                    result.Add(new ParticleSystemEmitter(emitterHandle));
                }
                return result;
            }
        }

        public int EmitterCount => ParticleSystem_GetEmitterCount(_handle);

        public bool IsDead => ParticleSystem_IsDead(_handle);

        public void Dispose()
        {
            if (_handle == IntPtr.Zero) return;
            ParticleSystem_Free(_handle);
            _handle = IntPtr.Zero;
        }

        public static ParticleSystem FromSpec(Device device, string dataPath, string spec)
        {
            var handle = ParticleSystem_FromSpec(device.NativePointer, dataPath, spec);
            if (handle == IntPtr.Zero)
            {
                throw new InvalidOperationException("Unable to parse particle system spec: " + spec);
            }

            return new ParticleSystem(handle);
        }

        public void Simulate(float elapsedSecs)
        {
            ParticleSystem_Simulate(_handle, elapsedSecs);
        }

        public void Render(Device device, float w, float h, float xTrans, float yTrans, float scale)
        {
            ParticleSystem_Render(device.NativePointer, _handle, w, h, xTrans, yTrans, scale);
        }

        public bool RenderVideo(Device device, Color background, string fileName)
        {
            return ParticleSystem_RenderVideo(device.NativePointer, _handle, background.ToArgb(), fileName, 60);
        }

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern IntPtr ParticleSystem_FromSpec(IntPtr deviceHandle, string dataPath, string spec);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        private static extern int ParticleSystem_GetEmitterCount(IntPtr handle);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern bool ParticleSystem_RenderVideo(IntPtr deviceHandle, IntPtr handle, int backgroundColor, string outputFile, int fps);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        private static extern IntPtr ParticleSystem_GetEmitter(IntPtr handle, int idx);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ParticleSystem_Simulate(IntPtr handle, float timeInSecs);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool ParticleSystem_IsDead(IntPtr handle);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void ParticleSystem_Render(IntPtr deviceHandle, IntPtr handle, float w, float h, float xTrans, float yTrans, float scale);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
        private static extern void ParticleSystem_Free(IntPtr particleSys);

    }
}