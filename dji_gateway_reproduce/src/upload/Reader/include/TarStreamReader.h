#ifndef TAR_STREAM_READER_H
#define TAR_STREAM_READER_H

#include <functional>
#include <string>
#include <vector>
#include <cstdint>
#include <sys/stat.h>
#include <map>

#include "filesystem_path.h" 

namespace daf::dins {

namespace filesystem {
class Path;
}

template <typename T, typename E>
class Expected {
public:
    bool HasError() const;
    T& Value();
    const T& Value() const;
    E& Error();
    const E& Error() const;

    static Expected Unexpected(E err);
    static Expected Ok(T val);

private:
    bool has_error_{false};
    T value_;
    E error_;
};

using Buffer = std::vector<char>;

struct FileInfo {//单个文件的信息
    filesystem::Path path;
    std::string name;
    std::uint64_t total_size{0};
    std::uint64_t modify_time{0};
    std::uint64_t content_size{0};
    mode_t mode{0};
};

// tar块头 512字节
struct TarBlockHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
};


class Persistency {//单例
public:
    static Persistency& Instance();
    Expected<int, std::string> OpenFile(const filesystem::Path& path);
    Expected<std::size_t, std::string> Read(int fd, void* buf, std::size_t count);
    Expected<std::size_t, std::string> Seek(int fd, std::size_t offset);
    void Close(int fd);
    Expected<std::size_t, std::string> GetFileSize(const filesystem::Path& path);

private:
    Persistency() = default;
};

// tar流读取
class TarStreamReader {
public:
    TarStreamReader(
        const filesystem::Path& path,
        std::function<Expected<bool, std::string>(const filesystem::Path&)> path_filter);
    ~TarStreamReader() noexcept;

    Expected<std::uint64_t, std::string> TotalSize() const noexcept;
    Expected<void, std::string> Seek(std::uint64_t pos) noexcept;
    Expected<std::uint64_t, std::string> Read(Buffer buf, bool close_fd_after_read) noexcept;

private:
    // RAII 资源守卫类
    class ScopeGuard;

    // 内部辅助方法
    Expected<struct stat, std::string> StatFile(const std::string& path);
    Expected<void, std::string> MakeBlockHeader(const FileInfo& info, TarBlockHeader* header) noexcept;
    Expected<std::size_t, std::string> ReadFromBlockHeader(Buffer buf, const FileInfo& file_info, std::size_t header_offset) noexcept;
    Expected<std::size_t, std::string> ReadFromFileContent(Buffer buf, const FileInfo& file_info, std::size_t file_offset) noexcept;
    Expected<std::size_t, std::string> ReadFromNullBlock(Buffer buf, std::size_t block_offset) noexcept;
    Expected<void, std::string> AddFileInfo(
        const filesystem::Path& path,
        std::size_t pos,
        const std::string& path_prefix_to_remove);
    Expected<std::string, std::string> CheckPathFormat(const filesystem::Path& path);
    void CloseOpenedFile() noexcept;
    Expected<void, std::string> BuildFilesInfo();

    // 工具方法
    static std::string Format(const char* fmt, ...);
    static void LogError(const char* module, const char* func, const char* msg, ...);

private:
    filesystem::Path path_;
    std::function<Expected<bool, std::string>(const filesystem::Path&)> path_filter_;
    std::uint64_t total_size_{0};
    std::map<std::size_t, FileInfo> files_info_map_;
    filesystem::Path opened_file_path_;
    int opened_file_handle_{-1};
    std::uint64_t offset_{0};

    static constexpr std::size_t kBlockSize = 512;
    static constexpr mode_t _S_IFREG = 0100000;
    static constexpr mode_t _S_IFDIR = 0040000;
};

// 资源守卫类实现（内部类）
class TarStreamReader::ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> func);
    ~ScopeGuard();
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> func_;
};

} // namespace daf::dins

#endif // TAR_STREAM_READER_H