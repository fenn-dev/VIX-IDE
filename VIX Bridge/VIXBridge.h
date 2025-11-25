#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#define VIX_EXPORT __declspec(dllexport)   // building DLL
#define VIX_IMPORT __declspec(dllimport)   // consuming DLL
#else
#define VIX_EXPORT __attribute__((visibility("default")))  // GCC/Clang export
#define VIX_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct VFile_struct {
		char* Name;
		char* Extension;
		char* Path;
		bool isDirectory;
		
		int ChildrenCount;
		struct VFile_struct** Children;   // <-- dynamic array (pointer to array)

	} VFile;

#define VFILE_INIT { \
    .Name = NULL,				\
	.Extension = NULL,			\
    .Path = NULL,				\
    .isDirectory = false,		\
    .ChildrenCount = 100,		\
}

#ifdef __cplusplus
}
#endif
