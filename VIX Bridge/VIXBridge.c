#define _CRT_SECURE_NO_WARNINGS // Suppresses warnings about unsafe C functions
#include "VIXBridge.h"
#include <stdio.h>
#include <windows.h>
#include <string.h>

// Function pointer types
typedef VFile* (*parseDir)(const char*, size_t*);
typedef VFile* (*parseDir_recursive)(const char*, size_t*);
typedef void (*freeDir)(VFile*, size_t); // Define type for memory cleanup

// Global callback storage and function pointers
// LogCallback is defined in VIXBridge.h, now correctly included
static LogCallback g_LogCallback = NULL;
parseDir ParseDir = NULL;
parseDir_recursive ParseDir_recursive = NULL;
freeDir FreeDir = NULL;

// Helper to send log messages (uses the callback if set, otherwise printf)
static void VixLog(const char* message) {
    if (g_LogCallback) {
        g_LogCallback(message);
    }
    else {
        // Fallback print for debugging the bridge itself
        printf("[VIX C-Bridge Log] %s\n", message);
    }
}

// Function to set the callback pointer from C#
VIX_EXPORT void SetLogCallback(LogCallback callback) {
    g_LogCallback = callback;
    VixLog("Log callback successfully registered.");
}


// Initialization function
int init() {
    char logBuffer[256];

    // CRITICAL: The exact path where VIX Backend.dll resides.
    HMODULE hDll = LoadLibrary(TEXT("C:/Users/rasmu/source/repos/VIX Interlink/x64/Release/VIX Backend.dll"));
    if (!hDll) {
        // Output error if the DLL cannot be found or loaded
        DWORD errorCode = GetLastError();
        // FIX: Correct usage of sprintf_s by adding buffer size
        sprintf_s(logBuffer, sizeof(logBuffer), "[C-BRIDGE ERROR] Failed to load VIX Backend.dll. Error code: %lu. Path: C:/Users/rasmu/source/repos/VIX Interlink/x64/Release/VIX Backend.dll", errorCode);
        VixLog(logBuffer);
        return 0;
    }

    // Resolve function addresses
    ParseDir = (parseDir)GetProcAddress(hDll, "ParseDirectory");
    ParseDir_recursive = (parseDir_recursive)GetProcAddress(hDll, "ParseDirectory_Recursive");
    // FreeDir = (freeDir)GetProcAddress(hDll, "FreeDirectory"); // Uncomment and resolve if C++ implements FreeDirectory


    if (!ParseDir || !ParseDir_recursive) {
        // Output error if functions are not found (mismatch name/export)
        // FIX: Correct usage of sprintf_s by adding buffer size
        sprintf_s(logBuffer, sizeof(logBuffer), "[C-BRIDGE ERROR] Function addresses not found. ParseDir: %p, ParseDir_recursive: %p", ParseDir, ParseDir_recursive);
        VixLog(logBuffer);
        FreeLibrary(hDll);
        return 0;
    }

    VixLog("[C-BRIDGE] VIX Backend initialized successfully.");
    return 1;
}

// Exported functions (The targets of C# P/Invoke)
VIX_EXPORT VFile* ParseDirectory(const char* Path, size_t* FileCount) {
    *FileCount = 0;
    if (!ParseDir) {
        if (!init()) {
            VixLog("[C-BRIDGE ERROR] ParseDirectory failed due to initialization error.");
            return NULL;
        }
    }
    // Safe dereference since init() guarantees it's non-NULL if successful
    return ParseDir(Path, FileCount);
}

VIX_EXPORT VFile* ParseDirectory_Recursive(const char* Path, size_t* FileCount) {
    *FileCount = 0;
    if (!ParseDir_recursive) {
        if (!init()) {
            VixLog("[C-BRIDGE ERROR] ParseDirectory_Recursive failed due to initialization error.");
            return NULL;
        }
    }
    // Safe dereference since init() guarantees it's non-NULL if successful
    return ParseDir_recursive(Path, FileCount);
}

// FIX: Implementation of FreeDirectory to resolve definition not found error
VIX_EXPORT void FreeDirectory(VFile* files, size_t fileCount) {
    if (FreeDir) {
        // Call the C++ function if the pointer was successfully resolved
        FreeDir(files, fileCount);
    }
    else {
        // If the C++ cleanup function wasn't resolved, it's a memory leak risk.
        // We log a warning, but we cannot safely free the memory here (since it was allocated
        // by the C++ DLL using malloc/new, which must be freed by the same module/method).
        VixLog("[C-BRIDGE WARNING] FreeDirectory called, but C++ cleanup function (FreeDir) pointer is NULL. Memory may be leaking.");
    }
}