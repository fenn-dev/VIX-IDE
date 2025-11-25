using System;
using Avalonia;
using System.Runtime.InteropServices;

namespace VIX_Frontend
{
    internal sealed class Program
    {
        // Import the necessary function to create a new console
        [DllImport("kernel32.dll")]
        private static extern bool AllocConsole();

        [STAThread]
        public static void Main(string[] args)
        {
            // --- FIX START ---
            // AllocConsole() is now called unconditionally, so it runs in both 
            // Debug and Release configurations.
            AllocConsole();
            // --- FIX END ---

            BuildAvaloniaApp()
                .StartWithClassicDesktopLifetime(args);
        }

        public static AppBuilder BuildAvaloniaApp()
            => AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .WithInterFont()
                .LogToTrace();
    }
}