using System;
using System.Drawing;
using System.Runtime.InteropServices;
using ParticleModel;
using SharpDX.Direct3D9;

namespace ParticleEditor
{
    internal static class VideoRenderer
    {
        public static bool RenderVideo(Device device, string dataPath, PartSysSpec spec, string filename)
        {
            var specStr = spec.ToSpec();

            var activeSys = ParticleSystem.FromSpec(device, dataPath, specStr);

            return activeSys.RenderVideo(device, Color.FromArgb(255, 32, 32, 32), filename);
        }
    }
}