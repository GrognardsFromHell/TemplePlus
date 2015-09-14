using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Text;
using System.Windows;

namespace ParticleModel
{
    public enum ParticleType
    {
        Point,
        Sprite,
        Disc,
        Billboard,
        Model
    }

    public enum EmitterSpace
    {
        World,
        ObjectPos,
        ObjectYpr,
        NodePos,
        NodeYpr,
        Bones
    }

    public enum CoordSys
    {
        Polar,
        Cartesian
    }

    public enum ParticleSpace
    {
        SameAsEmitter,
        World,
        EmitterYpr
    }

    public enum BlendMode
    {
        Add,
        Blend,
        Multiply,
        Subtract
    }

    public static class ParticleTypeHelper
    {
        public static string ToString(ParticleType type)
        {
            switch (type)
            {
                case ParticleType.Point:
                    return "Point";
                case ParticleType.Sprite:
                    return "Sprite";
                case ParticleType.Disc:
                    return "Disc";
                case ParticleType.Billboard:
                    return "Billboard";
                case ParticleType.Model:
                    return "Model";
                default:
                    throw new ArgumentOutOfRangeException(nameof(type), type, null);
            }
        }

        public static ParticleType Parse(string value)
        {
            switch (value.ToLowerInvariant())
            {
                case "point":
                    return ParticleType.Point;
                case "sprite":
                    return ParticleType.Sprite;
                case "disc":
                    return ParticleType.Disc;
                case "billboard":
                    return ParticleType.Billboard;
                case "model":
                    return ParticleType.Model;
                default:
                    return ParticleType.Point;
            }
        }

    }

    public static class BlendModeHelper
    {
        public static string ToString(BlendMode blendMode)
        {
            switch (blendMode)
            {
                case BlendMode.Add:
                    return "Add";
                case BlendMode.Blend:
                    return "Blend";
                case BlendMode.Multiply:
                    return "Multiply";
                case BlendMode.Subtract:
                    return "Subtract";
                default:
                    throw new ArgumentOutOfRangeException(nameof(blendMode), blendMode, null);
            }
        }

        public static BlendMode ParseString(string value)
        {
            switch (value.ToLowerInvariant())
            {
                case "blend":
                    return BlendMode.Blend;
                case "multiply":
                    return BlendMode.Multiply;
                case "subtract":
                    return BlendMode.Subtract;
                default:
                    return BlendMode.Add;
            }
        }
    }

    public static class CoordSysHelper
    {
        public static string ToString(CoordSys coordSys)
        {
            switch (coordSys)
            {
                case CoordSys.Polar:
                    return "Polar";
                case CoordSys.Cartesian:
                    return "Cartesian";
                default:
                    throw new ArgumentOutOfRangeException(nameof(coordSys), coordSys, null);
            }
        }

        public static CoordSys ParseString(string value)
        {
            switch (value.ToLowerInvariant())
            {
                case "polar":
                    return CoordSys.Polar;
                default:
                    return CoordSys.Cartesian;
            }
        }
    }

    public static class EmitterSpaceHelper
    {
        public static string ToString(EmitterSpace space)
        {
            switch (space)
            {
                case EmitterSpace.World:
                    return "World";
                case EmitterSpace.ObjectPos:
                    return "Object Pos";
                case EmitterSpace.ObjectYpr:
                    return "Object YPR";
                case EmitterSpace.NodePos:
                    return "Node Pos";
                case EmitterSpace.NodeYpr:
                    return "Node YPR";
                case EmitterSpace.Bones:
                    return "Bones";
                default:
                    throw new ArgumentOutOfRangeException(nameof(space), space, null);
            }
        }

        public static EmitterSpace ParseString(string value)
        {
            switch (value.ToLowerInvariant())
            {
                case "object pos":
                    return EmitterSpace.ObjectPos;
                case "object ypr":
                    return EmitterSpace.ObjectYpr;
                case "node pos":
                    return EmitterSpace.NodePos;
                case "node ypr":
                    return EmitterSpace.NodeYpr;
                case "bones":
                    return EmitterSpace.Bones;
                default:
                    return EmitterSpace.World;
            }
        }
    }

    public static class ParticleSpaceHelper
    {
        public static string ToString(ParticleSpace space)
        {
            switch (space)
            {
                case ParticleSpace.World:
                    return "World";
                case ParticleSpace.EmitterYpr:
                    return "Emitter YPR";
                case ParticleSpace.SameAsEmitter:
                    return "Same as Emitter";
                default:
                    throw new ArgumentOutOfRangeException(nameof(space), space, null);
            }
        }

        public static ParticleSpace ParseString(string value)
        {
            switch (value.ToLowerInvariant())
            {
                case "emitter ypr":
                    return ParticleSpace.EmitterYpr;
                case "same as emitter":
                    return ParticleSpace.SameAsEmitter;
                default:
                    return ParticleSpace.World;
            }
        }
    }

    public class EmitterSpec : DependencyObject
    {
        private const int ColPartsysName = 0;
        private const int ColEmitterName = 1;
        private const int ColDelay = 2;
        private const int ColEmitType = 3;// Unused
        private const int ColLifespan = 4;
        private const int ColParticleRate = 5;
        private const int ColBoundingRadius = 6; // Unused
        private const int ColEmitterSpace = 7;
        private const int ColEmitterNodeName = 8;
        private const int ColEmitterCoordSys = 9;
        private const int ColEmitterOffsetCoordSys = 10;
        private const int ColParticleType = 11;
        private const int ColParticleSpace = 12;
        private const int ColParticlePosCoordSys = 13;
        private const int ColParticleVelocityCoordSys = 14;
        private const int ColMaterial = 15;
        private const int ColPartLifespan = 16;
        private const int ColBlendMode = 17;
        private const int ColBounce = 18; // Unused
        private const int ColAnimSpeed = 19; // Unused
        private const int ColModel = 20;
        // animation

        private const int ColFirstParam = 22;

        private const int ColBbLeft = 67;
        private const int ColBbTop = 68;
        private const int ColBbRight = 69;
        private const int ColBbBottom = 70;
        private const int ColMinParticles = 71;
        private const int ColCount = 72;

        public static readonly DependencyProperty NameProperty = DependencyProperty.Register("Name", typeof (string), typeof (EmitterSpec), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty ParticleTypeProperty = DependencyProperty.Register("ParticleType", typeof (ParticleType), typeof (EmitterSpec), new PropertyMetadata(ParticleType.Point));

        public static readonly DependencyProperty PermanentProperty = DependencyProperty.Register("Permanent", typeof (bool), typeof (EmitterSpec), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty LifespanSecsProperty = DependencyProperty.Register("LifespanSecs", typeof (float), typeof (EmitterSpec), new PropertyMetadata(default(float)));

        public static readonly DependencyProperty DelaySecsProperty = DependencyProperty.Register("DelaySecs", typeof (float), typeof (EmitterSpec), new PropertyMetadata(default(float)));

        public static readonly DependencyProperty ParticlesPerSecProperty = DependencyProperty.Register("ParticlesPerSec", typeof (float), typeof (EmitterSpec), new PropertyMetadata(default(float)));

        public static readonly DependencyProperty ParticleLifespanProperty = DependencyProperty.Register("ParticleLifespan", typeof (float), typeof (EmitterSpec), new PropertyMetadata(1.0f));

        public static readonly DependencyProperty ParticlesPermanentProperty = DependencyProperty.Register("ParticlesPermanent", typeof (bool), typeof (EmitterSpec), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty MaterialProperty = DependencyProperty.Register("Material", typeof (string), typeof (EmitterSpec), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty SpaceProperty = DependencyProperty.Register("Space", typeof (EmitterSpace), typeof (EmitterSpec), new PropertyMetadata(EmitterSpace.World));

        public static readonly DependencyProperty NodeNameProperty = DependencyProperty.Register("NodeName", typeof (string), typeof (EmitterSpec), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty CoordSysProperty = DependencyProperty.Register("CoordSys", typeof (CoordSys), typeof (EmitterSpec), new PropertyMetadata(CoordSys.Cartesian));

        public static readonly DependencyProperty OffsetCoordSysProperty = DependencyProperty.Register("OffsetCoordSys", typeof (CoordSys), typeof (EmitterSpec), new PropertyMetadata(CoordSys.Cartesian));

        public static readonly DependencyProperty ParticleSpaceProperty = DependencyProperty.Register("ParticleSpace", typeof (ParticleSpace), typeof (EmitterSpec), new PropertyMetadata(ParticleSpace.World));

        public static readonly DependencyProperty ParticlePosCoordSysProperty = DependencyProperty.Register("ParticlePosCoordSys", typeof (CoordSys), typeof (EmitterSpec), new PropertyMetadata(CoordSys.Cartesian));

        public static readonly DependencyProperty ParticleVelocityCoordSysProperty = DependencyProperty.Register("ParticleVelocityCoordSys", typeof (CoordSys), typeof (EmitterSpec), new PropertyMetadata(CoordSys.Cartesian));

        public static readonly DependencyProperty BlendModeProperty = DependencyProperty.Register("BlendMode", typeof (BlendMode), typeof (EmitterSpec), new PropertyMetadata(BlendMode.Add));

        public static readonly DependencyProperty BoundingBoxLeftProperty = DependencyProperty.Register("BoundingBoxLeft", typeof (float), typeof (EmitterSpec), new PropertyMetadata(-399.0f));

        public static readonly DependencyProperty BoundingBoxTopProperty = DependencyProperty.Register("BoundingBoxTop", typeof (float), typeof (EmitterSpec), new PropertyMetadata(-299.0f));

        public static readonly DependencyProperty BoundingBoxRightProperty = DependencyProperty.Register("BoundingBoxRight", typeof (float), typeof (EmitterSpec), new PropertyMetadata(399.0f));

        public static readonly DependencyProperty BoundingBoxBottomProperty = DependencyProperty.Register("BoundingBoxBottom", typeof (float), typeof (EmitterSpec), new PropertyMetadata(299.0f));

        public static readonly DependencyProperty MinActiveParticlesProperty = DependencyProperty.Register("MinActiveParticles", typeof (int?), typeof (EmitterSpec), new PropertyMetadata(default(int?)));

        public static readonly DependencyProperty ModelProperty = DependencyProperty.Register("Model", typeof (string), typeof (EmitterSpec), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty ParamsProperty = DependencyProperty.Register("Params", typeof (ObservableCollection<VariableParam>), typeof (EmitterSpec), new PropertyMetadata(default(ObservableCollection<VariableParam>)));

        public EmitterSpec()
        {
            Params = new ObservableCollection<VariableParam>();
        }

        public ObservableCollection<VariableParam> Params
        {
            get { return (ObservableCollection<VariableParam>) GetValue(ParamsProperty); }
            set { SetValue(ParamsProperty, value); }
        }

        public float BoundingBoxLeft
        {
            get { return (float) GetValue(BoundingBoxLeftProperty); }
            set { SetValue(BoundingBoxLeftProperty, value); }
        }

        public float BoundingBoxTop
        {
            get { return (float) GetValue(BoundingBoxTopProperty); }
            set { SetValue(BoundingBoxTopProperty, value); }
        }

        public float BoundingBoxRight
        {
            get { return (float) GetValue(BoundingBoxRightProperty); }
            set { SetValue(BoundingBoxRightProperty, value); }
        }

        public float BoundingBoxBottom
        {
            get { return (float) GetValue(BoundingBoxBottomProperty); }
            set { SetValue(BoundingBoxBottomProperty, value); }
        }

        public int? MinActiveParticles
        {
            get { return (int?) GetValue(MinActiveParticlesProperty); }
            set { SetValue(MinActiveParticlesProperty, value); }
        }

        public BlendMode BlendMode
        {
            get { return (BlendMode) GetValue(BlendModeProperty); }
            set { SetValue(BlendModeProperty, value); }
        }

        public string Model
        {
            get { return (string) GetValue(ModelProperty); }
            set { SetValue(ModelProperty, value); }
        }

        public ParticleSpace ParticleSpace
        {
            get { return (ParticleSpace) GetValue(ParticleSpaceProperty); }
            set { SetValue(ParticleSpaceProperty, value); }
        }

        public CoordSys ParticlePosCoordSys
        {
            get { return (CoordSys) GetValue(ParticlePosCoordSysProperty); }
            set { SetValue(ParticlePosCoordSysProperty, value); }
        }

        public CoordSys ParticleVelocityCoordSys
        {
            get { return (CoordSys) GetValue(ParticleVelocityCoordSysProperty); }
            set { SetValue(ParticleVelocityCoordSysProperty, value); }
        }

        public string NodeName
        {
            get { return (string) GetValue(NodeNameProperty); }
            set { SetValue(NodeNameProperty, value); }
        }

        public string Name
        {
            get { return (string) GetValue(NameProperty); }
            set { SetValue(NameProperty, value); }
        }

        public ParticleType ParticleType
        {
            get { return (ParticleType) GetValue(ParticleTypeProperty); }
            set { SetValue(ParticleTypeProperty, value); }
        }

        public bool Permanent
        {
            get { return (bool) GetValue(PermanentProperty); }
            set { SetValue(PermanentProperty, value); }
        }

        public float LifespanSecs
        {
            get { return (float) GetValue(LifespanSecsProperty); }
            set { SetValue(LifespanSecsProperty, value); }
        }

        public float DelaySecs
        {
            get { return (float) GetValue(DelaySecsProperty); }
            set { SetValue(DelaySecsProperty, value); }
        }

        public float ParticlesPerSec
        {
            get { return (float) GetValue(ParticlesPerSecProperty); }
            set { SetValue(ParticlesPerSecProperty, value); }
        }

        public float ParticleLifespan
        {
            get { return (float) GetValue(ParticleLifespanProperty); }
            set { SetValue(ParticleLifespanProperty, value); }
        }

        public bool ParticlesPermanent
        {
            get { return (bool) GetValue(ParticlesPermanentProperty); }
            set { SetValue(ParticlesPermanentProperty, value); }
        }

        public string Material
        {
            get { return (string) GetValue(MaterialProperty); }
            set { SetValue(MaterialProperty, value); }
        }

        public EmitterSpace Space
        {
            get { return (EmitterSpace) GetValue(SpaceProperty); }
            set { SetValue(SpaceProperty, value); }
        }

        public CoordSys CoordSys
        {
            get { return (CoordSys) GetValue(CoordSysProperty); }
            set { SetValue(CoordSysProperty, value); }
        }

        public CoordSys OffsetCoordSys
        {
            get { return (CoordSys) GetValue(OffsetCoordSysProperty); }
            set { SetValue(OffsetCoordSysProperty, value); }
        }

        public event EventHandler EmitterChanged;

        public static EmitterSpec Parse(string line)
        {
            var result = new EmitterSpec();

            var cols = line.Split('\t');

            // Sanitize line parts (vertical tabs, spaces)
            for (var i = 0; i < cols.Length; ++i)
            {
                cols[i] = cols[i].Trim(' ', '\v');
            }

            result.Name = cols[ColEmitterName];
            result.ParticlesPerSec = float.Parse(cols[ColParticleRate], CultureInfo.InvariantCulture);

            result.ParticleType = ParticleTypeHelper.Parse(cols[ColParticleType]);

            if (cols[ColLifespan].Equals("perm", StringComparison.InvariantCultureIgnoreCase))
            {
                result.Permanent = true;
            }
            else
            {
                result.LifespanSecs = float.Parse(cols[ColLifespan]);
            }

            if (cols[ColPartLifespan].Equals("perm", StringComparison.InvariantCultureIgnoreCase))
            {
                result.ParticlesPermanent = true;
            }
            else
            {
                float floatVal;
                if (float.TryParse(cols[ColPartLifespan], out floatVal))
                {
                    result.ParticleLifespan = floatVal;
                }
            }

            result.Material = cols[ColMaterial];

            result.Space = EmitterSpaceHelper.ParseString(cols[ColEmitterSpace]);
            result.NodeName = cols[ColEmitterNodeName];

            result.CoordSys = CoordSysHelper.ParseString(cols[ColEmitterCoordSys]);
            result.OffsetCoordSys = CoordSysHelper.ParseString(cols[ColEmitterOffsetCoordSys]);

            result.ParticleSpace = ParticleSpaceHelper.ParseString(cols[ColParticleSpace]);
            result.ParticlePosCoordSys = CoordSysHelper.ParseString(cols[ColParticlePosCoordSys]);
            result.ParticleVelocityCoordSys = CoordSysHelper.ParseString(cols[ColParticleVelocityCoordSys]);

            result.BlendMode = BlendModeHelper.ParseString(cols[ColBlendMode]);
            result.Model = cols[ColModel];

            float bbVal;
            if (float.TryParse(cols[ColBbLeft], out bbVal))
            {
                result.BoundingBoxLeft = bbVal;
            }
            if (float.TryParse(cols[ColBbTop], out bbVal))
            {
                result.BoundingBoxTop = bbVal;
            }
            if (float.TryParse(cols[ColBbRight], out bbVal))
            {
                result.BoundingBoxRight = bbVal;
            }
            if (float.TryParse(cols[ColBbBottom], out bbVal))
            {
                result.BoundingBoxBottom = bbVal;
            }

            int minActiveParts;
            if (int.TryParse(cols[ColMinParticles], out minActiveParts))
            {
                result.MinActiveParticles = minActiveParts;
            }

            // Parse the variable parameters
            foreach (ParamId paramId in Enum.GetValues(typeof (ParamId)))
            {
                var col = ColFirstParam + (int) paramId;
                var param = VariableParam.Parse(cols[col]);
                if (param != null)
                {
                    param.Id = paramId;
                    result.Params.Add(param);
                }
            }

            return result;
        }

        public string ToSpec(string partSysName)
        {
            var cols = new object[ColCount];
            cols[ColPartsysName] = partSysName;
            cols[ColEmitterName] = Name;

            // Lifetime + Particle Count
            cols[ColDelay] = DelaySecs;

            cols[ColBounce] = 0; // Unused but this is used in the vanilla files

            if (Permanent)
            {
                cols[ColLifespan] = "perm";
            }
            else
            {
                cols[ColLifespan] = LifespanSecs;
            }

            if (ParticlesPermanent)
            {
                cols[ColPartLifespan] = "perm";
            }
            else
            {
                cols[ColPartLifespan] = ParticleLifespan;
            }

            cols[ColParticleRate] = ParticlesPerSec;

            // Emit type is not actually used and seems to be Point
            cols[ColEmitType] = "Point";
            cols[ColParticleType] = ParticleType;

            cols[ColMaterial] = Material;

            cols[ColEmitterSpace] = EmitterSpaceHelper.ToString(Space);

            cols[ColEmitterNodeName] = NodeName;

            cols[ColEmitterCoordSys] = CoordSysHelper.ToString(CoordSys);
            cols[ColEmitterOffsetCoordSys] = CoordSysHelper.ToString(OffsetCoordSys);
            cols[ColParticleSpace] = ParticleSpaceHelper.ToString(ParticleSpace);

            cols[ColParticlePosCoordSys] = CoordSysHelper.ToString(ParticlePosCoordSys);
            cols[ColParticleVelocityCoordSys] = CoordSysHelper.ToString(ParticleVelocityCoordSys);

            cols[ColBlendMode] = BlendModeHelper.ToString(BlendMode);
            cols[ColModel] = Model;

            cols[ColBbLeft] = BoundingBoxLeft;
            cols[ColBbTop] = BoundingBoxTop;
            cols[ColBbRight] = BoundingBoxRight;
            cols[ColBbBottom] = BoundingBoxBottom;

            if (MinActiveParticles.HasValue)
            {
                cols[ColMinParticles] = MinActiveParticles;
            }

            foreach (var param in Params)
            {
                if (param != null)
                {
                    cols[ColFirstParam + (int) param.Id] = param.ToSpec();
                }
            }

            return FormatColumns(cols);
        }

        private static string FormatColumns(IEnumerable<object> cols)
        {
            // Convert the columns to strings
            var result = new StringBuilder(512);

            foreach (var col in cols)
            {
                if (result.Length > 0)
                {
                    result.Append('\t');
                }

                // Do the proper formatting of the column type
                result.AppendFormat(CultureInfo.InvariantCulture, "{0}", col);
            }

            return result.ToString();
        }

        protected virtual void OnEmitterChanged()
        {
            EmitterChanged?.Invoke(this, EventArgs.Empty);
        }

        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);
            OnEmitterChanged();
        }
    }
}