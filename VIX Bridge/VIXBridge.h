#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#define VIX_EXPORT __declspec(dllexport)    // building DLL
#define VIX_IMPORT __declspec(dllimport)    // consuming DLL
#else
#define VIX_EXPORT __attribute__((visibility("default"))) // GCC/Clang export
#define VIX_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

    // Define the type for the logging function C# will pass
    typedef void (*LogCallback)(const char* message);

    typedef struct VFile_struct {
        char* Name;
        char* Extension;
        char* Path;
        bool isDirectory;

        int ChildrenCount;
        struct VFile_struct** Children;    // <-- dynamic array (pointer to array of pointers)

    } VFile;

    // We add a new function to register the callback
    VIX_EXPORT void SetLogCallback(LogCallback callback);

    // Exported function declarations
    VIX_EXPORT VFile* ParseDirectory(const char* Path, size_t* FileCount);
    VIX_EXPORT VFile* ParseDirectory_Recursive(const char* Path, size_t* FileCount);

#ifdef __cplusplus
}
#endif