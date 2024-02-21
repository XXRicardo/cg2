#include <iostream>
#include <filesystem>
#include <stack>
#include <ctime>
#include <chrono>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;
using namespace std;

// 定义文件信息结构
struct FileInfo {
    string filename;
    string path;
    uintmax_t file_size;
    time_t last_write_time;
    int depth; // 文件所在目录的深度
};

// 定义目录信息结构
struct DirectoryInfo {
    string name;
    int depth;
    int file_count;
    string parent_directory;
};

void createDirectory(const string& directory) {
    if (!fs::exists(directory)) {
        fs::create_directory(directory);
    }
}

void writeToFile(const string& prefix, const vector<FileInfo>& files, int file_index) {
    createDirectory("D:/myfile"); // 确保文件夹存在

    string filename = "D:/myfile/" + prefix + "_" + to_string(file_index) + ".txt";
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& file : files) {
            outFile << "文件路径: " << file.path << endl;
            outFile << "文件名: " << file.filename << endl;
            outFile << "文件大小: " << file.file_size << " 字节" << endl;
            outFile << "最后修改时间: " << file.last_write_time << endl;
            outFile << "所在目录深度: " << file.depth << endl;
        }
        outFile.close();
    }
}

void writeDirToFile(const string& prefix, const vector<DirectoryInfo>& dirs, int dir_index) {
    createDirectory("D:/mydir"); // 确保文件夹存在

    string filename = "D:/mydir/" + prefix + "_" + to_string(dir_index) + ".txt";
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& dir : dirs) {
            outFile << "目录名: " << dir.name << endl;
            outFile << "深度: " << dir.depth << endl;
            outFile << "文件数量: " << dir.file_count << endl;
            outFile << "上级目录: " << dir.parent_directory << endl;
        }
        outFile.close();
    }
}

void traverse(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories) {
    stack<pair<fs::path, int>> dirs; // pair中的第二项表示目录的深度
    dirs.push({ directory, 0 });

    int file_index = 1;
    int file_limit = 10000; // 每个 txt 文件最多存放的文件数量

    int dir_index = 1;
    int dir_limit = 10000; // 每个 txt 文件最多存放的目录数量

    while (!dirs.empty()) {
        fs::path current_directory = dirs.top().first;
        int current_depth = dirs.top().second;
        dirs.pop();

        DirectoryInfo dir_info;
        dir_info.name = current_directory.filename().string();
        dir_info.depth = current_depth;
        dir_info.file_count = 0;
        dir_info.parent_directory = current_directory.parent_path().string();
        directories.push_back(dir_info);
        ++dir_count;

        try {
            for (const auto& entry : fs::directory_iterator(current_directory)) {
                if (fs::is_directory(entry)) {
                    dirs.push({ entry.path(), current_depth + 1 });
                }
                else {
                    // 如果是文件，将文件信息存入 FileInfo 结构中
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.path = entry.path().string();
                    file_info.file_size = fs::file_size(entry.path());

                    // 获取文件最后修改时间并转换为 time_t 类型
                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_point = chrono::time_point_cast<chrono::system_clock::duration>(last_write_time - fs::file_time_type::clock::now() + chrono::system_clock::now());
                    file_info.last_write_time = chrono::system_clock::to_time_t(last_write_time_point);

                    file_info.depth = current_depth;

                    files.push_back(file_info);
                    ++file_count;

                    // 更新最大深度
                    max_depth = max(max_depth, current_depth);

                    // 更新深度最深的文件信息
                    if (current_depth > deepest_file_depth) {
                        deepest_file_depth = current_depth;
                        deepest_file_path = entry.path().string();
                    }

                    // 更新目录中的文件数量
                    ++directories.back().file_count;

                    // 如果达到每个文件的容量限制，将文件信息写入到 txt 文件中并清空文件信息列表
                    if (file_count % file_limit == 0) {
                        writeToFile("file_info", files, file_index++);
                        files.clear();
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // 捕获权限不足的异常，直接忽略
        }

        // 如果达到每个目录的容量限制，将目录信息写入到 txt 文件中并清空目录信息列表
        if (dir_count % dir_limit == 0) {
            writeDirToFile("dir_info", directories, dir_index++);
            directories.clear();
        }
    }

    // 如果文件信息列表不为空，将剩余的文件信息写入到最后一个 txt 文件中
    if (!files.empty()) {
        writeToFile("file_info", files, file_index);
    }

    // 如果目录信息列表不为空，将剩余的目录信息写入到最后一个 txt 文件中
    if (!directories.empty()) {
        writeDirToFile("dir_info", directories, dir_index);
    }
}

int main() {
    int file_count = 0;
    int dir_count = 0;
    vector<FileInfo> files;
    int max_depth = 0;
    int deepest_file_depth = 0;
    string deepest_file_path;
    vector<DirectoryInfo> directories;
    traverse("C:\\Windows", file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories);

    cout << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;

    cout << "深度最深的文件信息：" << endl;
    cout << "深度: " << deepest_file_depth << endl;
    cout << "文件路径及名字: " << deepest_file_path << endl;

    return 0;
}