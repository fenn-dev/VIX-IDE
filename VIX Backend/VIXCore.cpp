#include "../VIX Bridge/VIXBridge.h"
#include <filesystem>
#include <vector>
#include <iostream>

#ifdef _WIN32
// Windows: Use __declspec(dllexport) to mark functions for export from a DLL.
#define VIXExport __declspec(dllexport)
#else
// Linux/macOS/Other Unix-like systems: Use the GCC/Clang visibility attribute.
// 'default' means the symbol should be visible outside the shared library.
#define VIXExport __attribute__((visibility("default")))
#endif

namespace fs = std::filesystem;


namespace _VIX {
	inline char* Allocation(const char* str) {
		if (!str) return nullptr;
		size_t len = strlen(str) + 1;
		char* copy = (char*)malloc(len);
		if (copy) strcpy(copy, str);
		return copy;
	}
}
namespace VIX {

	

    VFile* parseDirectory(const char* Path, size_t* outCount) {
        if (!fs::exists(Path)) {
            std::cerr << "Directory does not exist: " << Path << "\n";
            *outCount = 0;
            return nullptr;
        }

        // Count number of entries first
        size_t count = 0;
        for (auto& entry : fs::directory_iterator(Path))
            ++count;

        if (count == 0) {
            *outCount = 0;
            return nullptr;
        }

        // Allocate C-style array
        VFile* files = (VFile*)malloc(sizeof(VFile) * count);
        if (!files) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }

        size_t i = 0;
        for (auto& entry : fs::directory_iterator(Path)) {
            VFile File = {};
            File.isDirectory = entry.is_directory();

            File.Name = _VIX::Allocation(entry.path().filename().string().c_str());
            File.Extension = _VIX::Allocation(entry.path().extension().string().c_str());
            File.Path = _VIX::Allocation(entry.path().string().c_str());

            if (!File.Name || !File.Extension || !File.Path) {
                perror("malloc failed");
                exit(EXIT_FAILURE);
            }

            files[i++] = File;
        }

        *outCount = count;
        return files;
    }

	VFile*	parseDirectory_Recursive(const char* Path) {

	}

	void	parseDirectory_Clear() {

	};
}

extern "C" {
	VIXExport VFile* ParseDirectory(const char* Path) {
		return VIX::parseDirectory(Path);
	}

	VIXExport VFile* ParseDirectory_Recursive(const char* Path) {
		return VIX::parseDirectory_Recursive(Path);
	}

	VIXExport void		ParseDirectory_Clear() {
		VIX::parseDirectory_Clear();
	}
}
