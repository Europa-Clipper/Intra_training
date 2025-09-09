#include "TarStreamReader.h"

#include <cstdio>
#include <cstdarg>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <memory>
#include <array>

namespace daf::dins {

template <typename T, typename E>
bool Expected<T, E>::HasError() const {
    return has_error_;
}

template <typename T, typename E>
T& Expected<T, E>::Value() {
    return value_;
}

template <typename T, typename E>
const T& Expected<T, E>::Value() const {
    return value_;
}

template <typename T, typename E>
E& Expected<T, E>::Error() {
    return error_;
}

template <typename T, typename E>
const E& Expected<T, E>::Error() const {
    return error_;
}

template <typename T, typename E>
Expected<T, E> Expected<T, E>::Unexpected(E err) {
    Expected e;
    e.has_error_ = true;
    e.error_ = std::move(err);
    return e;
}

template <typename T, typename E>
Expected<T, E> Expected<T, E>::Ok(T val) {
    Expected e;
    e.has_error_ = false;
    e.value_ = std::move(val);
    return e;
}

Persistency& Persistency::Instance() {
    static Persistency inst;
    return inst;
}

Expected<int, std::string> Persistency::OpenFile(const filesystem::Path& path) {
    FILE* fp = fopen(path.Native().c_str(), "rb");
    if (!fp) {
        return Expected<int, std::string>::Unexpected("Failed to open file: " + path.Native());
    }
    return Expected<int, std::string>::Ok(reinterpret_cast<int>(fp));
}

Expected<std::size_t, std::string> Persistency::Read(int fd, void* buf, std::size_t count) {
    FILE* fp = reinterpret_cast<FILE*>(fd);
    std::size_t bytes_read = fread(buf, 1, count, fp);
    if (bytes_read < count && ferror(fp)) {
        return Expected<std::size_t, std::string>::Unexpected("Read error");
    }
    return Expected<std::size_t, std::string>::Ok(bytes_read);
}

Expected<std::size_t, std::string> Persistency::Seek(int fd, std::size_t offset) {
    FILE* fp = reinterpret_cast<FILE*>(fd);
    if (fseek(fp, static_cast<long>(offset), SEEK_SET) != 0) {
        return Expected<std::size_t, std::string>::Unexpected("Seek error");
    }
    return Expected<std::size_t, std::string>::Ok(offset);
}

void Persistency::Close(int fd) {
    FILE* fp = reinterpret_cast<FILE*>(fd);
    fclose(fp);
}

Expected<std::size_t, std::string> Persistency::GetFileSize(const filesystem::Path& path) {
    struct stat st;
    if (stat(path.Native().c_str(), &st) != 0) {
        return Expected<std::size_t, std::string>::Unexpected("Stat error");
    }
    return Expected<std::size_t, std::string>::Ok(static_cast<std::size_t>(st.st_size));
}

TarStreamReader::ScopeGuard::ScopeGuard(std::function<void()> func) : func_(std::move(func)) {}

TarStreamReader::ScopeGuard::~ScopeGuard() {
    func_();
}

TarStreamReader::TarStreamReader(
    const filesystem::Path& path,
    std::function<Expected<bool, std::string>(const filesystem::Path&)> path_filter)
    : path_(path), path_filter_(std::move(path_filter)) {
    auto result = BuildFilesInfo();
    if (result.HasError()) {
        throw std::runtime_error(result.Error());
    }
}

TarStreamReader::~TarStreamReader() noexcept {
    CloseOpenedFile();
}

Expected<std::uint64_t, std::string> TarStreamReader::TotalSize() const noexcept {
    return Expected<std::uint64_t, std::string>::Ok(total_size_);
}

Expected<void, std::string> TarStreamReader::Seek(std::uint64_t pos) noexcept {
    Expected<void, std::string> finally_result = Expected<void, std::string>::Ok({});
    auto finally = ScopeGuard([this, &finally_result]() {
        if (finally_result.HasError()) {
            CloseOpenedFile();
        }
    });

    if (total_size_ < pos) {
        finally_result = Expected<void, std::string>::Unexpected(
            Format("TarStreamReader::Seek: total_size_({}) < pos({})", total_size_, pos));
        return finally_result;
    }

    offset_ = pos;
    return finally_result;
}

Expected<std::uint64_t, std::string> TarStreamReader::Read(Buffer buf, bool close_fd_after_read) noexcept {
    if (buf.empty()) {
        return Expected<std::uint64_t, std::string>::Ok(offset_);
    }

    Expected<std::uint64_t, std::string> finally_result;
    auto finally = ScopeGuard([this, &finally_result, close_fd_after_read]() {
        if (finally_result.HasError()) {
            CloseOpenedFile();
        }
        if (close_fd_after_read) {
            CloseOpenedFile();
        }
    });

    try {
        auto iter = files_info_map_.upper_bound(offset_);
        if (iter != files_info_map_.begin()) --iter;

        std::size_t size_of_read = 0;
        std::size_t size_to_read = buf.size();

        auto AddSize = [this, &size_of_read, &size_to_read](std::size_t size) {
            size_of_read += size;
            size_to_read -= size;
            offset_ += size;
        };

        do {
            auto& file_info = iter->second;
            auto& begin_pos = iter->first;

            if (offset_ - begin_pos < kBlockSize) {
                auto result = ReadFromBlockHeader(
                    //Buffer(buf.data() + size_of_read, size_to_read),
                    Buffer(buf.data() + size_of_read, buf.data() + size_of_read + size_to_read),
                    file_info, offset_ - begin_pos);
                if (result.HasError()) {
                    finally_result = Expected<std::uint64_t, std::string>::Unexpected(result.Error());
                    return finally_result;
                }
                AddSize(result.Value());
                if (size_to_read == 0) break;
            }

            if ((file_info.mode >> 12 << 12) == S_IFREG && offset_ - begin_pos < file_info.total_size) {
                std::size_t file_offset = offset_ - begin_pos - kBlockSize;
                auto result = ReadFromFileContent(
                    //Buffer(buf.data() + size_of_read, size_to_read),
                    Buffer(buf.data() + size_of_read, buf.data() + size_of_read + size_to_read),
                    file_info, file_offset);
                if (result.HasError()) {
                    finally_result = Expected<std::uint64_t, std::string>::Unexpected(result.Error());
                    return finally_result;
                }
                AddSize(result.Value());
                if (size_to_read == 0) break;
            }

            if (++iter == files_info_map_.end()) {
                auto result = ReadFromNullBlock(
                    //Buffer(buf.data() + size_of_read, size_to_read),
                    Buffer(buf.data() + size_of_read, buf.data() + size_of_read + size_to_read),
                    offset_ - begin_pos - file_info.total_size);
                if (result.HasError()) {
                    finally_result = Expected<std::uint64_t, std::string>::Unexpected(result.Error());
                    return finally_result;
                }
                AddSize(result.Value());
                break;
            }
        } while (size_to_read > 0);

        finally_result = Expected<std::uint64_t, std::string>::Ok(size_of_read);
        return finally_result;
    } catch (const std::exception& e) {
        LogError("Persistency", __func__, "exception: {}", e.what());
        finally_result = Expected<std::uint64_t, std::string>::Unexpected("exception occurred");
        return finally_result;
    }
}

Expected<struct stat, std::string> TarStreamReader::StatFile(const std::string& path) {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return Expected<struct stat, std::string>::Unexpected(std::string(strerror(errno)));
    }
    return Expected<struct stat, std::string>::Ok(st);
}

Expected<void, std::string> TarStreamReader::MakeBlockHeader(const FileInfo& info, TarBlockHeader* header) noexcept {
    try {
        memset(header, 0, sizeof(TarBlockHeader));

        strncpy(header->name, info.name.c_str(), sizeof(header->name) - 1);
        snprintf(header->mode, sizeof(header->mode), "%07o", info.mode);
        snprintf(header->uid, sizeof(header->uid), "%06o", 0);
        snprintf(header->gid, sizeof(header->gid), "%06o", 0);
        snprintf(header->size, sizeof(header->size), "%011o", info.content_size);
        snprintf(header->mtime, sizeof(header->mtime), "%011o", info.modify_time);
        memset(header->chksum, ' ', sizeof(header->chksum));

        header->typeflag = (info.mode & S_IFREG) ? '0' : '5';

        size_t chksum = 0;
        for (const auto& byte : *reinterpret_cast<const std::array<char, sizeof(TarBlockHeader)>*>(header)) {
            chksum += static_cast<unsigned char>(byte);
        }
        snprintf(header->chksum, sizeof(header->chksum), "%06o", chksum);
        header->chksum[6] = '\0';

        return Expected<void, std::string>::Ok({});
    } catch (const std::exception& e) {
        return Expected<void, std::string>::Unexpected(
            Format("MakeBlockHeader exception: {}", e.what()));
    }
}

Expected<std::size_t, std::string> TarStreamReader::ReadFromBlockHeader(Buffer buf, const FileInfo& file_info, std::size_t header_offset) noexcept {
    TarBlockHeader header;
    auto result = MakeBlockHeader(file_info, &header);
    if (result.HasError()) {
        return Expected<std::size_t, std::string>::Unexpected(result.Error());
    }

    std::size_t size = std::min(buf.size(), kBlockSize - header_offset);
    memcpy(buf.data(), reinterpret_cast<char*>(&header) + header_offset, size);
    return Expected<std::size_t, std::string>::Ok(size);
}

Expected<std::size_t, std::string> TarStreamReader::ReadFromFileContent(Buffer buf, const FileInfo& file_info, std::size_t file_offset) noexcept {
    if (file_offset >= file_info.total_size) {
        return Expected<std::size_t, std::string>::Unexpected("file_offset out of range");
    }

    filesystem::Path opened_file_path;
    int opened_file_handle = -1;
    if (!opened_file_path_.empty() && opened_file_path_ != file_info.path) {
        CloseOpenedFile();
        auto open_result = Persistency::Instance().OpenFile(file_info.path);
        if (open_result.HasError()) {
            return Expected<std::size_t, std::string>::Unexpected(
                Format("OpenFile failed: {}", open_result.Error()));
        }
        opened_file_handle_ = open_result.Value();
        opened_file_path_ = file_info.path;
    }

    auto seek_result = Persistency::Instance().Seek(opened_file_handle_, file_offset);
    if (seek_result.HasError()) {
        return Expected<std::size_t, std::string>::Unexpected(seek_result.Error());
    }

    std::size_t size = std::min(buf.size(), file_info.content_size - file_offset);
    std::size_t size_of_file_read = 0;
    while (size_of_file_read < size) {
        auto read_result = Persistency::Instance().Read(
            opened_file_handle_,
            buf.data() + size_of_file_read,
            size - size_of_file_read);
        if (read_result.HasError()) {
            return Expected<std::size_t, std::string>::Unexpected(
                Format("Read failed: {}", read_result.Error()));
        }
        std::size_t read_size = read_result.Value();
        if (read_size == 0) break;
        size_of_file_read += read_size;
    }

    if (size_of_file_read < size) {
        return Expected<std::size_t, std::string>::Unexpected("read less than expected");
    }

    if (buf.size() > size_of_file_read) {
        std::size_t fill_size = std::min(
            buf.size() - size_of_file_read,
            file_info.total_size - kBlockSize - file_info.content_size);
        memset(buf.data() + size_of_file_read, 0, fill_size);
        size_of_file_read += fill_size;
    }

    return Expected<std::size_t, std::string>::Ok(size_of_file_read);
}

Expected<std::size_t, std::string> TarStreamReader::ReadFromNullBlock(Buffer buf, std::size_t block_offset) noexcept {
    if (block_offset >= kBlockSize) {
        return Expected<std::size_t, std::string>::Unexpected("block_offset out of range");
    }
    std::size_t size = std::min(buf.size(), kBlockSize - block_offset);
    memset(buf.data(), 0, size);
    return Expected<std::size_t, std::string>::Ok(size);
}

Expected<void, std::string> TarStreamReader::AddFileInfo(
    const filesystem::Path& path,
    std::size_t pos,
    const std::string& path_prefix_to_remove) {
    auto stat_result = StatFile(path.Native());
    if (stat_result.HasError()) {
        return Expected<void, std::string>::Unexpected(
            Format("StatFile failed: {}", stat_result.Error()));
    }

    std::size_t header_size = kBlockSize;
    std::size_t file_total_size = 0;
    std::size_t file_content_size = 0;

    if ((stat_result.Value().st_mode >> 12 << 12) == S_IFREG) {
        auto size_result = Persistency::Instance().GetFileSize(path);
        if (size_result.HasError() || size_result.Value() < 0) {
            return Expected<void, std::string>::Unexpected("GetFileSize failed");
        }
        file_content_size = size_result.Value();
        std::size_t aligned_blocks = (file_content_size / kBlockSize) + 
                                    ((file_content_size % kBlockSize) != 0 ? 1 : 0);
        file_total_size = header_size + aligned_blocks * kBlockSize;
    } else if ((stat_result.Value().st_mode >> 12 << 12) == S_IFDIR) {
        file_total_size = header_size;
    } else {
        return Expected<void, std::string>::Ok({});
    }

    total_size_ += file_total_size;

    auto& file_info = files_info_map_[pos];
    std::string sub_path = path.Native().substr(path_prefix_to_remove.size());
    file_info.path = path;
    file_info.name = std::move(sub_path);
    file_info.total_size = file_total_size;
    file_info.modify_time = stat_result.Value().st_mtime;
    file_info.content_size = file_content_size;
    file_info.mode = stat_result.Value().st_mode;

    return Expected<void, std::string>::Ok({});
}

Expected<std::string, std::string> TarStreamReader::CheckPathFormat(const filesystem::Path& path) {
    if (path.IsRelative()) {
        return Expected<std::string, std::string>::Unexpected(
            Format("path is relative: {}", path.Native()));
    }
    if (!path.HasFilename()) {
        return Expected<std::string, std::string>::Unexpected(
            Format("path has no filename: {}", path.Native()));
    } else if (path.Filename() == "." || path.Filename() == "..") {
        return Expected<std::string, std::string>::Unexpected(
            Format("invalid filename: {}", path.Native()));
    }

    auto stat_result = StatFile(path.Native());
    if (stat_result.HasError()) {
        return Expected<std::string, std::string>::Unexpected(
            Format("StatFile failed: {}", stat_result.Error()));
    }

    if ((stat_result.Value().st_mode >> 12 << 12) == S_IFREG) {
        return Expected<std::string, std::string>::Ok(path.ParentPath().Native() + "/");
    } else if ((stat_result.Value().st_mode >> 12 << 12) == S_IFDIR) {
        return Expected<std::string, std::string>::Ok(path.Native() + "/");
    } else {
        return Expected<std::string, std::string>::Unexpected(
            Format("invalid file type: {}", stat_result.Value().st_mode));
    }
}

void TarStreamReader::CloseOpenedFile() noexcept {
    if (!opened_file_path_.empty() && opened_file_handle_ != -1) {
        Persistency::Instance().Close(opened_file_handle_);
        opened_file_path_ = filesystem::Path();
        opened_file_handle_ = -1;
    }
}

Expected<void, std::string> TarStreamReader::BuildFilesInfo() {
    //打开 tar 文件（通过 Persistency 单例）
    auto open_result = Persistency::Instance().OpenFile(path_);
    if (open_result.HasError()) {
        return Expected<void, std::string>::Unexpected(
            Format("Failed to open tar file: {}", open_result.Error()));
    }
    int fd = open_result.Value();

    // 2. 使用 ScopeGuard 确保文件句柄必然关闭（替代 defer）
    auto close_guard = ScopeGuard([fd]() {
        Persistency::Instance().Close(fd);
    });

    // 3. 循环读取 tar 归档中的块头（每个块头 512 字节）
    TarBlockHeader header;
    std::uint64_t current_offset = 0; // 记录当前在 tar 中的偏移量

    while (true) {
        // 读取 512 字节块头
        auto read_header_result = Persistency::Instance().Read(fd, &header, kBlockSize);
        if (read_header_result.HasError()) {
            return Expected<void, std::string>::Unexpected(
                Format("Failed to read tar header: {}", read_header_result.Error()));
        }

        // 若读取字节数小于块大小，说明到达文件末尾（或 tar 结束）
        std::size_t read_bytes = read_header_result.Value();
        if (read_bytes < kBlockSize) {
            // 检查是否为 tar 结束标识（两个连续空块）
            bool is_end = (read_bytes == 0);
            if (!is_end) {
                // 部分读取，判断是否为全零块
                std::array<char, kBlockSize> zero_buf{};
                is_end = (memcmp(&header, zero_buf.data(), read_bytes) == 0);
            }
            if (is_end) break;

            return Expected<void, std::string>::Unexpected(
                "Invalid tar format: incomplete header");
        }

        // 4. 解析块头中的文件名（tar 规范：name 字段以 null 结尾）
        std::string file_name(header.name);
        if (file_name.empty()) {
            // 空文件名可能是填充块，跳过
            current_offset += kBlockSize;
            continue;
        }

        // 5. 构建文件路径并校验格式
        filesystem::Path file_path(file_name);
        auto check_result = CheckPathFormat(file_path);
        if (check_result.HasError()) {
            return Expected<void, std::string>::Unexpected(
                Format("Invalid file path in tar: {}", check_result.Error()));
        }

        // 6. 解析文件大小（tar 中 size 字段为八进制字符串）
        std::uint64_t content_size = 0;
        try {
            content_size = std::stoull(header.size, nullptr, 8); // 八进制转整数
        } catch (const std::exception& e) {
            return Expected<void, std::string>::Unexpected(
                Format("Invalid file size in tar header: {}", e.what()));
        }

        // 7. 计算文件总大小（内容大小 + 块对齐填充，确保是 512 倍数）
        std::uint64_t total_size = kBlockSize + ((content_size + kBlockSize - 1) / kBlockSize) * kBlockSize;

        // 8. 添加文件信息到映射表（current_offset 为文件在 tar 中的起始偏移量）
        auto add_result = AddFileInfo(file_path, current_offset, check_result.Value());
        if (add_result.HasError()) {
            return add_result;
        }

        // 9. 移动偏移量到下一个文件的块头
        current_offset += total_size;

        // 10. 定位文件指针到下一个块头（跳过当前文件内容）
        auto seek_result = Persistency::Instance().Seek(fd, current_offset);
        if (seek_result.HasError()) {
            return Expected<void, std::string>::Unexpected(
                Format("Failed to seek in tar file: {}", seek_result.Error()));
        }
    }

    return Expected<void, std::string>::Ok({});
}


void TarStreamReader::LogError(const char* module, const char* func, const char* msg, ...) {
    va_list args;
    va_start(args, msg);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), msg, args);
    va_end(args);
    fprintf(stderr, "[ERROR] %s::%s: %s\n", module, func, buf);
}

} // namespace daf::dins