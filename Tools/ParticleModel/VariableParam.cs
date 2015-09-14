using System;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows;

namespace ParticleModel
{
    public enum ParamId
    {
        EmitAccelX = 0x0,
        EmitAccelY = 0x1,
        EmitAccelZ = 0x2,
        EmitVelVariationX = 0x3,
        EmitVelVariationY = 0x4,
        EmitVelVariationZ = 0x5,
        EmitPosVariationX = 0x6,
        EmitPosVariationY = 0x7,
        EmitPosVariationZ = 0x8,
        EmitYaw = 0x9,
        EmitPitch = 0xA,
        EmitRoll = 0xB,
        EmitScaleX = 0xC,
        EmitScaleY = 0xD,
        EmitScaleZ = 0xE,
        EmitOffsetX = 0xF,
        EmitOffsetY = 0x10,
        EmitOffsetZ = 0x11,
        EmitInitVelX = 0x12,
        EmitInitVelY = 0x13,
        EmitInitVelZ = 0x14,
        EmitInitAlpha = 0x15,
        EmitInitRed = 0x16,
        EmitInitGreen = 0x17,
        EmitInitBlue = 0x18,
        PartAccelX = 0x19,
        PartAccelY = 0x1A,
        PartAccelZ = 0x1B,
        PartVelVariationX = 0x1C,
        PartVelVariationY = 0x1D,
        PartVelVariationZ = 0x1E,
        PartPosVariationX = 0x1F,
        PartPosVariationY = 0x20,
        PartPosVariationZ = 0x21,
        PartScaleX = 0x22,
        PartScaleY = 0x23,
        PartScaleZ = 0x24,
        PartYaw = 0x25,
        PartPitch = 0x26,
        PartRoll = 0x27,
        PartAlpha = 0x28,
        PartRed = 0x29,
        PartGreen = 0x2A,
        PartBlue = 0x2B,
        PartAttractorBlend = 0x2C
    }

    public abstract class VariableParam : DependencyObject
    {
        public static readonly DependencyProperty IdProperty = DependencyProperty.Register(
            "Id", typeof (ParamId), typeof (VariableParam), new PropertyMetadata(default(ParamId)));

        public ParamId Id
        {
            get { return (ParamId) GetValue(IdProperty); }
            set { SetValue(IdProperty, value); }
        }

        public abstract string ToSpec();

        internal static float ParseFloat(string val)
        {
            // To emulate what the sscanf function used by vanilla ToEE would do,
            // We ignore certain variants of junk behind the floating point number
            if (val.Contains('('))
            {
                val = val.Substring(0, val.IndexOf('('));
            }

            return float.Parse(val, CultureInfo.InvariantCulture);
        }

        public static VariableParam Parse(string value)
        {
            if (string.IsNullOrWhiteSpace(value))
            {
                return null;
            }

            if (value == "#radius")
            {
                return new ObjectRadiusParam();
            }

            if (value.Contains(','))
            {
                // Parse keyframes (ugh...)
                var frameStrs = value.Split(',');
                var frames = frameStrs.Select(Keyframe.FromSpec);
                return new KeyframeParam {Frames = new ObservableCollection<Keyframe>(frames)};
            }

            if (value.Contains('?'))
            {
                // Another attempt at cleaning up after Troika.
                // Some values are incorrectly formatted here
                if (value.Contains(' '))
                {
                    // TODO: Warn here
                    value = value.Substring(0, value.IndexOf(' '));
                }

                var parts = value.Split('?');
                return new RandomParam {From = ParseFloat(parts[0]), To = ParseFloat(parts[1])};
            }

            return new ConstantParam {Value = ParseFloat(value)};
        }
    }

    public class ConstantParam : VariableParam
    {
        public static readonly DependencyProperty ValueProperty = DependencyProperty.Register(
            "Value", typeof (float), typeof (ConstantParam), new PropertyMetadata(default(float)));

        public float Value
        {
            get { return (float) GetValue(ValueProperty); }
            set { SetValue(ValueProperty, value); }
        }

        public override string ToSpec()
        {
            return string.Format(CultureInfo.InvariantCulture, "{0}", Value);
        }
    }

    public class RandomParam : VariableParam
    {
        public static readonly DependencyProperty FromProperty = DependencyProperty.Register(
            "From", typeof (float), typeof (RandomParam), new PropertyMetadata(default(float)));

        public static readonly DependencyProperty ToProperty = DependencyProperty.Register(
            "To", typeof (float), typeof (RandomParam), new PropertyMetadata(default(float)));

        public float From
        {
            get { return (float) GetValue(FromProperty); }
            set { SetValue(FromProperty, value); }
        }

        public float To
        {
            get { return (float) GetValue(ToProperty); }
            set { SetValue(ToProperty, value); }
        }

        public override string ToSpec()
        {
            return string.Format(CultureInfo.InvariantCulture, "{0}?{1}", From, To);
        }
    }

    public class Keyframe : DependencyObject
    {
        public static readonly DependencyProperty ValueProperty = DependencyProperty.Register(
            "Value", typeof (float), typeof (Keyframe), new PropertyMetadata(default(float)));

        public static readonly DependencyProperty PositionPercentageProperty = DependencyProperty.Register(
            "PositionPercentage", typeof (int?), typeof (Keyframe), new PropertyMetadata(default(int?)));

        public static readonly DependencyProperty PositionLifespanProperty = DependencyProperty.Register(
            "PositionLifespan", typeof (float?), typeof (Keyframe), new PropertyMetadata(default(float?)));

        public float Value
        {
            get { return (float) GetValue(ValueProperty); }
            set { SetValue(ValueProperty, value); }
        }

        public int? PositionPercentage
        {
            get { return (int?) GetValue(PositionPercentageProperty); }
            set { SetValue(PositionPercentageProperty, value); }
        }

        public float? PositionLifespan
        {
            get { return (float?) GetValue(PositionLifespanProperty); }
            set { SetValue(PositionLifespanProperty, value); }
        }

        public string ToSpec()
        {
            if (PositionPercentage.HasValue)
            {
                return string.Format(CultureInfo.InvariantCulture, "{0}({1}%)", Value, PositionPercentage.Value);
            }
            if (PositionLifespan.HasValue)
            {
                return string.Format(CultureInfo.InvariantCulture, "{0}({1})", Value, PositionLifespan.Value);
            }
            return string.Format(CultureInfo.InvariantCulture, "{0}", Value);
        }

        /// <summary>
        /// Vanilla ToEE particle systems contain keyframe values that are actually invalid: 100?200.
        /// Apparently, designers tried to do random values inside of keyframes, but this actually
        /// was never implemented and as such we skip it.
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        private static float ParseValue(string value)
        {
            if (string.IsNullOrWhiteSpace(value))
            {
                // TODO: Warn here...
                return 0;
            }

            var pos = value.IndexOf('?');
            if (pos != -1)
            {
                return VariableParam.ParseFloat(value.Substring(0, pos));
            }
            return VariableParam.ParseFloat(value);
        }

        public static Keyframe FromSpec(string arg)
        {
            // No ( -> constant
            if (!arg.Contains('('))
            {
                return new Keyframe
                {
                    Value = ParseValue(arg)
                };
            }

            var regexp = new Regex(@"^(.+)\((.+)\)$");
            var match = regexp.Match(arg);
            if (!match.Success)
            {
                throw new ArgumentException("Invalid keyframe: " + arg);
            }

            var result = new Keyframe();
            if (match.Groups[2].Value.EndsWith("%"))
            {
                // Parse everything up to the percentage sign as the percentage of the emitter lifespan
                var val = match.Groups[2].Value;
                result.PositionPercentage = int.Parse(val.Substring(0, val.Length - 1));
            }
            else
            {
                result.PositionLifespan = VariableParam.ParseFloat(match.Groups[2].Value);
            }
            result.Value = ParseValue(match.Groups[1].Value);
            return result;
        }
    }

    public class KeyframeParam : VariableParam
    {
        public static readonly DependencyProperty FramesProperty = DependencyProperty.Register(
            "Frames", typeof (ObservableCollection<Keyframe>), typeof (KeyframeParam),
            new PropertyMetadata(default(ObservableCollection<Keyframe>)));

        public ObservableCollection<Keyframe> Frames
        {
            get { return (ObservableCollection<Keyframe>) GetValue(FramesProperty); }
            set { SetValue(FramesProperty, value); }
        }

        public override string ToSpec()
        {
            return string.Join(",", Frames.Select(f => f.ToSpec()));
        }
    }

    public class ObjectRadiusParam : VariableParam
    {
        public override string ToSpec()
        {
            return "#radius";
        }
    }
}