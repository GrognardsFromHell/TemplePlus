using System;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;
using ParticleModel;
using SharpDX;
using SharpDX.Direct3D9;

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

        private bool _dragging;

        private TimeSpan _lastRender;

        private int _outputHeight;

        private int _outputWidth;

        private AnimatedModel _previewModel;
        private Surface _renderTargetDepth;
        private int _renderTargetHeight;

        private Surface _renderTargetSurface;
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
                    ReloadAnimatedModel();
                }
            }
        }

        public PartSysSpec ActiveSystem
        {
            get { return (PartSysSpec) GetValue(ActiveSystemProperty); }
            set { SetValue(ActiveSystemProperty, value); }
        }

        private void ReloadAnimatedModel()
        {
            _previewModel = AnimatedModel.FromFiles(
                TempleDll.Instance,
                Path.Combine(_dataPath, @"art\meshes\PCs\PC_Human_Male\PC_Human_Male.SKM"),
                Path.Combine(_dataPath, @"art\meshes\PCs\PC_Human_Male\PC_Human_Male.SKA")
                );
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
            Device.Clear(ClearFlags.Target | ClearFlags.ZBuffer, new ColorBGRA(0, 0, 0, 255), 1, 0);

            if (_timeSinceLastSimul.HasValue)
            {
                var simulTime = (float)(renderTime.TotalSeconds - _timeSinceLastSimul.Value.TotalSeconds);
                if (simulTime > 1 / 60.0f)
                {
                    _previewModel.AdvanceTime(simulTime);

                    if (!_model.Paused)
                    {
                        if (_activeSystem != null)
                        {
                            _activeSystem.Simulate(simulTime);
                            UpdateParticleStatistics();
                        }
                    }
                    _timeSinceLastSimul = renderTime;
                }
            }
            else
            {
                _timeSinceLastSimul = renderTime;
            }

            _previewModel?.Render(Device, w, h, _model.Scale);
            _activeSystem?.Render(Device, w, h, 0, 0, _model.Scale);

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

        private void RenderOutput_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _dragging = true;
        }

        private void RenderOutput_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed && _dragging)
            {
                var pos = e.GetPosition(RenderOutput);
                pos.X -= RenderOutput.ActualWidth*0.5;
                pos.Y -= RenderOutput.ActualHeight*0.5;

                // If the preview is scaled, we have to reverse the scaling here to get back
                // to a 1:1 mapping of pixels
                pos.X /= _model.Scale;
                pos.Y /= _model.Scale;

                if (ActiveSystem != null)
                {
                    _activeSystem.ScreenPosition = new Vector2((float) pos.X, (float) pos.Y);
                }
            }
            else
            {
                _dragging = false;
            }
        }

        private void RenderOutput_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            _dragging = false;
        }

        private void RenderOutput_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            // Effective delta only seems to come i multiples of 120. So 5 clicks mean double size
            _model.Scale += e.Delta/(5*120.0f);
        }
    }
}