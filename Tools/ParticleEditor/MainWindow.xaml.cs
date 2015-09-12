using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;
using Microsoft.Win32;
using SharpDX;
using SharpDX.Direct3D9;
using Color = System.Drawing.Color;

namespace ParticleEditor
{
    /// <summary>
    ///     Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private const long SimulTimeStepTicks = TimeSpan.TicksPerSecond/30; // 30 steps per second

        private readonly ParticleSystem _activeSystem;

        private Device _device;

        private TimeSpan _lastRender;
        private int _outputHeight;

        private int _outputWidth;

        private Texture _renderTarget;
        private int _renderTargetHeight;

        private int _renderTargetWidth;

        private TimeSpan? _timeSinceLastSimul;

        public MainWindow()
        {
            InitializeComponent();

            var sizeTimer = new DispatcherTimer(DispatcherPriority.Render);
            sizeTimer.Tick += SizeTimer_Tick;
            sizeTimer.Interval = new TimeSpan(0, 0, 0, 0, 1000/30);
            sizeTimer.Start();

            CompositionTarget.Rendering += RenderFrame;

            // Init a particle system (exemplary)
            _activeSystem = ParticleSystem.FromSpec("",
                "sp-Bane	little sparkles			0	600						Point		Polar		flare-big	60	Add	0				0	0	0	0	0	0	0	30	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0?360	0	0,600	4	0	0	0,360	0	0	255,0	255,0,0	255,0,0	255,128,128						");
            CountLabel.Content = _activeSystem.Emitters.Sum(e => e.ActiveParticles);
        }

        [DllImport("user32.dll", SetLastError = false)]
        private static extern IntPtr GetDesktopWindow();

        private void SizeTimer_Tick(object sender, EventArgs e)
        {
            _outputWidth = (int) RenderOutput.ActualWidth;
            _outputHeight = (int) RenderOutput.ActualHeight;
        }

        private void RenderFrame(object sender, EventArgs e)
        {
            var args = (RenderingEventArgs) e;

            var w = _outputWidth;
            var h = _outputHeight;
            if (w <= 0 || h <= 0)
            {
                return;
            }

            if (_device == null)
            {
                var presentParams = new PresentParameters(w, h)
                {
                    Windowed = true,
                    SwapEffect = SwapEffect.Discard,
                    DeviceWindowHandle = GetDesktopWindow(),
                    PresentationInterval = PresentInterval.Default
                };
                _device = new DeviceEx(new Direct3DEx(),
                    0,
                    DeviceType.Hardware,
                    IntPtr.Zero,
                    CreateFlags.HardwareVertexProcessing | CreateFlags.Multithreaded | CreateFlags.FpuPreserve,
                    presentParams);
            }

            if (D3Dimg.IsFrontBufferAvailable && _lastRender != args.RenderingTime)
            {
                if (_renderTarget == null || _renderTargetWidth != w || _renderTargetHeight != h)
                {
                    _renderTargetWidth = w;
                    _renderTargetHeight = h;

                    _renderTarget?.Dispose();

                    _renderTarget = new Texture(_device,
                        _renderTargetWidth,
                        _renderTargetHeight,
                        1,
                        Usage.RenderTarget,
                        Format.A8R8G8B8,
                        Pool.Default);

                    using (var surface = _renderTarget.GetSurfaceLevel(0))
                    {
                        _device.SetRenderTarget(0, surface);
                    }
                }

                D3Dimg.Lock();
                using (var surface = _renderTarget.GetSurfaceLevel(0))
                {
                    D3Dimg.SetBackBuffer(D3DResourceType.IDirect3DSurface9, surface.NativePointer);
                }

                RenderParticleSystem(w, h, args.RenderingTime);
                D3Dimg.AddDirtyRect(new Int32Rect(0, 0, w, h));
                D3Dimg.Unlock();

                _lastRender = args.RenderingTime;
            }
        }

        private void RenderParticleSystem(int w, int h, TimeSpan renderTime)
        {
            _device.BeginScene();
            _device.Clear(ClearFlags.Target, new ColorBGRA(32, 32, 32, 255), 0, 0);

            if (_activeSystem != null)
            {
                if (_timeSinceLastSimul.HasValue)
                {
                    var simulTime = (float) (renderTime.TotalSeconds - _timeSinceLastSimul.Value.TotalSeconds);
                    if (simulTime > 1/60.0f)
                    {
                        if (!PauseButton.IsChecked.Value)
                        {
                            _activeSystem.Simulate(simulTime);
                            CountLabel.Content = _activeSystem.Emitters.Sum(e => e.ActiveParticles);
                        }
                        _timeSinceLastSimul = renderTime;
                    }
                }
                else
                {
                    _timeSinceLastSimul = renderTime;
                }


                _activeSystem.Render(_device, w, h, 0, 0, 1f);
            }

            _device.EndScene();
            _device.Present();
        }

        private void SaveVideo_OnClick(object sender, RoutedEventArgs e)
        {
            if (_activeSystem == null)
                return;

            var ofd = new SaveFileDialog();
            ofd.AddExtension = true;
            ofd.DefaultExt = "mp4";
            ofd.Filter = "MP4 Video|*.mp4|All Files|*.*";
            var result = ofd.ShowDialog(this);
            if (result.Value)
            {
                _activeSystem.RenderVideo(_device, Color.DarkGray, ofd.FileName);
            }
        }
    }
}