#include <string>

namespace arduino { namespace ftp {

    std::string bytes_to_string(size_t bytes) {
        if (bytes < 1024) {
            return std::string(bytes) + "B";
        } else if (bytes < (1024 * 1024)) {
            return std::string(bytes / 1024.0) + "KB";
        } else if (bytes < (1024 * 1024 * 1024)) {
            return std::string(bytes / 1024.0 / 1024.0) + "MB";
        } else {
            return std::string(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
        }
    }

}
