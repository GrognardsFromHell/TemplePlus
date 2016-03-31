using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using IniParser.Model;

namespace TemplePlusConfig
{
    public class IniViewModel : DependencyObject
    {
        public static readonly DependencyProperty InstallationPathProperty = DependencyProperty.Register(
            "InstallationPath", typeof (string), typeof (IniViewModel), new PropertyMetadata(default(string)));

        public static readonly DependencyProperty RenderWidthProperty = DependencyProperty.Register(
            "RenderWidth", typeof (int), typeof (IniViewModel), new PropertyMetadata(default(int)));

        public static readonly DependencyProperty RenderHeightProperty = DependencyProperty.Register(
            "RenderHeight", typeof (int), typeof (IniViewModel), new PropertyMetadata(default(int)));

        public static readonly DependencyProperty WindowedModeProperty = DependencyProperty.Register(
            "WindowedMode", typeof (bool), typeof (IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty DisableAutomaticUpdatesProperty = DependencyProperty.Register(
            "DisableAutomaticUpdates", typeof (bool), typeof (IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty SoftShadowsProperty = DependencyProperty.Register(
            "SoftShadows", typeof (bool), typeof (IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty HpOnLevelUpProperty = DependencyProperty.Register(
            "HpOnLevelUp", typeof (HpOnLevelUpType), typeof (IniViewModel),
            new PropertyMetadata(default(HpOnLevelUpType)));

        public static readonly DependencyProperty PointBuyPointsProperty = DependencyProperty.Register(
            "PointBuyPoints", typeof (int), typeof (IniViewModel), new PropertyMetadata(default(int)));

        public IEnumerable<HpOnLevelUpType> HpOnLevelUpTypes => Enum.GetValues(typeof (HpOnLevelUpType))
            .Cast<HpOnLevelUpType>();

        public IniViewModel()
        {
            RenderWidth = (int)SystemParameters.PrimaryScreenWidth;
            RenderHeight = (int)SystemParameters.PrimaryScreenHeight;
            PointBuyPoints = 25;
        }

        public string InstallationPath
        {
            get { return (string) GetValue(InstallationPathProperty); }
            set { SetValue(InstallationPathProperty, value); }
        }

        public int RenderWidth
        {
            get { return (int) GetValue(RenderWidthProperty); }
            set { SetValue(RenderWidthProperty, value); }
        }

        public int RenderHeight
        {
            get { return (int) GetValue(RenderHeightProperty); }
            set { SetValue(RenderHeightProperty, value); }
        }

        public bool WindowedMode
        {
            get { return (bool) GetValue(WindowedModeProperty); }
            set { SetValue(WindowedModeProperty, value); }
        }

        public bool DisableAutomaticUpdates
        {
            get { return (bool) GetValue(DisableAutomaticUpdatesProperty); }
            set { SetValue(DisableAutomaticUpdatesProperty, value); }
        }

        public bool SoftShadows
        {
            get { return (bool) GetValue(SoftShadowsProperty); }
            set { SetValue(SoftShadowsProperty, value); }
        }

        public HpOnLevelUpType HpOnLevelUp
        {
            get { return (HpOnLevelUpType) GetValue(HpOnLevelUpProperty); }
            set { SetValue(HpOnLevelUpProperty, value); }
        }

        public int PointBuyPoints
        {
            get { return (int) GetValue(PointBuyPointsProperty); }
            set { SetValue(PointBuyPointsProperty, value); }
        }

        public void LoadFromIni(IniData iniData)
        {
            var tpData = iniData["TemplePlus"];
            InstallationPath = tpData["toeeDir"];
            DisableAutomaticUpdates = tpData["autoUpdate"] != "true";
            if (tpData["hpOnLevelup"] != null)
            {
                switch (tpData["hpOnLevelup"].ToLowerInvariant())
                {
                    case "max":
                        HpOnLevelUp = HpOnLevelUpType.Max;
                        break;
                    case "average":
                        HpOnLevelUp = HpOnLevelUpType.Max;
                        break;
                    default:
                        HpOnLevelUp = HpOnLevelUpType.Normal;
                        break;
                }
            }

            int points;
            if (int.TryParse(tpData["pointBuyPoints"], out points))
            {
                PointBuyPoints = points;
            }
            else
            {
                PointBuyPoints = 25;
            }

            int renderWidth, renderHeight;
            if (int.TryParse(tpData["renderWidth"], out renderWidth)
                && int.TryParse(tpData["renderHeight"], out renderHeight))
            {
                RenderWidth = renderWidth;
                RenderHeight = renderHeight;
            }
            else
            {
                renderWidth = (int) SystemParameters.PrimaryScreenWidth;
                renderHeight = (int) SystemParameters.PrimaryScreenHeight;
            }

            SoftShadows = tpData["softShadows"] == "true";
            WindowedMode = tpData["windowed"] == "true";
        }

        public void SaveToIni(IniData iniData)
        {
            var tpData = iniData["TemplePlus"];
            if (tpData == null)
            {
                iniData.Sections.Add(new SectionData("TemplePlus"));
                tpData = iniData["TemplePlus"];
            }
            
            tpData["toeeDir"] = InstallationPath;
            tpData["autoUpdate"] = DisableAutomaticUpdates ? "false" : "true";
            switch (HpOnLevelUp)
            {
                case HpOnLevelUpType.Max:
                    tpData["hpOnLevelup"] = "max";
                    break;
                case HpOnLevelUpType.Average:
                    tpData["hpOnLevelup"] = "average";
                    break;
                default:
                    tpData["hpOnLevelup"] = "normal";
                    break;
            }
            tpData["pointBuyPoints"] = PointBuyPoints.ToString();
            tpData["renderWidth"] = RenderWidth.ToString();
            tpData["renderHeight"] = RenderHeight.ToString();
            tpData["windowed"] = WindowedMode ? "true" : "false";
            tpData["windowWidth"] = RenderWidth.ToString();
            tpData["windowHeight"] = RenderHeight.ToString();
            tpData["softShadows"] = SoftShadows ? "true" : "false";
        }
    }

    public enum HpOnLevelUpType
    {
        Normal,
        Max,
        Average
    }
}