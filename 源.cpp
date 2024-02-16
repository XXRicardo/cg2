#include <iostream>
#include <filesystem>
#include <string>
#include <system_error>

namespace fs = std::filesystem;
using namespace std;

void traverse(const fs::path& directory) {
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            try {
                if (fs::is_directory(entry)) {
                    // 如果是目录，递归遍历子目录
                    traverse(entry.path());
                }
                else {
                    // 如果是文件，输出文件名、路径和文件大小
                    cout << "文件名: " << entry.path().filename() << endl;
                    cout << "路径: " << entry.path() << endl;
                    cout << "文件大小: " << fs::file_size(entry.path()) << " 字节" << endl;
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                // 捕获权限不足的异常，跳过该文件或文件夹
                cerr << "Error: " << e.what() << endl;
                cerr << "Skipping directory: " << entry.path() << endl;
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // 捕获权限不足的异常，跳过该目录
        cerr << "Error: " << e.what() << endl;
        cerr << "Skipping directory: " << directory << endl;
    }
}

int main() {
    // 递归遍历 C:\Windows 目录
    traverse("C:\\Windows");
    return 0;
}
