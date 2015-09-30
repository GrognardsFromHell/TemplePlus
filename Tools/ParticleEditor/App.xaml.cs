using System.Windows;

namespace ParticleEditor
{
    /// <summary>
    ///     Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private TempleDll _templeDll;

        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            _templeDll = new TempleDll(@"C:\TemplePlus\ToEE\");
        }

        protected override void OnExit(ExitEventArgs e)
        {
            _templeDll?.Dispose();
            base.OnExit(e);
        }
    }
}