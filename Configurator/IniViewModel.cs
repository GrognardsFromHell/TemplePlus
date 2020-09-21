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

        public static readonly DependencyProperty HpForNPCHdProperty = DependencyProperty.Register(
            "HpForNPCHd", typeof(HpForNPCHdType), typeof(IniViewModel),
            new PropertyMetadata(default(HpForNPCHdType)));

        public static readonly DependencyProperty FogOfWarProperty = DependencyProperty.Register(
            "FogOfWar", typeof(FogOfWarType), typeof(IniViewModel),
            new PropertyMetadata(default(FogOfWarType)));

        public static readonly DependencyProperty PointBuyPointsProperty = DependencyProperty.Register(
            "PointBuyPoints", typeof (int), typeof (IniViewModel), new PropertyMetadata(default(int)));

        public static readonly DependencyProperty MaxLevelProperty = DependencyProperty.Register(
            "MaxLevel", typeof(int), typeof(IniViewModel), new PropertyMetadata(default(int)));

        public static readonly DependencyProperty NumberOfPcsProperty = DependencyProperty.Register(
            "NumberOfPcs", typeof(NumberOfPcsType), typeof(IniViewModel), new PropertyMetadata(default(NumberOfPcsType)));

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
        public static readonly DependencyProperty WalkDistanceFtProperty = DependencyProperty.Register(
            "WalkDistanceFt", typeof(int), typeof(IniViewModel), new PropertyMetadata(default(int)));

        public static readonly DependencyProperty DisableDoorRelockingProperty = DependencyProperty.Register(
          "DisableDoorRelocking", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty AlertAiThroughDoorsProperty = DependencyProperty.Register(
          "AlertAiThroughDoors", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty PreferUse5FootStepProperty = DependencyProperty.Register(
          "PreferUse5FootStep", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty ExtendedSpellDescriptionsProperty = DependencyProperty.Register(
          "ExtendedSpellDescriptions", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty NewClassesProperty = DependencyProperty.Register(
          "NewClasses", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty NonCoreProperty = DependencyProperty.Register(
          "NonCore", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty NewRacesProperty = DependencyProperty.Register(
          "NewRaces", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty MonstrousRacesProperty = DependencyProperty.Register(
          "MonstrousRaces", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty ForgottenRealmsRacesProperty = DependencyProperty.Register(
          "ForgottenRealmsRacesRaces", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty LaxRulesProperty = DependencyProperty.Register(
          "LaxRules", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty StricterRulesEnforcementProperty = DependencyProperty.Register(
          "StricterRulesEnforcement", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));

        public static readonly DependencyProperty DisableAlignmentRestrictionsProperty = DependencyProperty.Register(
          "DisableAlignmentRestrictions", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty DisableCraftingSpellReqsProperty = DependencyProperty.Register(
          "DisableCraftingSpellReqs", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty DisableMulticlassXpPenaltyProperty = DependencyProperty.Register(
          "DisableMulticlassXpPenalty", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty ShowTargetingCirclesInFogOfWarProperty = DependencyProperty.Register(
          "ShowTargetingCirclesInFogOfWar", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        public static readonly DependencyProperty WildshapeUsableItemsProperty = DependencyProperty.Register(
          "WildshapeUsableItems", typeof(bool), typeof(IniViewModel), new PropertyMetadata(default(bool)));
        

        public IEnumerable<HpOnLevelUpType> HpOnLevelUpTypes => Enum.GetValues(typeof (HpOnLevelUpType))
            .Cast<HpOnLevelUpType>();

        public IEnumerable<HpForNPCHdType> HpForNPCHdTypes => Enum.GetValues(typeof(HpForNPCHdType))
            .Cast<HpForNPCHdType>();

        public IEnumerable<FogOfWarType> FogOfWarTypes => Enum.GetValues(typeof(FogOfWarType))
           .Cast<FogOfWarType>();
        public IEnumerable<NumberOfPcsType> NumberOfPcsTypes => Enum.GetValues(typeof(NumberOfPcsType))
           .Cast<NumberOfPcsType>();

        public IniViewModel()
        {
            var screenSize = ScreenResolution.ScreenSize;
            RenderWidth = (int)screenSize.Width;
            RenderHeight = (int)screenSize.Height;
            PointBuyPoints = 25;
            MaxLevel = 10;
            WalkDistanceFt = 0;
            SlowerLevelling = false;
            AllowXpOverflow = false;
            NumberOfPcs = NumberOfPcsType.PCs_5_NPCs_3;
            NeedsCo8Defaults = false;
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

        public HpForNPCHdType HpForNPCHd
        {
            get { return (HpForNPCHdType)GetValue(HpForNPCHdProperty); }
            set { SetValue(HpForNPCHdProperty, value); }
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

        public NumberOfPcsType NumberOfPcs
        {
            get { return (NumberOfPcsType)GetValue(NumberOfPcsProperty); }
            set { SetValue(NumberOfPcsProperty, value); }
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

        public int WalkDistanceFt
        {
            get { return (int)GetValue(WalkDistanceFtProperty); }
            set { if (value < 0)
                    SetValue(WalkDistanceFtProperty, 0);
                else
                    SetValue(WalkDistanceFtProperty, value ); }
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

        public bool PreferUse5FootStep
        {
            get { return (bool)GetValue(PreferUse5FootStepProperty); }
            set { SetValue(PreferUse5FootStepProperty, value); }
        }

        public bool ExtendedSpellDescriptions
        {
            get { return (bool)GetValue(ExtendedSpellDescriptionsProperty); }
            set { SetValue(ExtendedSpellDescriptionsProperty, value); }
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

        public bool NewRaces
        {
            get { return (bool)GetValue(NewRacesProperty); }
            set { SetValue(NewRacesProperty, value); }
        }
        public bool MonstrousRaces
        {
            get { return (bool)GetValue(MonstrousRacesProperty); }
            set { SetValue(MonstrousRacesProperty, value); }
        }
        public bool ForgottenRealmsRaces
        {
            get { return (bool)GetValue(ForgottenRealmsRacesProperty); }
            set { SetValue(ForgottenRealmsRacesProperty, value); }
        }

        public bool LaxRules
        {
            get { return (bool)GetValue(LaxRulesProperty); }
            set { SetValue(LaxRulesProperty, value); }
        }

        public bool StricterRulesEnforcement
        {
            get { return (bool)GetValue(StricterRulesEnforcementProperty); }
            set { SetValue(StricterRulesEnforcementProperty, value); }
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
        public bool WildshapeUsableItems
        {
            get { return (bool)GetValue(WildshapeUsableItemsProperty); }
            set { SetValue(WildshapeUsableItemsProperty, value); }
        }

        public bool NeedsCo8Defaults { get; internal set; }

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

            if (tpData["HpForNPCHd"] != null)
            {
                switch (tpData["HpForNPCHd"].ToLowerInvariant())
                {
                    case "min":
                        HpForNPCHd = HpForNPCHdType.Min;
                        break;
                    case "max":
                        HpForNPCHd = HpForNPCHdType.Max;
                        break;
                    case "average":
                        HpForNPCHd = HpForNPCHdType.Average;
                        break;
                    case "threefourth":
                        HpForNPCHd = HpForNPCHdType.ThreeFourth;
                        break;
                    default:
                        HpForNPCHd = HpForNPCHdType.Normal;
                        break;
                }
            }

            //Handle reading in old setting (won't be written later)
            bool MaxHpForNpcHitdice = tpData["maxHpForNpcHitdice"] == "true";
            if (MaxHpForNpcHitdice)
            {
                HpForNPCHd = HpForNPCHdType.Max;
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

            bool maxPCsFlexible = false;
            
            bool.TryParse(tpData["maxPCsFlexible"], out maxPCsFlexible);
            if (tpData["maxPCsFlexible"] == null)
            {
                NeedsCo8Defaults = true;
            }
            if (maxPCsFlexible)
            {
                NumberOfPcs = NumberOfPcsType.Flexible;
            }
            else
            {
                int maxPCs;
                if (int.TryParse(tpData["maxPCs"], out maxPCs)){
                    switch (maxPCs)
                    {
                        case 3:
                            NumberOfPcs = NumberOfPcsType.PCs_3_NPCs_5;
                            break;
                        case 4:
                            NumberOfPcs = NumberOfPcsType.PCs_4_NPCs_4;
                            break;
                        case 5:
                            NumberOfPcs = NumberOfPcsType.PCs_5_NPCs_3;
                            break;
                        case 6:
                            NumberOfPcs = NumberOfPcsType.PCs_6_NPCs_2;
                            break;
                        case 7:
                            NumberOfPcs = NumberOfPcsType.PCs_7_NPCs_1;
                            break;
                        case 8:
                            NumberOfPcs = NumberOfPcsType.PCs_8_NPCs_0;
                            break;
                        default:
                            NumberOfPcs = NumberOfPcsType.PCs_5_NPCs_3;
                            break;
                    }
                }
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
            int walkDistFt;
            if (int.TryParse(tpData["walkDistanceFt"], out walkDistFt))
            {
                if (walkDistFt < 0)
                    walkDistFt = 0;
                WalkDistanceFt = walkDistFt;
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

            bool preferUse5FootStep;
            if (bool.TryParse(tpData["preferUse5FootStep"], out preferUse5FootStep))
            {
                PreferUse5FootStep = preferUse5FootStep;
            }

            bool extendedSpellDescriptions;
            if (bool.TryParse(tpData["extendedSpellDescriptions"], out extendedSpellDescriptions))
            {
                ExtendedSpellDescriptions = extendedSpellDescriptions;
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
            bool newRaces;
            if (bool.TryParse(tpData["newRaces"], out newRaces))
            {
                NewRaces = newRaces;
            }
            bool monstrousRaces;
            if (bool.TryParse(tpData["monstrousRaces"], out monstrousRaces))
            {
                MonstrousRaces = monstrousRaces;
            }
            bool forgottenRealmsRaces;
            if (bool.TryParse(tpData["forgottenRealmsRaces"], out forgottenRealmsRaces))
            {
                ForgottenRealmsRaces = forgottenRealmsRaces;
            }

            // Lax Rules
            bool laxRules;
            if (bool.TryParse(tpData["laxRules"], out laxRules))
            {
                LaxRules = laxRules;
            }

            bool stricterRulesEnforcement;
            if (bool.TryParse(tpData["stricterRulesEnforcement"], out stricterRulesEnforcement))
            {
                StricterRulesEnforcement = stricterRulesEnforcement;
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
            bool wildshapeUsableItems;
            if (bool.TryParse(tpData["wildShapeUsableItems"], out wildshapeUsableItems))
            {
                WildshapeUsableItems = wildshapeUsableItems;
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

            switch (HpForNPCHd)
            {
                case HpForNPCHdType.Min:
                    tpData["HpForNPCHd"] = "min";
                    break;
                case HpForNPCHdType.Max:
                    tpData["HpForNPCHd"] = "max";
                    break;
                case HpForNPCHdType.Average:
                    tpData["HpForNPCHd"] = "average";
                    break;
                case HpForNPCHdType.ThreeFourth:
                    tpData["HpForNPCHd"] = "threefourth";
                    break;
                default:
                    tpData["HpForNPCHd"] = "normal";
                    break;
            }

            //Set the old setting to false
            tpData["maxHpForNpcHitdice"] = "false";
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
            tpData["stricterRulesEnforcement"] = StricterRulesEnforcement ? "true" : "false";

            tpData["disableAlignmentRestrictions"] = DisableAlignmentRestrictions ? "true" : "false";
            tpData["disableCraftingSpellReqs"] = DisableCraftingSpellReqs ? "true" : "false";
            tpData["disableMulticlassXpPenalty"] = DisableMulticlassXpPenalty ? "true" : "false";
            tpData["showTargetingCirclesInFogOfWar"] = ShowTargetingCirclesInFogOfWar ? "true" : "false";
            tpData["wildShapeUsableItems"] = WildshapeUsableItems ? "true" : "false";
            

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
            tpData["maxPCsFlexible"] = "false";
            switch (NumberOfPcs)
            {
                case NumberOfPcsType.Flexible:
                    tpData["maxPCsFlexible"] = "true";
                    break;
                case NumberOfPcsType.PCs_3_NPCs_5:
                    tpData["maxPCs"] = 3.ToString();
                    break;
                case NumberOfPcsType.PCs_4_NPCs_4:
                    tpData["maxPCs"] = 4.ToString();
                    break;
                case NumberOfPcsType.PCs_5_NPCs_3:
                    tpData["maxPCs"] = 5.ToString();
                    break;
                case NumberOfPcsType.PCs_6_NPCs_2:
                    tpData["maxPCs"] = 6.ToString();
                    break;
                case NumberOfPcsType.PCs_7_NPCs_1:
                    tpData["maxPCs"] = 7.ToString();
                    break;
                case NumberOfPcsType.PCs_8_NPCs_0:
                    tpData["maxPCs"] = 8.ToString();
                    break;
                default:
                    tpData["maxPCs"] = 5.ToString();
                    break;
            }
            tpData["allowXpOverflow"] = AllowXpOverflow ? "true" : "false";
            tpData["slowerLevelling"] = SlowerLevelling ? "true" : "false";
            tpData["newClasses"] = NewClasses? "true" : "false";
            tpData["newRaces"] = NewRaces? "true" : "false";
            tpData["monstrousRaces"] = MonstrousRaces? "true" : "false";
            tpData["forgottenRealmsRaces"] = ForgottenRealmsRaces ? "true" : "false";
            tpData["nonCoreMaterials"] = NonCore ? "true" : "false";
            tpData["tolerantNpcs"] = TolerantTownsfolk? "true" : "false";
            tpData["showExactHPforNPCs"] = TransparentNpcStats? "true" : "false";
            tpData["showNpcStats"] = TransparentNpcStats ? "true" : "false";
            tpData["fastSneakAnim"] = FastSneaking ? "true" : "false";
            if (WalkDistanceFt < 0) WalkDistanceFt = 0;
            tpData["walkDistanceFt"] =WalkDistanceFt.ToString();
            tpData["disableDoorRelocking"] = DisableDoorRelocking? "true" : "false";
            tpData["alertAiThroughDoors"] = AlertAiThroughDoors ? "true" : "false";
            tpData["preferUse5FootStep"] = PreferUse5FootStep ? "true" : "false";
            tpData["extendedSpellDescriptions"] = ExtendedSpellDescriptions ? "true" : "false";
        }
    }

    public enum NumberOfPcsType
    {
        [System.ComponentModel.Description("3 PCs")]
        PCs_3_NPCs_5,
        [System.ComponentModel.Description("4 PCs")]
        PCs_4_NPCs_4,
        [System.ComponentModel.Description("5 PCs")]
        PCs_5_NPCs_3,
        [System.ComponentModel.Description("6 PCs")]
        PCs_6_NPCs_2,
        [System.ComponentModel.Description("7 PCs")]
        PCs_7_NPCs_1,
        [System.ComponentModel.Description("8 PCs")]
        PCs_8_NPCs_0,
        [System.ComponentModel.Description("Flexible")]
        Flexible // allows UPTO 8 PCs
    }

    public enum HpOnLevelUpType
    {
        Normal,
        Max,
        Average
    }

    public enum HpForNPCHdType
    {
        Normal,
        Min,
        Average,
        ThreeFourth,
		Max
    }

    public enum FogOfWarType
    {
        Normal,
        Unfogged,
        Always // ensures maps are fogged even despite UNFOGGED flag in the module defs
    }
}