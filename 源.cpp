#include <iostream>
#include <filesystem>
#include <stack>
#include <system_error>

namespace fs = std::filesystem;
using namespace std;

void traverse(const fs::path& directory) {
    stack<fs::path> directories;
    directories.push(directory);

    while (!directories.empty()) {
        fs::path current_directory = directories.top();
        directories.pop();

        try {
            for (const auto& entry : fs::directory_iterator(current_directory)) {
                if (fs::is_directory(entry)) {
                    directories.push(entry.path());
                }
                else {
                    // 如果是文件，输出文件名、路径和文件大小
                    //cout << "文件名: " << entry.path().filename() << endl;
                    //cout << "路径: " << entry.path() << endl;
                    //cout << "文件大小: " << fs::file_size(entry.path()) << " 字节" << endl;
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // 捕获权限不足的异常，直接忽略
        }
    }
}

int main() {
    // 递归遍历 C:\Windows 目录
    traverse("C:\\Windows");
    return 0;
}
