using System;
using System.Drawing;
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

        private TimeSpan? _timeSinceLastSimul;

        private TempleDll _templeDll;

        private bool _lastVisible;

        public PreviewControl()
        {
            InitializeComponent();

            RenderOutput.Loaded += RenderOutput_Loaded;
            RenderOutput.SizeChanged += RenderOutput_SizeChanged;

            // Tricky... DataContext is actually inherited from our parent for bindings done there
            // So we just set the DataContext for this control's content instead.
            ((FrameworkElement) Content).DataContext = _model;

            var sizeTimer = new DispatcherTimer(DispatcherPriority.Render);
            sizeTimer.Tick += SizeTimer_Tick;
            sizeTimer.Interval = new TimeSpan(0, 0, 0, 0, 1000/30);
            sizeTimer.Start();
        }

        private void RenderOutput_Loaded(object sender, RoutedEventArgs e)
        {
            InitializeRendering();

            RenderOutput.MouseLeftButtonDown += RenderOutput_MouseLeftButtonDown;
            RenderOutput.MouseMove += RenderOutput_MouseMove;
            RenderOutput.MouseLeftButtonUp += RenderOutput_MouseLeftButtonUp;
            RenderOutput.MouseWheel += RenderOutput_MouseWheel;
        }

        private void RenderOutput_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            var dpiScale = 1.0; // default value for 96 dpi

            // determine DPI
            // (as of .NET 4.6.1, this returns the DPI of the primary monitor, if you have several different DPIs)
            var hwndTarget = PresentationSource.FromVisual(this).CompositionTarget as HwndTarget;
            if (hwndTarget != null)
            {
                dpiScale = hwndTarget.TransformToDevice.M11;
            }

            int surfWidth = (int)(RenderOutput.ActualWidth < 0 ? 0 : Math.Ceiling(RenderOutput.ActualWidth * dpiScale));
            int surfHeight = (int)(RenderOutput.ActualHeight < 0 ? 0 : Math.Ceiling(RenderOutput.ActualHeight * dpiScale));

            // Notify the D3D11Image of the pixel size desired for the DirectX rendering.
            // The D3DRendering component will determine the size of the new surface it is given, at that point.
            InteropImage.SetPixelSize(surfWidth, surfHeight);

            // Stop rendering if the D3DImage isn't visible - currently just if width or height is 0
            // TODO: more optimizations possible (scrolled off screen, etc...)
            bool isVisible = (surfWidth != 0 && surfHeight != 0);
            if (_lastVisible != isVisible)
            {
                _lastVisible = isVisible;
                if (_lastVisible)
                {
                    CompositionTarget.Rendering += CompositionTarget_Rendering;
                }
                else
                {
                    CompositionTarget.Rendering -= CompositionTarget_Rendering;
                }
            }
        }

        private void CompositionTarget_Rendering(object sender, EventArgs e)
        {
            var args = (RenderingEventArgs)e;

            // It's possible for Rendering to call back twice in the same frame 
            // so only render when we haven't already rendered in this frame.
            if (_lastRender != args.RenderingTime) {
                InteropImage.RequestRender();
                _lastRender = args.RenderingTime;
            }
        }

        private void InitializeRendering()
        {
            // Get the topmost window
            var parentWindow = Window.GetWindow(this);

            InteropImage.WindowOwner = new WindowInteropHelper(parentWindow).Handle;
            InteropImage.OnRender = DoRender;
            InteropImage.RequestRender();

            _templeDll = new TempleDll(DataPath);
        }

        private void DoRender(IntPtr surface, bool isNewSurface)
        {
            if (DisableRendering)
                return;

            var w = _outputWidth;
            var h = _outputHeight;
            if (w <= 0 || h <= 0)
            {
                return;
            }

            if (_activeSystem == null && ActiveSystem != null)
            {
                _activeSystem = ParticleSystem.FromSpec(ActiveSystem.ToSpec());
            }

            
            if (_timeSinceLastSimul.HasValue)
            {
                var simulTime = (float)(_lastRender.TotalSeconds - _timeSinceLastSimul.Value.TotalSeconds);
                if (simulTime > 1 / 60.0f)
                {
                    _previewModel?.AdvanceTime(simulTime);

                    if (!_model.Paused)
                    {
                        if (_activeSystem != null)
                        {
                            _activeSystem.Simulate(simulTime);
                            UpdateParticleStatistics();
                        }
                    }
                    _timeSinceLastSimul = _lastRender;
                }
            }
            else
            {
                _timeSinceLastSimul = _lastRender;
            }

            _templeDll.SetRenderTarget(surface);

            _templeDll.Scale = _model.Scale;
            _templeDll.CenterOn(0, 0, 0);

            _previewModel?.Render(TempleDll.Instance);
            _activeSystem?.Render();

            _templeDll.Flush();
        }

        public bool DisableRendering
        {
            get { return (bool) GetValue(DisableRenderingProperty); }
            set { SetValue(DisableRenderingProperty, value); }
        }

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
            if (TempleDll.Instance == null)
            {
                return;
            }

            _previewModel = AnimatedModel.FromFiles(
                TempleDll.Instance,
                @"art\meshes\PCs\PC_Human_Male\PC_Human_Male.SKM",
                @"art\meshes\PCs\PC_Human_Male\PC_Human_Male.SKA"
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
                    _activeSystem.ScreenPosition = pos;
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