#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <sys/stat.h>

namespace daf::dins::filesystem {

class Path {
public:
    Path() = default;
    explicit Path(const std::string& native_path) : native_path_(native_path) {
        Normalize();
    }

    // 假设已有 == 运算符：
    bool operator==(const Path& other) const {
        return Native() == other.Native(); // 比较路径的字符串表示
    }

    // 添加 != 运算符，基于 == 实现
    bool operator!=(const Path& other) const {
        return !(*this == other);
    }

    bool IsRelative() const {
        return !native_path_.empty() && native_path_[0] != '/';
    }

    bool HasFilename() const {
        if (native_path_.empty()) return false;
        size_t last_sep = native_path_.find_last_of('/');
        return last_sep != std::string::npos && last_sep < native_path_.size() - 1;
    }

    std::string Filename() const {
        if (!HasFilename()) return "";
        size_t last_sep = native_path_.find_last_of('/');
        return native_path_.substr(last_sep + 1);
    }

    filesystem::Path ParentPath() const {
        if (native_path_.empty()) return filesystem::Path("");
        size_t last_sep = native_path_.find_last_of('/');
        if (last_sep == 0) return filesystem::Path("/");
        if (last_sep == std::string::npos) return filesystem::Path("");
        return filesystem::Path(native_path_.substr(0, last_sep));
    }

    std::string Native() const {
        return native_path_;
    }

    size_t size() const {
        return native_path_.size();
    }

    bool empty() const {
        return native_path_.empty();
    }

private:
    void Normalize() {
        std::string normalized;
        for (char c : native_path_) {
            if (c == '/' && !normalized.empty() && normalized.back() == '/') {
                continue;
            }
            normalized += c;
        }
        if (normalized.size() > 1 && normalized.back() == '/') {
            normalized.pop_back();
        }
        native_path_ = normalized;
    }

    std::string native_path_;
};

} // namespace daf::dins::filesystem