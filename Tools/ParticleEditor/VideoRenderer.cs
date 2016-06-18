using System;
using System.Drawing;
using ParticleModel;

namespace ParticleEditor
{
    internal static class VideoRenderer
    {
        public static bool RenderVideo(string dataPath, PartSysSpec spec, string filename)
        {
            var specStr = spec.ToSpec();

            var activeSys = ParticleSystem.FromSpec(specStr);

            return activeSys.RenderVideo(Color.FromArgb(255, 32, 32, 32), filename);
        }
    }
}