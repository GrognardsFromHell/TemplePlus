using NuGet;
using Splat;
using Squirrel;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

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
            HandleEvents(
                onInitialInstall: () => CreateShortcuts(),
                onAppUpdate: () => CreateShortcuts(),
                onAppUninstall: () => RemoveShortcuts(),
                onFirstRun: () =>{
                    firstStart = true;
                    var notifWnd = new InstalledNotifWnd();
                    notifWnd.ShowDialog();
                });

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

        /// <summary>
        /// Call this method as early as possible in app startup. This method
        /// will dispatch to your methods to set up your app. Depending on the
        /// parameter, your app will exit after this method is called, which 
        /// is required by Squirrel. UpdateManager has methods to help you to
        /// do this, such as CreateShortcutForThisExe.
        /// </summary>
        /// <param name="onInitialInstall">Called when your app is initially
        /// installed. Set up app shortcuts here as well as file associations.
        /// </param>
        /// <param name="onAppUpdate">Called when your app is updated to a new
        /// version.</param>
        /// <param name="onAppObsoleted">Called when your app is no longer the
        /// latest version (i.e. they have installed a new version and your app
        /// is now the old version)</param>
        /// <param name="onAppUninstall">Called when your app is uninstalled 
        /// via Programs and Features. Remove all of the things that you created
        /// in onInitialInstall.</param>
        /// <param name="onFirstRun">Called the first time an app is run after
        /// being installed. Your application will **not** exit after this is
        /// dispatched, you should use this as a hint (i.e. show a 'Welcome' 
        /// screen, etc etc.</param>
        /// <param name="arguments">Use in a unit-test runner to mock the 
        /// arguments. In your app, leave this as null.</param>
        public static void HandleEvents(
            Action onInitialInstall = null,
            Action onAppUpdate = null,
            Action onAppObsoleted = null,
            Action onAppUninstall = null,
            Action onFirstRun = null,
            string[] arguments = null)
        {
            Action defaultBlock = (() => { });
            var args = arguments ?? Environment.GetCommandLineArgs().Skip(1).ToArray();
            if (args.Length == 0) return;

            var lookup = new[] {
            new { Key = "--squirrel-install", Value = onInitialInstall ?? defaultBlock },
            new { Key = "--squirrel-updated", Value = onAppUpdate ?? defaultBlock },
            new { Key = "--squirrel-obsolete", Value = onAppObsoleted ?? defaultBlock },
            new { Key = "--squirrel-uninstall", Value = onAppUninstall ?? defaultBlock },
        }.ToDictionary(k => k.Key, v => v.Value);

            if (args[0] == "--squirrel-firstrun")
            {
                (onFirstRun ?? (() => { }))();
                return;
            }

            if (args.Length != 2) return;

            if (!lookup.ContainsKey(args[0])) return;

            try
            {
                lookup[args[0]]();
                Environment.Exit(0);
            }
            catch (Exception ex)
            {
                LogHost.Default.ErrorException("Failed to handle Squirrel events", ex);
                Environment.Exit(-1);
            }
        }
    }
}
