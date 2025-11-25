#include "../VIX Bridge/VIXBridge.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;


namespace _VIX {
    inline char* Allocation(const char* str) {
        if (!str) return nullptr;
        size_t len = strlen(str) + 1;
        char* copy = (char*)malloc(len);
        if (copy) memcpy(copy, str, len);
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

        size_t count = 0;
        for (auto& entry : fs::directory_iterator(Path))
            ++count;

        if (count == 0) {
            *outCount = 0;
            return nullptr;
        }

        VFile* files = (VFile*)malloc(sizeof(VFile) * count);
        if (!files) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }

        size_t i = 0;
        for (auto& entry : fs::directory_iterator(Path)) {
            VFile File = {};

            File.isDirectory = entry.is_directory();
            File.Children = nullptr;      // <-- important
            File.ChildrenCount = 0;       // <-- important

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


    VFile* parseDirectory_Recursive(const char* Path, size_t* outCount)
    {
        namespace fs = std::filesystem;

        if (!fs::exists(Path)) {
            std::cerr << "Directory does not exist: " << Path << "\n";
            *outCount = 0;
            return nullptr;
        }

        // Count direct children
        size_t count = 0;
        for (auto& entry : fs::directory_iterator(Path))
            ++count;

        if (count == 0) {
            *outCount = 0;
            return nullptr;
        }

        // Allocate array of VFile structs
        VFile* files = (VFile*)malloc(sizeof(VFile) * count);
        memset(files, 0, sizeof(VFile) * count);

        size_t i = 0;
        for (auto& entry : fs::directory_iterator(Path))
        {
            VFile* f = &files[i];
            memset(f, 0, sizeof(VFile));

            f->isDirectory = entry.is_directory();

            f->Name = _VIX::Allocation(entry.path().filename().string().c_str());
            f->Extension = _VIX::Allocation(entry.path().extension().string().c_str());
            f->Path = _VIX::Allocation(entry.path().string().c_str());

            if (!f->Name || !f->Extension || !f->Path) {
                perror("malloc failed");
                exit(EXIT_FAILURE);
            }

            // Recursively load subdirectories
            if (f->isDirectory)
            {
                size_t childCount = 0;
                VFile* childArray = parseDirectory_Recursive(f->Path, &childCount);

                f->ChildrenCount = (int)childCount;

                if (childCount > 0)
                {
                    // Allocate dynamic array of VFile* pointers
                    f->Children = (VFile**)malloc(sizeof(VFile*) * childCount);

                    // Fill the pointer array
                    for (size_t c = 0; c < childCount; ++c)
                    {
                        f->Children[c] = &childArray[c];
                    }
                }
                else
                {
                    f->Children = NULL;
                }
            }
            else
            {
                f->Children = NULL;
                f->ChildrenCount = 0;
            }

            ++i;
        }

        *outCount = count;
        return files;
    }

}

extern "C" {
	VIX_EXPORT VFile* ParseDirectory(const char* Path, size_t* FileCount) {
		return VIX::parseDirectory(Path, FileCount);
	}

    VIX_EXPORT VFile* ParseDirectory_Recursive(const char* Path, size_t* FileCount) {
		return VIX::parseDirectory_Recursive(Path, FileCount);
	}
}
