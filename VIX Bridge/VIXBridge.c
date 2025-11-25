#include "VIXBridge.h"
#include <stdio.h>
#include <windows.h>

typedef VFile* (*parseDir)(const char*, size_t*);
typedef VFile* (*parseDir_recursive)(const char*, size_t*);

parseDir ParseDir = NULL;
parseDir_recursive ParseDir_recursive = NULL;

int init() {
    HMODULE hDll = LoadLibrary(TEXT("VIX Backend.dll"));
    if (!hDll) {
        printf("Failed to load DLL. Error code: %lu\n", GetLastError());
        return 0;
    }

    ParseDir = (parseDir)GetProcAddress(hDll, "ParseDirectory");
    ParseDir_recursive = (parseDir_recursive)GetProcAddress(hDll, "ParseDirectory_Recursive");

    if (!ParseDir || !ParseDir_recursive) {
        printf("Failed to get function addresses. Error code: %lu\n", GetLastError());
        FreeLibrary(hDll);
        return 0;
    }

    return 1;
}

// Exported functions
VIX_EXPORT VFile* ParseDirectory(const char* Path, size_t* FileCount) {
    return ParseDir(Path, FileCount);
}

VIX_EXPORT VFile* ParseDirectory_Recursive(const char* Path, size_t* FileCount) {
    return ParseDir_recursive(Path, FileCount);
}
