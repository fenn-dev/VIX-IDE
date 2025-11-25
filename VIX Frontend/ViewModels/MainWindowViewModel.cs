using Avalonia.Threading;
using CommunityToolkit.Mvvm.Input;
using System;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows.Input;
using VIX_Frontend.ViewModels;

// Assuming ViewModelBase is defined elsewhere
public abstract class ViewModelBase { }

#region Unmanaged Bridge

// Define the C# delegate signature that matches the C LogCallback typedef
[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void LogCallbackDelegate(string message);

[StructLayout(LayoutKind.Sequential)]
public struct VFile
{
    // ... (VFile struct remains unchanged)
    public IntPtr Name;
    public IntPtr Extension;
    public IntPtr Path;

    [MarshalAs(UnmanagedType.I1)]
    public bool IsDirectory;

    public int ChildrenCount;
    public IntPtr Children; // VFile**
}

public static class VixBridge
{
    const string DLL_NAME = "C:/Users/rasmu/source/repos/VIX Interlink/x64/Release/VIX Bridge.dll";

    // New DllImport for registering the callback
    [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetLogCallback(LogCallbackDelegate callback);

    // Existing DllImports
    [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ParseDirectory(string path, out UIntPtr fileCount);

    [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ParseDirectory_Recursive(string path, out UIntPtr fileCount);

    [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
    public static extern void FreeDirectory(IntPtr ptr, UIntPtr fileCount);

    // ... (MarshalVFile remains unchanged)
    public static FileItem MarshalVFile(IntPtr ptr)
    {
        if (ptr == IntPtr.Zero) return null;

        VFile file = Marshal.PtrToStructure<VFile>(ptr);
        string name = file.Name != IntPtr.Zero ? Marshal.PtrToStringAnsi(file.Name) : "Unknown";

        FileItem item = new FileItem(name: name, isDirectory: file.IsDirectory);

        if (file.Children != IntPtr.Zero && file.ChildrenCount > 0)
        {
            int ptrSize = IntPtr.Size;
            for (int i = 0; i < file.ChildrenCount; i++)
            {
                IntPtr childPtr = Marshal.ReadIntPtr(file.Children, i * ptrSize);
                if (childPtr != IntPtr.Zero)
                {
                    FileItem childItem = MarshalVFile(childPtr);
                    if (childItem != null)
                    {
                        item.Children.Add(childItem);
                    }
                }
            }
        }
        return item;
    }
}

#endregion

namespace VIX_Frontend.ViewModels
{
    // ... (FileItem region remains unchanged)
    #region FileItem

    public class FileItem
    {
        public string Name { get; set; }
        public bool IsDirectory { get; set; }
        public ObservableCollection<FileItem> Children { get; set; }

        public FileItem(string name, bool isDirectory)
        {
            Name = name;
            IsDirectory = isDirectory;
            Children = new ObservableCollection<FileItem>();
        }
    }

    #endregion

    #region MainWindowViewModel

    public partial class MainWindowViewModel : ViewModelBase
    {
        // ⚠️ CRITICAL: Hold a reference to the delegate so the GC doesn't collect it.
        private readonly LogCallbackDelegate _logCallbackDelegate;

        public MainWindowViewModel()
        {
            // 1. Instantiate the delegate pointing to our local logging method
            _logCallbackDelegate = LogFromUnmanaged;

            // 2. Register the callback with the C-Bridge immediately
            try
            {
                VixBridge.SetLogCallback(_logCallbackDelegate);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[C# ERROR] Failed to register Log Callback: {ex.Message}");
            }

            Console.WriteLine("WEEEEE");
        }

        // 3. The managed method the C-Bridge will call
        private void LogFromUnmanaged(string message)
        {
            Console.WriteLine($"[UNMANAGED LOG] {message}");
        }


        public ObservableCollection<FileItem> FileTree { get; } = new ObservableCollection<FileItem>();
        public ICommand RefreshCommand => new RelayCommand(FileExplorer);

        public void FileExplorer()
        {
            // The unmanaged call is run on a background thread
            Task.Run(() =>
            {
                UIntPtr count = UIntPtr.Zero;
                IntPtr ptr = IntPtr.Zero;

                try
                {
                    // Call the unmanaged code
                    ptr = VixBridge.ParseDirectory_Recursive("C:\\Users\\rasmu\\source\\repos\\FoxGaming208\\Eclipse-of-Order", out count);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[VIX Bridge ERROR] P/Invoke call failed: {ex.Message}");
                    return;
                }

                long fileCount = unchecked((long)count);
                Console.WriteLine($"[VIX Bridge] Returned PTR: {ptr}");
                Console.WriteLine($"[VIX Bridge] Returned COUNT: {fileCount}");

                if (ptr == IntPtr.Zero || count == UIntPtr.Zero)
                {
                    Console.WriteLine("[VIX Bridge] RESULT IS EMPTY. Check UNMANAGED LOG for initialization error.");
                    return;
                }

                var items = new ObservableCollection<FileItem>();
                int size = Marshal.SizeOf<VFile>();
                int successfulMarshals = 0;

                for (int i = 0; i < (int)count; i++)
                {
                    IntPtr filePtr = ptr + i * size;
                    FileItem rootItem = VixBridge.MarshalVFile(filePtr);

                    if (rootItem != null)
                    {
                        items.Add(rootItem);
                        successfulMarshals++;
                    }
                }

                Console.WriteLine($"[VIX Bridge] Successfully marshaled {successfulMarshals} root items.");

                Dispatcher.UIThread.Post(() =>
                {
                    FileTree.Clear();
                    foreach (var item in items)
                        FileTree.Add(item);
                });

                // --- MEMORY CLEANUP ---
                // ... (Memory cleanup logic remains)
                try
                {
                    VixBridge.FreeDirectory(ptr, count);
                    Console.WriteLine("[VIX Bridge] Unmanaged memory freed successfully.");
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[VIX Bridge ERROR] Failed to free unmanaged memory: {ex.Message}");
                }
            });
        }
    }

    #endregion
}