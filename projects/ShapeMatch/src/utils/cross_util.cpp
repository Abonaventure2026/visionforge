#include "cross_util.h"
void PrintLog(const std::string& msg) { std::cout << "[INFO] " << msg << std::endl; }
bool FileExists(const std::string& filePath) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(filePath.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat buf;
    if (stat(filePath.c_str(), &buf) == 0) {
        return S_ISREG(buf.st_mode);
    }
    return false;
#endif
}
