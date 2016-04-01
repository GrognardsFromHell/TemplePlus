using Squirrel;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace TemplePlusConfig
{
    class Program
    {

        private const string UpdateFeed = "https://templeplus.org/update-feeds/stable";

        [STAThread]
        public static void Main(string[] args)
        {
            bool firstStart = false;

            // Note, in most of these scenarios, the app exits after this method
            // completes!
            SquirrelAwareApp.HandleEvents(
                onInitialInstall: v => CreateShortcuts(),
                onAppUpdate: v => CreateShortcuts(),
                onAppUninstall: v => RemoveShortcuts(),
                onFirstRun: () => firstStart = true);

            App.LaunchAfterSave = firstStart;
            App.Main();
        }

        private static void CreateShortcuts()
        {
            using (var mgr = new UpdateManager(UpdateFeed))
            {
                var update = Environment.CommandLine.Contains("squirrel-install") == false;

                mgr.CreateShortcutsForExecutable("TemplePlusConfig.exe",
                        ShortcutLocation.StartMenu,
                        update,
                        null,
                        null);

                // Get path to TemplePlus.exe and create shortcut for it
                mgr.CreateShortcutsForExecutable("TemplePlus.exe",
                        ShortcutLocation.StartMenu,
                        update,
                        null,
                        null);

            }
        }

        private static void RemoveShortcuts()
        {
            using (var mgr = new UpdateManager(UpdateFeed))
            {
                mgr.RemoveShortcutsForExecutable("TemplePlusConfig.exe", ShortcutLocation.StartMenu);
                mgr.RemoveShortcutsForExecutable("TemplePlus.exe", ShortcutLocation.StartMenu);
            }
        }
    }
}
