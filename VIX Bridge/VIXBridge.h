#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#ifdef BUILD_VIX_BRIDGE
#define VIX_API __declspec(dllexport)
#else
#define VIX_API __declspec(dllimport)
#endif
#else
#define VIX_API
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
		VFile* Children[100];

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
