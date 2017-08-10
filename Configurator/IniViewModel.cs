using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using IniParser.Model;
using Microsoft.Win32;

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

        public static readonly DependencyProperty AntiAliasingProperty = DependencyProperty.Register(
            "AntiAliasing", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty SoftShadowsProperty = DependencyProperty.Register(
            "SoftShadows", typeof (bool), typeof (IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty WindowedLockCursorProperty = DependencyProperty.Register(
           "WindowedLockCursor", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty DungeonMasterProperty = DependencyProperty.Register(
           "DungeonMaster", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        
        public static readonly DependencyProperty HpOnLevelUpProperty = DependencyProperty.Register(
            "HpOnLevelUp", typeof (HpOnLevelUpType), typeof (IniViewModel),
            new PropertyMetadata(default(HpOnLevelUpType)));

        public static readonly DependencyProperty FogOfWarProperty = DependencyProperty.Register(
            "FogOfWar", typeof(FogOfWarType), typeof(IniViewModel),
            new PropertyMetadata(default(FogOfWarType)));

        public static readonly DependencyProperty PointBuyPointsProperty = DependencyProperty.Register(
            "PointBuyPoints", typeof (int), typeof (IniViewModel), new PropertyMetadata(default(int)));

        public static readonly DependencyProperty MaxLevelProperty = DependencyProperty.Register(
            "MaxLevel", typeof(int), typeof(IniViewModel), new PropertyMetadata(default(int)));

        public static readonly DependencyProperty AllowXpOverflowProperty = DependencyProperty.Register(
           "AllowXpOverflow", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty SlowerLevellingProperty = DependencyProperty.Register(
           "SlowerLevelling", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty TolerantTownsfolkProperty = DependencyProperty.Register(
          "TolerantTownsfolk", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty TransparentNpcStatsProperty = DependencyProperty.Register(
          "TransparentNpcStats", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty FastSneakingProperty = DependencyProperty.Register(
          "FastSneaking", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty DisableDoorRelockingProperty = DependencyProperty.Register(
          "DisableDoorRelocking", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty AlertAiThroughDoorsProperty = DependencyProperty.Register(
          "AlertAiThroughDoors", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        


        public static readonly DependencyProperty NewClassesProperty = DependencyProperty.Register(
          "NewClasses", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty NonCoreProperty = DependencyProperty.Register(
          "NonCore", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty LaxRulesProperty = DependencyProperty.Register(
          "LaxRules", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty DisableAlignmentRestrictionsProperty = DependencyProperty.Register(
          "DisableAlignmentRestrictions", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty DisableCraftingSpellReqsProperty = DependencyProperty.Register(
          "DisableCraftingSpellReqs", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty DisableMulticlassXpPenaltyProperty = DependencyProperty.Register(
          "DisableMulticlassXpPenalty", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty ShowTargetingCirclesInFogOfWarProperty = DependencyProperty.Register(
          "ShowTargetingCirclesInFogOfWar", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        

        public IEnumerable<HpOnLevelUpType> HpOnLevelUpTypes => Enum.GetValues(typeof (HpOnLevelUpType))
            .Cast<HpOnLevelUpType>();
        public IEnumerable<FogOfWarType> FogOfWarTypes => Enum.GetValues(typeof(FogOfWarType))
           .Cast<FogOfWarType>();

        public IniViewModel()
        {
            var screenSize = ScreenResolution.ScreenSize;
            RenderWidth = (int)screenSize.Width;
            RenderHeight = (int)screenSize.Height;
            PointBuyPoints = 25;
            MaxLevel = 10;
            SlowerLevelling = false;
            AllowXpOverflow = false;
        }

        public string InstallationPath
        {
            get
            {
                return (string) GetValue(InstallationPathProperty);
            }
            set
            {
                SetValue(InstallationPathProperty, value);
            }
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

        public bool AntiAliasing
        {
            get { return (bool)GetValue(AntiAliasingProperty); }
            set { SetValue(AntiAliasingProperty, value); }
        }

        public bool SoftShadows
        {
            get { return (bool) GetValue(SoftShadowsProperty); }
            set { SetValue(SoftShadowsProperty, value); }
        }

        public bool WindowedLockCursor
        {
            get { return (bool)GetValue(WindowedLockCursorProperty); }
            set { SetValue(WindowedLockCursorProperty, value); }
        }

        public bool DungeonMaster
        {
            get { return (bool)GetValue(DungeonMasterProperty); }
            set { SetValue(DungeonMasterProperty, value); }
        }

        public HpOnLevelUpType HpOnLevelUp
        {
            get { return (HpOnLevelUpType) GetValue(HpOnLevelUpProperty); }
            set { SetValue(HpOnLevelUpProperty, value); }
        }

        public FogOfWarType FogOfWar
        {
            get { return (FogOfWarType)GetValue(FogOfWarProperty); }
            set { SetValue(FogOfWarProperty, value); }
        }

        public int PointBuyPoints
        {
            get { return (int) GetValue(PointBuyPointsProperty); }
            set { SetValue(PointBuyPointsProperty, value); }
        }

        public int MaxLevel
        {
            get { return (int)GetValue(MaxLevelProperty); }
            set { SetValue(MaxLevelProperty, value); }
        }

        public bool AllowXpOverflow
        {
            get { return (bool)GetValue(AllowXpOverflowProperty); }
            set { SetValue(AllowXpOverflowProperty, value); }
        }

        public bool SlowerLevelling
        {
            get { return (bool)GetValue(SlowerLevellingProperty); }
            set { SetValue(SlowerLevellingProperty, value); }
        }

        public bool TolerantTownsfolk
        {
            get { return (bool)GetValue(TolerantTownsfolkProperty); }
            set { SetValue(TolerantTownsfolkProperty, value); }
        }

        public bool TransparentNpcStats
        {
            get { return (bool)GetValue(TransparentNpcStatsProperty); }
            set { SetValue(TransparentNpcStatsProperty, value); }
        }

        public bool FastSneaking
        {
            get { return (bool)GetValue(FastSneakingProperty); }
            set { SetValue(FastSneakingProperty, value); }
        }
        public bool DisableDoorRelocking
        {
            get { return (bool)GetValue(DisableDoorRelockingProperty); }
            set { SetValue(DisableDoorRelockingProperty, value); }
        }
        public bool AlertAiThroughDoors
        {
            get { return (bool)GetValue(AlertAiThroughDoorsProperty); }
            set { SetValue(AlertAiThroughDoorsProperty, value); }
        }
        

        public bool NewClasses
        {
            get { return (bool)GetValue(NewClassesProperty); }
            set { SetValue(NewClassesProperty, value); }
        }

        public bool NonCore
        {
            get { return (bool)GetValue(NonCoreProperty); }
            set { SetValue(NonCoreProperty, value); }
        }

        public bool LaxRules
        {
            get { return (bool)GetValue(LaxRulesProperty); }
            set { SetValue(LaxRulesProperty, value); }
        }
        public bool DisableAlignmentRestrictions
        {
            get { return (bool)GetValue(DisableAlignmentRestrictionsProperty); }
            set { SetValue(DisableAlignmentRestrictionsProperty, value); }
        }
        public bool DisableCraftingSpellReqs
        {
            get { return (bool)GetValue(DisableCraftingSpellReqsProperty); }
            set { SetValue(DisableCraftingSpellReqsProperty, value); }
        }
        public bool DisableMulticlassXpPenalty
        {
            get { return (bool)GetValue(DisableMulticlassXpPenaltyProperty); }
            set { SetValue(DisableMulticlassXpPenaltyProperty, value); }
        }
        public bool ShowTargetingCirclesInFogOfWar
        {
            get { return (bool)GetValue(ShowTargetingCirclesInFogOfWarProperty); }
            set { SetValue(ShowTargetingCirclesInFogOfWarProperty, value); }
        }

        /// <summary>
        /// Tries to find an installation directory based on common locations and the Windows registry.
        /// </summary>
        public void AutoDetectInstallation()
        {
            var gogKey = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\\GOG.com\\Games\\1207658889");
            var gogPath = gogKey?.GetValue("PATH", null) as string;

            if (gogPath != null)
            {
                var gogStatus = InstallationDirValidator.Validate(gogPath);
                if (gogStatus.Valid)
                {
                    InstallationPath = gogPath;
                }
            }

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
                        HpOnLevelUp = HpOnLevelUpType.Average;
                        break;
                    default:
                        HpOnLevelUp = HpOnLevelUpType.Normal;
                        break;
                }
            }

            if (tpData["fogOfWar"] != null)
            {
                switch (tpData["fogOfWar"].ToLowerInvariant())
                {
                    case "always":
                        FogOfWar = FogOfWarType.Always;
                        break;
                    case "normal":
                        FogOfWar = FogOfWarType.Normal;
                        break;
                    case "unfogged":
                        FogOfWar = FogOfWarType.Unfogged;
                        break;
                    default:
                        FogOfWar = FogOfWarType.Normal;
                        break;
                }
            }

            int points;
            if (int.TryParse(tpData["pointBuyPoints"], out points))
            {
                PointBuyPoints = points;
            }

            int renderWidth, renderHeight;
            if (int.TryParse(tpData["renderWidth"], out renderWidth)
                && int.TryParse(tpData["renderHeight"], out renderHeight))
            {
                RenderWidth = renderWidth;
                RenderHeight = renderHeight;
            }

            SoftShadows = tpData["softShadows"] == "true";
            AntiAliasing = tpData["antialiasing"] == "true";
            WindowedMode = tpData["windowed"] == "true";
            WindowedLockCursor = tpData["windowedLockCursor"] == "true";
            DungeonMaster = tpData["dungeonMaster"] == "true";

            int maxLevel;
            if (int.TryParse(tpData["maxLevel"], out maxLevel))
            {
                MaxLevel = maxLevel;
            }

            bool allowXpOverflow;
            if (bool.TryParse(tpData["allowXpOverflow"], out allowXpOverflow))
            {
                AllowXpOverflow = allowXpOverflow;
            }

            bool slowerLevelling;
            if (bool.TryParse(tpData["slowerLevelling"], out slowerLevelling))
            {
                SlowerLevelling = slowerLevelling;
            }

            bool tolerantTownsfolk;
            if (bool.TryParse(tpData["tolerantNpcs"], out tolerantTownsfolk))
            {
                TolerantTownsfolk = tolerantTownsfolk;
            }

            bool showExactHPforNPCs, showNpcStats;
            if (bool.TryParse(tpData["showExactHPforNPCs"], out showExactHPforNPCs)
                && bool.TryParse(tpData["showNpcStats"], out showNpcStats))
            {
                TransparentNpcStats = showNpcStats && showExactHPforNPCs;
            }

            bool fastSneaking;
            if (bool.TryParse(tpData["fastSneakAnim"], out fastSneaking)){
                FastSneaking = fastSneaking;
            }
            bool disableDoorRelocking;
            if (bool.TryParse(tpData["disableDoorRelocking"], out disableDoorRelocking))
            {
                DisableDoorRelocking = disableDoorRelocking;
            }
            bool alertAiThroughDoors;
            if (bool.TryParse(tpData["alertAiThroughDoors"], out alertAiThroughDoors))
            {
                AlertAiThroughDoors = alertAiThroughDoors;
            }

            

            bool newClasses;
            if (bool.TryParse(tpData["newClasses"], out newClasses))
            {
                NewClasses = newClasses;
            }

            bool nonCore;
            if (bool.TryParse(tpData["nonCoreMaterials"], out nonCore))
            {
                NonCore = nonCore;
            }


            // Lax Rules
            bool laxRules;
            if (bool.TryParse(tpData["laxRules"], out laxRules))
            {
                LaxRules = laxRules;
            }
            bool disableAlignmentRestrictions;
            if (bool.TryParse(tpData["disableAlignmentRestrictions"], out disableAlignmentRestrictions))
            {
                DisableAlignmentRestrictions = disableAlignmentRestrictions;
            }
            bool disableCraftingSpellReqs;
            if (bool.TryParse(tpData["disableCraftingSpellReqs"], out disableCraftingSpellReqs))
            {
                DisableCraftingSpellReqs = disableCraftingSpellReqs;
            }
            bool disableMulticlassXpPenalty;
            if (bool.TryParse(tpData["disableMulticlassXpPenalty"], out disableMulticlassXpPenalty))
            {
                DisableMulticlassXpPenalty = disableMulticlassXpPenalty;
            }
            bool showTargetingCirclesInFogOfWar;
            if (bool.TryParse(tpData["showTargetingCirclesInFogOfWar"], out showTargetingCirclesInFogOfWar))
            {
                ShowTargetingCirclesInFogOfWar = showTargetingCirclesInFogOfWar;
            }

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
            switch (FogOfWar)
            {
                case FogOfWarType.Unfogged:
                    tpData["fogOfWar"] = "unfogged";
                    break;
                case FogOfWarType.Always:
                    tpData["fogOfWar"] = "always";
                    break;
                default:
                    tpData["fogOfWar"] = "normal";
                    break;
            }
            tpData["laxRules"] = LaxRules ? "true" : "false";

            tpData["disableAlignmentRestrictions"] = DisableAlignmentRestrictions ? "true" : "false";
            tpData["disableCraftingSpellReqs"] = DisableCraftingSpellReqs ? "true" : "false";
            tpData["disableMulticlassXpPenalty"] = DisableMulticlassXpPenalty ? "true" : "false";
            tpData["showTargetingCirclesInFogOfWar"] = ShowTargetingCirclesInFogOfWar ? "true" : "false";


            tpData["pointBuyPoints"] = PointBuyPoints.ToString();
            tpData["renderWidth"] = RenderWidth.ToString();
            tpData["renderHeight"] = RenderHeight.ToString();
            tpData["windowed"] = WindowedMode ? "true" : "false";
            tpData["windowWidth"] = RenderWidth.ToString();
            tpData["windowHeight"] = RenderHeight.ToString();
            tpData["antialiasing"] = AntiAliasing? "true" : "false";
            tpData["softShadows"] = SoftShadows ? "true" : "false";
            tpData["windowedLockCursor"] = WindowedLockCursor ? "true" : "false";
            tpData["dungeonMaster"] = DungeonMaster ? "true" : "false";
            tpData["maxLevel"] = MaxLevel.ToString();
            tpData["allowXpOverflow"] = AllowXpOverflow ? "true" : "false";
            tpData["slowerLevelling"] = SlowerLevelling ? "true" : "false";
            tpData["newClasses"] = NewClasses? "true" : "false";
            tpData["nonCoreMaterials"] = NonCore ? "true" : "false";
            tpData["tolerantNpcs"] = TolerantTownsfolk? "true" : "false";
            tpData["showExactHPforNPCs"] = TransparentNpcStats? "true" : "false";
            tpData["showNpcStats"] = TransparentNpcStats ? "true" : "false";
            tpData["fastSneakAnim"] = FastSneaking ? "true" : "false";
            tpData["disableDoorRelocking"] = DisableDoorRelocking? "true" : "false";
            tpData["alertAiThroughDoors"] = AlertAiThroughDoors ? "true" : "false";
            
        }
    }

    public enum HpOnLevelUpType
    {
        Normal,
        Max,
        Average
    }

    public enum FogOfWarType
    {
        Normal,
        Unfogged,
        Always // ensures maps are fogged even despite UNFOGGED flag in the module defs
    }
}