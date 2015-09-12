using System;
using System.Runtime.InteropServices;

namespace ParticleEditor
{
    /// <summary>
    /// A particle emitter. Objects of this class are alive and valid as long as their parent
    /// particle systems are valid.
    /// </summary>
    public class ParticleSystemEmitter
    {
        private readonly IntPtr _handle;

        public ParticleSystemEmitter(IntPtr handle)
        {
            _handle = handle;
        }

        public int MaxParticles
        {
            get { return ParticleSystemEmitter_GetMaxParticles(_handle); }
        }

        public int ActiveParticles
        {
            get { return ParticleSystemEmitter_GetActiveParticles(_handle); }
        }

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ParticleSystemEmitter_GetMaxParticles(IntPtr handle);

        [DllImport("ParticleEditorNative.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ParticleSystemEmitter_GetActiveParticles(IntPtr handle);
        
    }
}