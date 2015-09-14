using System;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;
using ParticleModel;
using SharpDX;
using SharpDX.Direct3D9;
using Color = SharpDX.Color;

namespace ParticleEditor
{
    /// <summary>
    ///     Interaction logic for PreviewControl.xaml
    /// </summary>
    public partial class PreviewControl : UserControl
    {
        public static readonly DependencyProperty ActiveSystemProperty = DependencyProperty.Register(
            "ActiveSystem", typeof (PartSysSpec), typeof (PreviewControl),
            new PropertyMetadata(default(PartSysSpec), OnActiveSystemChange));

        public static readonly DependencyProperty DisableRenderingProperty = DependencyProperty.Register(
            "DisableRendering", typeof (bool), typeof (PreviewControl), new PropertyMetadata(default(bool)));

        private readonly PreviewControlModel _model = new PreviewControlModel();

        private ParticleSystem _activeSystem;

        private string _dataPath;

        private TimeSpan _lastRender;

        private int _outputHeight;

        private int _outputWidth;

        private Surface _renderTargetSurface;
        private Surface _renderTargetDepth;
        private int _renderTargetHeight;
        private int _renderTargetWidth;

        private TimeSpan? _timeSinceLastSimul;

        public PreviewControl()
        {
            InitializeComponent();

            // Tricky... DataContext is actually inherited from our parent for bindings done there
            // So we just set the DataContext for this control's content instead.
            ((FrameworkElement) Content).DataContext = _model;

            var sizeTimer = new DispatcherTimer(DispatcherPriority.Render);
            sizeTimer.Tick += SizeTimer_Tick;
            sizeTimer.Interval = new TimeSpan(0, 0, 0, 0, 1000/30);
            sizeTimer.Start();

            CompositionTarget.Rendering += RenderFrame;
        }

        public bool DisableRendering
        {
            get { return (bool) GetValue(DisableRenderingProperty); }
            set { SetValue(DisableRenderingProperty, value); }
        }

        public Device Device { get; private set; }

        public string DataPath
        {
            get { return _dataPath; }
            set
            {
                _dataPath = value;
                if (string.IsNullOrWhiteSpace(_dataPath))
                {
                    DataPathErrorLabel.Visibility = Visibility.Visible;
                }
                else
                {
                    DataPathErrorLabel.Visibility = Visibility.Hidden;
                }
            }
        }

        public PartSysSpec ActiveSystem
        {
            get { return (PartSysSpec) GetValue(ActiveSystemProperty); }
            set
            {
                SetValue(ActiveSystemProperty, value);
            }
        }

        public event EventHandler ConfigureDataPath;

        private static void OnActiveSystemChange(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            // Dispose of the active system and recreate it
            var control = d as PreviewControl;
            if (control != null)
            {
                control._activeSystem?.Dispose();
                control._activeSystem = null;
                var spec = (PartSysSpec) e.NewValue;
                if (spec != null)
                {
                    spec.AnyPropertyChanged += control.PartSysPropertyChanged;
                }
                spec = (PartSysSpec) e.OldValue;
                if (spec != null)
                {
                    spec.AnyPropertyChanged -= control.PartSysPropertyChanged;
                }
            }
        }

        private void PartSysPropertyChanged(object sender, EventArgs e)
        {
            _activeSystem?.Dispose();
            _activeSystem = null;
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
            if (DisableRendering)
                return;

            var args = (RenderingEventArgs) e;

            var w = _outputWidth;
            var h = _outputHeight;
            if (w <= 0 || h <= 0)
            {
                return;
            }

            if (Device == null)
            {
                var presentParams = new PresentParameters(w, h)
                {
                    Windowed = true,
                    SwapEffect = SwapEffect.Discard,
                    DeviceWindowHandle = GetDesktopWindow(),
                    PresentationInterval = PresentInterval.Default
                };
                Device = new DeviceEx(new Direct3DEx(),
                    0,
                    DeviceType.Hardware,
                    IntPtr.Zero,
                    CreateFlags.HardwareVertexProcessing | CreateFlags.Multithreaded | CreateFlags.FpuPreserve,
                    presentParams);
            }

            if (_activeSystem == null && ActiveSystem != null)
            {
                _activeSystem = ParticleSystem.FromSpec(Device, DataPath, ActiveSystem.ToSpec());
            }

            if (D3Dimg.IsFrontBufferAvailable && _lastRender != args.RenderingTime)
            {
                if (_renderTargetSurface == null || _renderTargetWidth != w || _renderTargetHeight != h)
                {
                    _renderTargetWidth = w;
                    _renderTargetHeight = h;

                    _renderTargetSurface?.Dispose();
                    _renderTargetDepth?.Dispose();

                    _renderTargetSurface = Surface.CreateRenderTarget(Device,
                        _renderTargetWidth,
                        _renderTargetHeight,
                        Format.X8R8G8B8,
                        MultisampleType.None,
                        0,
                        false);
                    _renderTargetDepth = Surface.CreateDepthStencil(Device,
                        _renderTargetWidth,
                        _renderTargetHeight,
                        Format.D16,
                        MultisampleType.None, 
                        0,
                        true);
                }

                D3Dimg.Lock();

                Device.SetRenderTarget(0, _renderTargetSurface);
                Device.DepthStencilSurface = _renderTargetDepth;
                RenderParticleSystem(w, h, args.RenderingTime);

                D3Dimg.SetBackBuffer(D3DResourceType.IDirect3DSurface9, _renderTargetSurface.NativePointer);
                D3Dimg.AddDirtyRect(new Int32Rect(0, 0, _renderTargetWidth, _renderTargetHeight));
                D3Dimg.Unlock();

                _lastRender = args.RenderingTime;
            }
        }

        private void RenderParticleSystem(int w, int h, TimeSpan renderTime)
        {
            Device.BeginScene();
            Device.Clear(ClearFlags.Target|ClearFlags.ZBuffer, new ColorBGRA(32, 32, 32, 255),  1, 0);

            if (_activeSystem != null)
            {
                if (_timeSinceLastSimul.HasValue)
                {
                    var simulTime = (float) (renderTime.TotalSeconds - _timeSinceLastSimul.Value.TotalSeconds);
                    if (simulTime > 1/60.0f)
                    {
                        if (!_model.Paused)
                        {
                            _activeSystem.Simulate(simulTime);
                            UpdateParticleStatistics();
                        }
                        _timeSinceLastSimul = renderTime;
                    }
                }
                else
                {
                    _timeSinceLastSimul = renderTime;
                }


                _activeSystem.Render(Device, w, h, 0, 0, 1f);
            }

            Device.EndScene();
            Device.Present();
        }

        private void UpdateParticleStatistics()
        {
            _model.ActiveParticles = _activeSystem.Emitters.Sum(e => e.ActiveParticles);
        }

        private void EmitConfigureDataPath(object sender, RoutedEventArgs e)
        {
            OnConfigureDataPath();
        }

        protected virtual void OnConfigureDataPath()
        {
            ConfigureDataPath?.Invoke(this, EventArgs.Empty);
        }
    }
}