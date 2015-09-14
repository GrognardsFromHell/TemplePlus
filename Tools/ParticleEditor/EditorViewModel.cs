using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using ParticleModel;

namespace ParticleEditor
{
    public class EditorViewModel : DependencyObject
    {
        public static readonly DependencyProperty SystemsProperty = DependencyProperty.Register(
            "Systems", typeof (ObservableCollection<PartSysSpec>), typeof (EditorViewModel),
            new PropertyMetadata(default(ObservableCollection<PartSysSpec>)));

        public static readonly DependencyProperty OpenedFileNameProperty = DependencyProperty.Register(
            "OpenedFileName", typeof (string), typeof (EditorViewModel), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty SelectedSystemProperty = DependencyProperty.Register(
            "SelectedSystem", typeof (PartSysSpec), typeof (EditorViewModel), new PropertyMetadata(default(PartSysSpec)));

        public static readonly DependencyProperty FileOpenedProperty = DependencyProperty.Register("FileOpened",
            typeof (bool), typeof (EditorViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty SelectedEmitterProperty = DependencyProperty.Register(
            "SelectedEmitter", typeof (EmitterSpec), typeof (EditorViewModel),
            new PropertyMetadata(default(EmitterSpec)));

        public EmitterSpec SelectedEmitter
        {
            get { return (EmitterSpec) GetValue(SelectedEmitterProperty); }
            set { SetValue(SelectedEmitterProperty, value); }
        }

        public PartSysSpec SelectedSystem
        {
            get { return (PartSysSpec) GetValue(SelectedSystemProperty); }
            set { SetValue(SelectedSystemProperty, value); }
        }

        public string OpenedFileName
        {
            get { return (string) GetValue(OpenedFileNameProperty); }
            set { SetValue(OpenedFileNameProperty, value); }
        }

        public ObservableCollection<PartSysSpec> Systems
        {
            get { return (ObservableCollection<PartSysSpec>) GetValue(SystemsProperty); }
            set { SetValue(SystemsProperty, value); }
        }

        public List<BlendMode> BlendModes => Enum.GetValues(typeof (BlendMode)).Cast<BlendMode>().ToList();

        public List<CoordSys> CoordinateSystems => Enum.GetValues(typeof (CoordSys)).Cast<CoordSys>().ToList();
    }
}