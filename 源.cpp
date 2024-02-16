#include <iostream>
#include <filesystem>
#include <queue>
#include <system_error>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;
using namespace std;

void traverse(const fs::path& directory, int& file_count, int& dir_count) {
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
                    // 如果是文件，输出文件名、路径和文件大小
                    cout << "文件名: " << entry.path().filename() << endl;
                    cout << "路径: " << entry.path() << endl;
                    cout << "文件大小: " << fs::file_size(entry.path()) << " 字节" << endl;

                    // 获取文件最后修改时间并转换为从 1970 年 1 月 1 日开始到现在的秒数
                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_seconds = chrono::duration_cast<chrono::seconds>(last_write_time.time_since_epoch()).count();
                    cout << "最后修改时间: " << last_write_time_seconds << " 秒" << endl;
                    cout << "*********************************************" << endl;
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
    traverse("C:\\Windows", file_count, dir_count);

    cout << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;

    return 0;
}
