#include <iostream>
#include <filesystem>
#include <stack>
#include <ctime>
#include <chrono>
#include <vector>
#include <fstream>
#include <iomanip> // 添加头文件以使用 std::put_time

namespace fs = std::filesystem;
using namespace std;

// 定义文件信息结构
struct FileInfo {
    string filename;    //文件名
    string path;        //文件路径
    uintmax_t file_size;//文件大小
    time_t last_write_time; //文件最后修改时间
    int depth;          // 文件所在目录的深度
};

// 定义目录信息结构
struct DirectoryInfo {
    string pathname;            //带全路径的目录名
    int depth;              //目录深度
    int file_count;         //目录下的文件数量
    string parent_directory;//父目录名
    FileInfo earliest_file; // 最早修改时间的文件信息
    FileInfo latest_file;   // 最晚修改时间的文件信息
    uintmax_t total_file_size; // 总的文件大小
    int td;                 //树深
    // Binary tree pointers
    DirectoryInfo* left_child;  // 左孩子指针
    DirectoryInfo* right_sibling; // 右兄弟指针

    DirectoryInfo() : left_child(nullptr), right_sibling(nullptr),td(0) {} // 构造函数初始化指针
};

// 原有的函数，用于写入文件信息
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

// 修改后的函数，用于将目录信息写入文件，包括年月日时分秒格式的修改时间
void writeDirToFile(const string& filename, const vector<DirectoryInfo>& dirs) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& dir : dirs) {
            // 格式化最早时间的文件的修改时间（年月日时分秒）
            std::tm* earliest_time = std::localtime(&dir.earliest_file.last_write_time);
            std::stringstream earliest_time_str;
            if (earliest_time) {
                earliest_time_str << std::put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
            }
            else {
                earliest_time_str << "Invalid Time"; // 如果时间无效，则输出错误信息
            }
            // 格式化最晚时间的文件的修改时间（年月日时分秒）
            std::tm* latest_time = std::localtime(&dir.latest_file.last_write_time);
            std::stringstream latest_time_str;
            if (latest_time) {
                latest_time_str << std::put_time(latest_time, "%Y-%m-%d %H:%M:%S");
            }
            else {
                latest_time_str << "Invalid Time"; // 如果时间无效，则输出错误信息
            }
            outFile << dir.pathname << ","
                << dir.depth << ","
                << dir.file_count << ","
                << dir.total_file_size << ","
                << earliest_time_str.str() << "," // 输出最早时间的文件的修改时间
                << latest_time_str.str() << ","   // 输出最晚时间的文件的修改时间
                << dir.parent_directory  << endl;
        }
        outFile.close();
    }
}


// 比较两个时间点的先后顺序
bool compareTime(const time_t& time1, const time_t& time2) {
    return difftime(time1, time2) < 0;
}

void buildBinaryTree(const fs::path& rootDirectory, DirectoryInfo* root) {
    stack<pair<fs::path, DirectoryInfo*>> nodeStack;
    nodeStack.push({ rootDirectory, root });
    while (!nodeStack.empty()) {
        fs::path currentPath = nodeStack.top().first;
        DirectoryInfo* currentNode = nodeStack.top().second;
        nodeStack.pop();
        DirectoryInfo* prevSibling = nullptr;
        int parent_td = currentNode->td; // 记录父节点的 td 值

        try {
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (fs::is_directory(entry)) {
                    DirectoryInfo* newDirectory = new DirectoryInfo();
                    newDirectory->pathname = entry.path().string()+"\\";
                    newDirectory->depth = currentNode->depth + 1;
                    newDirectory->file_count = 0;
                    newDirectory->parent_directory = currentNode->pathname;
                    newDirectory->total_file_size = 0;
                    newDirectory->td = parent_td + 1;  // 子节点的 td 值比父节点的 td 值增加 1

                    if (prevSibling) {
                        newDirectory->td = prevSibling->td + 1;//非常关键
                        prevSibling->right_sibling = newDirectory;
                    }
                    else {
                        currentNode->left_child = newDirectory;
                    }

                    prevSibling = newDirectory;

                    nodeStack.push({ entry, newDirectory });
                }
                else {
                    ++currentNode->file_count;
                    currentNode->total_file_size += fs::file_size(entry.path());
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // 捕获权限不足的异常，直接忽略
        }
    }
}


int findMaxTd(DirectoryInfo* root) {
    if (!root)
        return -1;

    int maxTd = root->td;

    // 使用栈来进行深度优先遍历
    stack<DirectoryInfo*> nodeStack;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
        DirectoryInfo* currentNode = nodeStack.top();
        nodeStack.pop();

        // 检查当前节点的 td 值是否比最大值大
        if (currentNode->td > maxTd)
            maxTd = currentNode->td;

        // 将当前节点的所有子节点压入栈中
        DirectoryInfo* childNode = currentNode->left_child;
        while (childNode) {
            nodeStack.push(childNode);
            childNode = childNode->right_sibling;
        }
    }

    return maxTd*3;
}


// 原有的函数，用于递归遍历目录并收集信息
void traverse(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories) {
    stack<pair<fs::path, int>> dirs; // pair中的第二项表示目录的深度
    dirs.push({ directory, 0 });

    while (!dirs.empty()) {
        fs::path current_directory = dirs.top().first;
        int current_depth = dirs.top().second;
        dirs.pop();

        DirectoryInfo dir_info;
        dir_info.pathname = current_directory.string()+"\\";
        dir_info.depth = current_depth;
        dir_info.file_count = 0;
        dir_info.parent_directory = current_directory.parent_path().string()+"\\";
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

int main() {
    int file_count = 0;
    int dir_count = 0;
    vector<FileInfo> files;
    int max_depth = 0;
    int deepest_file_depth = 0;
    string deepest_file_path;
    vector<DirectoryInfo> directories;

    cout << "正在扫描..." << endl;
    //traverse("C:\\Windows", file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories); // 扫描目录
    //// 写入文件信息到文件
    //writeToFile("D:/myfile.txt", files);
    //// 写入目录信息到文件
    //writeDirToFile("D:/mydir.txt", directories);
    //cout << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;
    //cout << "深度最深的文件信息：" << endl;
    //cout << "最大深度: " << deepest_file_depth << endl;
    //cout << "文件路径及名字: " << deepest_file_path << endl;

    cout << "正在建树。" << endl;
    // 构建二叉树
    DirectoryInfo* root = new DirectoryInfo(); // 创建根节点
    root->pathname = "C:\\Windows\\";
    root->depth = 0;
    root->td = 0;
    root->file_count = 0;
    root->parent_directory = "";
    root->total_file_size = 0;
    root->left_child = nullptr;
    root->right_sibling = nullptr;
    buildBinaryTree("C:\\Windows", root); // 构建二叉树
    cout << "二叉树构建完成。" << endl;
    int maxTd = findMaxTd(root); // 找出所有节点中最大的 td 值
    cout << "所有节点中最大的 td 值为: " << maxTd<< endl;
    return 0;
}