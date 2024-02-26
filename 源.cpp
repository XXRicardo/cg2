#include <iostream>
#include <filesystem>
#include <stack>
#include <ctime>
#include <chrono>
#include <vector>
#include <fstream>
#include <memory>

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
    vector<shared_ptr<DirectoryInfo>> children; // 子目录列表
};

// 比较两个时间点的先后顺序
bool compareTime(const time_t& time1, const time_t& time2) {
    return difftime(time1, time2) < 0;
}

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
                << dir.total_file_size << ","
                << dir.earliest_file.filename << ","
                << dir.earliest_file.last_write_time << ","
                << dir.parent_directory << endl;
        }
        outFile.close();
    }
}

void traverse(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories) {
    stack<pair<fs::path, int>> dirs; // pair中的第二项表示目录的深度
    dirs.push({ directory, 0 });

    while (!dirs.empty()) {
        fs::path current_directory = dirs.top().first;
        int current_depth = dirs.top().second;
        dirs.pop();

        DirectoryInfo dir_info;
        dir_info.name = current_directory.filename().string();
        dir_info.depth = current_depth;
        dir_info.file_count = 0;
        dir_info.parent_directory = current_directory.parent_path().string();
        dir_info.total_file_size = 0; // 初始化总的文件大小为 0

        // 初始化最早时间的文件和最晚时间的文件的信息
        dir_info.earliest_file.last_write_time = std::numeric_limits<time_t>::max();
        dir_info.latest_file.last_write_time = 0;

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

                    // 更新最早时间的文件和最晚时间的文件的信息
                    if (compareTime(file_info.last_write_time, directories.back().earliest_file.last_write_time)) {
                        directories.back().earliest_file = file_info;
                    }
                    if (compareTime(directories.back().latest_file.last_write_time, file_info.last_write_time)) {
                        directories.back().latest_file = file_info;
                    }

                    // 累加总的文件大小
                    directories.back().total_file_size += file_info.file_size;
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // 捕获权限不足的异常，直接忽略
        }
    }
}

void buildTree(const fs::path& root_directory, vector<shared_ptr<DirectoryInfo>>& nodes) {
    stack<pair<fs::path, shared_ptr<DirectoryInfo>>> dirs;
    dirs.push({ root_directory, nullptr });

    while (!dirs.empty()) {
        fs::path current_directory = dirs.top().first;
        shared_ptr<DirectoryInfo> parent_node = dirs.top().second;
        dirs.pop();

        // 创建当前目录节点
        shared_ptr<DirectoryInfo> current_node = make_shared<DirectoryInfo>();
        current_node->name = current_directory.filename().string();
        current_node->depth = current_directory.relative_path().string().size();
        current_node->file_count = 0;
        current_node->parent_directory = current_directory.parent_path().string();
        current_node->total_file_size = 0;

        // 初始化最早时间的文件和最晚时间的文件的信息
        current_node->earliest_file.last_write_time = std::numeric_limits<time_t>::max();
        current_node->latest_file.last_write_time = 0;

        // 如果有父节点，将当前节点加入父节点的孩子列表中
        if (parent_node != nullptr) {
            parent_node->children.push_back(current_node);
        }

        nodes.push_back(current_node);

        try {
            for (const auto& entry : fs::directory_iterator(current_directory)) {
                if (fs::is_directory(entry)) {
                    // 将子目录和节点加入栈中，准备处理子目录
                    dirs.push({ entry.path(), current_node });
                }
                else {
                    // 如果是文件，更新当前节点的文件信息
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.path = entry.path().string();
                    file_info.file_size = fs::file_size(entry.path());

                    // 获取文件最后修改时间并转换为 time_t 类型
                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_point = chrono::time_point_cast<chrono::system_clock::duration>(last_write_time - fs::file_time_type::clock::now() + chrono::system_clock::now());
                    file_info.last_write_time = chrono::system_clock::to_time_t(last_write_time_point);

                    // 更新当前节点的文件数量和总文件大小
                    current_node->file_count++;
                    current_node->total_file_size += file_info.file_size;

                    // 更新最早时间的文件和最晚时间的文件的信息
                    if (compareTime(file_info.last_write_time, current_node->earliest_file.last_write_time)) {
                        current_node->earliest_file = file_info;
                    }
                    if (compareTime(current_node->latest_file.last_write_time, file_info.last_write_time)) {
                        current_node->latest_file = file_info;
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // 捕获权限不足的异常，直接忽略
        }
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
    cout << "正在扫描..." << endl;
    traverse("C:\\Windows", file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories);

    // 写入文件信息到文件
    writeToFile("D:/myfile.txt", files);

    // 写入目录信息到文件
    writeDirToFile("D:/mydir.txt", directories);

    cout << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;

    cout << "深度最深的文件信息：" << endl;
    cout << "最大深度: " << deepest_file_depth << endl;
    cout << "文件路径及名字: " << deepest_file_path << endl;

    // 构建目录树
    vector<shared_ptr<DirectoryInfo>> nodes;
    buildTree("C:\\Windows", nodes);

    //// 打印目录树
    //cout << "目录树：" << endl;
    //for (const auto& node : nodes) {
    //    cout << "目录名: " << node->name << endl;
    //    cout << "深度: " << node->depth << endl;
    //    cout << "文件数量: " << node->file_count << endl;
    //    cout << "总的文件大小: " << node->total_file_size << " 字节" << endl;
    //    cout << "父目录: " << node->parent_directory << endl;
    //}

    return 0;
}
