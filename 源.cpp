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
    FileInfo earliest_file; // 最早时间的文件信息
    FileInfo latest_file;   // 最晚时间的文件信息
    uintmax_t total_file_size; // 总的文件大小

    vector<DirectoryInfo*> children; // 子目录
};

void writeToFile(const string& filename, const vector<FileInfo>& files) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& file : files) {
            outFile << file.filename << ","
                << file.path << ","
                << file.file_size << ","
                << file.last_write_time << ","
                << file.depth << endl;
        }
        outFile.close();
    }
}

void writeDirToFile(const string& filename, const vector<DirectoryInfo>& dirs) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& dir : dirs) {
            outFile << dir.name << ","
                << dir.depth << ","
                << dir.file_count << ","
                << dir.parent_directory << endl;
        }
        outFile.close();
    }
}

// 比较两个时间点的先后顺序
bool compareTime(const time_t& time1, const time_t& time2) {
    return difftime(time1, time2) < 0;
}

void traverse(const fs::path& directory, DirectoryInfo* parent, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories) {
    DirectoryInfo dir_info;
    dir_info.name = directory.filename().string();
    dir_info.depth = parent ? parent->depth + 1 : 0; // 计算深度
    dir_info.file_count = 0;
    dir_info.parent_directory = directory.parent_path().string();
    dir_info.total_file_size = 0;
    // 初始化最早时间的文件和最晚时间的文件的信息
    dir_info.earliest_file.last_write_time = std::numeric_limits<time_t>::max();
    dir_info.latest_file.last_write_time = 0;
    DirectoryInfo* current = &dir_info;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_directory(entry)) {
                DirectoryInfo* child = new DirectoryInfo();
                child->name = entry.path().filename().string();
                child->depth = current->depth + 1;
                child->file_count = 0;
                child->parent_directory = directory.string();
                child->total_file_size = 0;
                // 初始化最早时间的文件和最晚时间的文件的信息
                child->earliest_file.last_write_time = std::numeric_limits<time_t>::max();
                child->latest_file.last_write_time = 0;
                traverse(entry.path(), current, file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories);
                current->children.push_back(child);
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
                file_info.depth = current->depth;
                files.push_back(file_info);
                ++file_count;
                // 更新最大深度
                max_depth = max(max_depth, current->depth);
                // 更新深度最深的文件信息
                if (current->depth > deepest_file_depth) {
                    deepest_file_depth = current->depth;
                    deepest_file_path = entry.path().string();
                }
                // 更新目录中的文件数量
                ++current->file_count;

                // 更新最早时间的文件和最晚时间的文件的信息
                if (compareTime(file_info.last_write_time, current->earliest_file.last_write_time)) {
                    current->earliest_file = file_info;
                }
                if (compareTime(current->latest_file.last_write_time, file_info.last_write_time)) {
                    current->latest_file = file_info;
                }

                // 累加总的文件大小
                current->total_file_size += file_info.file_size;
            }
        }
    }
    catch (const std::filesystem::filesystem_error&) {
        // 捕获权限不足的异常，直接忽略
    }

    directories.push_back(dir_info);
    ++dir_count;
}

int main() {
    int file_count = 0;
    int dir_count = 0;
    vector<FileInfo> files;
    int max_depth = 0;
    int deepest_file_depth = 0;
    string deepest_file_path;
    vector<DirectoryInfo> directories;
    cout << "正在扫盘..." << endl;
    traverse("C:\\Windows", nullptr, file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories);
    // 写入文件信息到文件
    writeToFile("D:/myfile.txt", files);
    // 写入目录信息到文件
    writeDirToFile("D:/mydir.txt", directories);
    // 打印扫盘信息
    cout << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;
    cout << "深度最深的文件信息：" << endl;
    cout << "最大深度: " << deepest_file_depth << endl;
    cout << "文件路径及名字: " << deepest_file_path << endl;

    return 0;
}
