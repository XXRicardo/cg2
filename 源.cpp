#include <iostream>
#include <filesystem>
#include <queue>
#include <system_error>
#include <chrono>
#include <ctime>
#include <vector>

namespace fs = std::filesystem;
using namespace std;

// 定义文件信息结构
struct FileInfo {
    string filename;
    string path;
    uintmax_t file_size;
    time_t last_write_time;
};

void traverse(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files) {
    queue<fs::path> directories;
    directories.push(directory);

    while (!directories.empty()) {
        fs::path current_directory = directories.front();
        directories.pop();

        for (const auto& entry : fs::directory_iterator(current_directory)) {
            try {
                if (fs::is_directory(entry)) {
                    directories.push(entry.path());
                    ++dir_count; // 目录计数加一
                }
                else {
                    // 如果是文件，将文件信息存入 FileInfo 结构中
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.path = entry.path().string();
                    file_info.file_size = fs::file_size(entry.path());

                    // 将文件最后修改时间转换为 time_t 类型
                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_point = chrono::time_point_cast<chrono::system_clock::duration>(last_write_time - fs::file_time_type::clock::now() + chrono::system_clock::now());
                    file_info.last_write_time = chrono::system_clock::to_time_t(last_write_time_point);

                    files.push_back(file_info);
                    ++file_count; // 文件计数加一
                }
            }
            catch (const std::filesystem::filesystem_error&) {
                // 捕获权限不足的异常，直接忽略
            }
        }
    }
}

int main() {
    int file_count = 0;
    int dir_count = 0;
    vector<FileInfo> files;
    traverse("C:\\Windows", file_count, dir_count, files);

    cout << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;

    return 0;
}
