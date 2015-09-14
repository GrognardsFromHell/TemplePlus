using System.Windows;

namespace ParticleEditor
{
    public class PreviewControlModel : DependencyObject
    {
        public static readonly DependencyProperty PausedProperty = DependencyProperty.Register(
            "Paused", typeof (bool), typeof (PreviewControlModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty ActiveParticlesProperty = DependencyProperty.Register(
            "ActiveParticles", typeof (int), typeof (PreviewControlModel), new PropertyMetadata(default(int)));
        
        public int ActiveParticles
        {
            get { return (int) GetValue(ActiveParticlesProperty); }
            set { SetValue(ActiveParticlesProperty, value); }
        }

        public bool Paused
        {
            get { return (bool) GetValue(PausedProperty); }
            set { SetValue(PausedProperty, value); }
        }
    }
}