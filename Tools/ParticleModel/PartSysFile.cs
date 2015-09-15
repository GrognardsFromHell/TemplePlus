using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;

namespace ParticleModel
{
    /// <summary>
    ///     A container for a lot of particle systems...
    /// </summary>
    public class PartSysFile : DependencyObject
    {
        public static readonly DependencyProperty SpecsProperty = DependencyProperty.Register(
            "Specs", typeof (ObservableCollection<PartSysSpec>), typeof (PartSysFile),
            new PropertyMetadata(default(ObservableCollection<PartSysSpec>)));

        public ObservableCollection<PartSysSpec> Specs
        {
            get { return (ObservableCollection<PartSysSpec>) GetValue(SpecsProperty); }
            set { SetValue(SpecsProperty, value); }
        }

        public void Load(string path)
        {
            var emittersBySystem = new Dictionary<string, PartSysSpec>();

            using (var file = new FileStream(path, FileMode.Open))
            {
                var reader = new StreamReader(file, Encoding.ASCII);

                // Slurp in all lines, do a preliminary sorting by particle system name
                string line;
                while ((line = reader.ReadLine()) != null)
                {
                    var firstTab = line.IndexOf('\t');
                    if (firstTab == -1)
                    {
                        continue; // Not a valid line
                    }
                    var systemName = line.Substring(0, firstTab);
                    if (systemName.Length == 0)
                    {
                        continue; // Also probably not a valid line
                    }

                    // Trim it by removing the vertical tab at the end
                    systemName = systemName.Replace("\v", "");

                    var key = systemName.ToLowerInvariant();
                    if (!emittersBySystem.ContainsKey(key))
                    {
                        emittersBySystem[key] = new PartSysSpec
                        {
                            Name = systemName
                        };
                    }
                    emittersBySystem[key].Emitters.Add(EmitterSpec.Parse(line));
                }
            }

            // Sort by name ascending
            var list = emittersBySystem.Values.ToList();
            list.Sort();

            Specs = new ObservableCollection<PartSysSpec>(list);
        }
    }

    public class PartSysSpec : DependencyObject, IComparable<PartSysSpec>
    {
        public static readonly DependencyProperty NameProperty = DependencyProperty.Register(
            "Name", typeof (string), typeof (PartSysSpec), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty EmittersProperty = DependencyProperty.Register(
            "Emitters", typeof (ObservableCollection<EmitterSpec>), typeof (PartSysSpec),
            new PropertyMetadata(default(ObservableCollection<EmitterSpec>)));

        public PartSysSpec()
        {
            Emitters = new ObservableCollection<EmitterSpec>();
            Emitters.CollectionChanged += EmittersChanged;
        }

        public ObservableCollection<EmitterSpec> Emitters
        {
            get { return (ObservableCollection<EmitterSpec>) GetValue(EmittersProperty); }
            set
            {
                if (Emitters != null) Emitters.CollectionChanged -= EmittersChanged;
                SetValue(EmittersProperty, value);
                if (value != null) value.CollectionChanged += EmittersChanged;
            }
        }

        public string Name
        {
            get { return (string) GetValue(NameProperty); }
            set { SetValue(NameProperty, value); }
        }

        public int CompareTo(PartSysSpec other)
        {
            return string.Compare(Name.ToLowerInvariant(),
                other.Name.ToLowerInvariant(),
                StringComparison.InvariantCultureIgnoreCase);
        }

        private void EmittersChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.OldItems != null)
            {
                foreach (var emitter in e.OldItems)
                {
                    ((EmitterSpec) emitter).EmitterChanged -= EmitterChanged;
                }
            }

            if (e.NewItems != null)
            {
                foreach (var emitter in e.NewItems)
                {
                    ((EmitterSpec) emitter).EmitterChanged += EmitterChanged;
                }
            }

            OnAnyPropertyChanged();
        }

        private void EmitterChanged(object sender, EventArgs e)
        {
            OnAnyPropertyChanged();
        }

        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);

            OnAnyPropertyChanged();
        }

        public string ToSpec()
        {
            var result = "";
            foreach (var emitterSpec in Emitters)
            {
                result += emitterSpec.ToSpec(Name) + "\n";
            }
            return result;
        }

        public event EventHandler AnyPropertyChanged;

        protected virtual void OnAnyPropertyChanged()
        {
            AnyPropertyChanged?.Invoke(this, EventArgs.Empty);
        }
    }
}