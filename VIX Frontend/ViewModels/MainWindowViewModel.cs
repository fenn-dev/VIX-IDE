using Avalonia.Threading;
using System;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;

#region Unmanaged Bridge

[StructLayout(LayoutKind.Sequential)]
public struct VFile
{
    public IntPtr Name;
    public IntPtr Extension;
    public IntPtr Path;

    [MarshalAs(UnmanagedType.I1)]
    public bool IsDirectory;

    public int ChildrenCount;
    public IntPtr Children; // VFile** (pointer to unmanaged array of VFile*)

    // Not part of unmanaged memory — filled in C#
    public VFile[] ManagedChildren;
}

public static class VixBridge
{
    const string DLL_NAME = "VIX Bridge.dll";

    [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ParseDirectory(string path, out UIntPtr fileCount);

    [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ParseDirectory_Recursive(string path, out UIntPtr fileCount);

    public static VFile MarshalVFile(IntPtr ptr)
    {
        VFile file = Marshal.PtrToStructure<VFile>(ptr);

        if (file.Children != IntPtr.Zero && file.ChildrenCount > 0)
        {
            file.ManagedChildren = new VFile[file.ChildrenCount];
            int ptrSize = IntPtr.Size;

            for (int i = 0; i < file.ChildrenCount; i++)
            {
                IntPtr childPtr = Marshal.ReadIntPtr(file.Children, i * ptrSize);
                if (childPtr != IntPtr.Zero)
                {
                    file.ManagedChildren[i] = MarshalVFile(childPtr);
                }
            }
        }
        else
        {
            file.ManagedChildren = Array.Empty<VFile>();
        }

        return file;
    }
}

#endregion

namespace VIX_Frontend.ViewModels
{
    using CommunityToolkit.Mvvm.Input;
    using System.Collections.ObjectModel;
    using System.Runtime.InteropServices;
    using System.Threading.Tasks;
    using System.Windows.Input;

    #region FileItem

    public class FileItem
    {
        public string Name { get; set; }
        public bool IsDirectory { get; set; }
        public ObservableCollection<FileItem> Children { get; set; }

        public FileItem(VFile file)
        {
            Name = Marshal.PtrToStringAnsi(file.Name);
            IsDirectory = file.IsDirectory;
            Children = new ObservableCollection<FileItem>();

            if (file.ManagedChildren != null)
            {
                foreach (var child in file.ManagedChildren)
                {
                    Children.Add(new FileItem(child));
                }
            }
        }
    }

    #endregion

    #region MainWindowViewModel

    public partial class MainWindowViewModel : ViewModelBase
    {
        public ObservableCollection<FileItem> FileTree { get; } = new ObservableCollection<FileItem>();

        public ICommand RefreshCommand => new RelayCommand(FileExplorer);

        public void FileExplorer()
        {
            Task.Run(() =>
            {
                try
                {
                    string path = @"C:\Users\rasmu\Desktop\fenn-dev.github.io";
                    UIntPtr count;
                    IntPtr ptr = VixBridge.ParseDirectory_Recursive(path, out count);

                    Console.WriteLine($"ptr: {ptr}, count: {count}");
                    if (ptr == IntPtr.Zero || count == UIntPtr.Zero) return;

                    var items = new ObservableCollection<FileItem>();
                    int total = (int)count;

                    for (int i = 0; i < total; i++)
                    {
                        IntPtr filePtr = Marshal.ReadIntPtr(ptr, i * IntPtr.Size);
                        VFile file = VixBridge.MarshalVFile(filePtr);
                        items.Add(new FileItem(file));
                    }

                    Dispatcher.UIThread.Post(() =>
                    {
                        FileTree.Clear();
                        foreach (var item in items)
                            FileTree.Add(item);
                    });
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Exception: {ex}");
                }
            });
        }

    }

    #endregion vd
}
