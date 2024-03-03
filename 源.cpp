#include <iostream>
#include <filesystem>
#include <stack>
#include <queue>
#include <ctime>
#include <chrono>
#include <vector>
#include <fstream>
#include <iomanip> 
#include <algorithm> 

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
void writeFile(const string& filename, const vector<FileInfo>& files) {
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
void UI() {
    cout << endl;
    cout << "***********************************************" << endl;
    cout << "* ------------------------------------------- *" << endl;
    cout << "*|    请选择您要执行的功能：                 |*" << endl;
    cout << "*|    1.统计指定目录信息                     |*" << endl;
    cout << "*|    2.模拟文件操作                         |*" << endl;
    cout << "*|    3.模拟目录操作                         |*" << endl;
    cout << "*|    0.退出程序                             |*" << endl;
    cout << "* ------------------------------------------- *" << endl;
    cout << "***********************************************" << endl;
    cout << endl;
}
// 修改后的函数，用于将目录信息写入文件，包括年月日时分秒格式的修改时间
void writeDir(const string& filename, const vector<DirectoryInfo>& dirs) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& dir : dirs) {
            // 格式化最早时间的文件的修改时间（年月日时分秒）
            tm* earliest_time = localtime(&dir.earliest_file.last_write_time);
            stringstream earliest_time_str;
            if (earliest_time) {
                earliest_time_str << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
            }
            else {
                earliest_time_str << "Invalid Time"; // 如果时间无效，则输出错误信息
            }
            // 格式化最晚时间的文件的修改时间（年月日时分秒）
            tm* latest_time = localtime(&dir.latest_file.last_write_time);
            stringstream latest_time_str;
            if (latest_time) {
                latest_time_str << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
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

void buildTree(const fs::path& rootDirectory, DirectoryInfo* root) {
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
                    newDirectory->pathname = entry.path().string() + "\\";
                    newDirectory->depth = currentNode->depth + 1;
                    newDirectory->file_count = 0;
                    newDirectory->parent_directory = currentNode->pathname;
                    newDirectory->total_file_size = 0;
                    newDirectory->earliest_file.last_write_time = std::numeric_limits<time_t>::max(); // 初始化为最大时间
                    newDirectory->latest_file.last_write_time = 0; // 初始化为最小时间

                    if (prevSibling) {
                        newDirectory->td = prevSibling->td + 1; //非常关键
                        prevSibling->right_sibling = newDirectory;
                    }
                    else {
                        newDirectory->td = parent_td + 1;  // 子节点的 td 值比父节点的 td 值增加 1
                        currentNode->left_child = newDirectory;
                    }

                    prevSibling = newDirectory;

                    nodeStack.push({ entry, newDirectory });
                }
                else {
                    ++currentNode->file_count;
                    currentNode->total_file_size += fs::file_size(entry.path());

                    // 更新最早时间的文件和最晚时间的文件的信息
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.path = entry.path().string();
                    file_info.file_size = fs::file_size(entry.path());

                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_point = chrono::time_point_cast<chrono::system_clock::duration>(last_write_time - fs::file_time_type::clock::now() + chrono::system_clock::now());
                    file_info.last_write_time = chrono::system_clock::to_time_t(last_write_time_point);

                    if (compareTime(file_info.last_write_time, currentNode->earliest_file.last_write_time)) {
                        currentNode->earliest_file = file_info;
                    }
                    if (compareTime(currentNode->latest_file.last_write_time, file_info.last_write_time)) {
                        currentNode->latest_file = file_info;
                    }
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

// 用于遍历目录并收集信息
void scan(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories) {
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

void printDir(const string& inputPath, DirectoryInfo* root) {
    string targetPath = inputPath;
    // 找到对应的节点
    DirectoryInfo* targetNode = nullptr;
    stack<DirectoryInfo*> nodeStack;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
        DirectoryInfo* currentNode = nodeStack.top();
        nodeStack.pop();

        if (currentNode->pathname == targetPath) {
            targetNode = currentNode;
            break;
        }

        DirectoryInfo* childNode = currentNode->left_child;
        while (childNode) {
            nodeStack.push(childNode);
            childNode = childNode->right_sibling;
        }
    }

    // 如果找到了目标节点，打印信息
    if (targetNode) {
        cout << "目录搜寻成功" << endl;
        cout << "文件总数为: " << targetNode->file_count << endl;
        cout << "文件总大小: " << targetNode->total_file_size << " 字节" << endl;

        // 打印最早修改时间的文件信息
        tm* earliest_time = std::localtime(&targetNode->earliest_file.last_write_time);
        if (earliest_time) {
            cout << "最早文件信息: " << endl;
            stringstream ss;
            ss << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
            cout << "最后修改时间: " << ss.str() << endl;
            cout << "文件名: " << targetNode->earliest_file.filename << endl;
            cout << "文件大小: " << targetNode->earliest_file.file_size << " 字节" << endl;
        }
        else {
            cout << "最早文件信息获取失败" << endl;
        }

        // 打印最晚修改时间的文件信息
        tm* latest_time = std::localtime(&targetNode->latest_file.last_write_time);
        if (latest_time) {
            cout << "最晚文件信息: " << endl;
            stringstream ss;
            ss << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
            cout << "最后修改时间: " << ss.str() << endl;
            cout << "文件名: " << targetNode->latest_file.filename << endl;
            cout << "文件大小: " << targetNode->latest_file.file_size << " 字节" << endl;
        }
        else {
            cout << "最晚文件信息获取失败" << endl;
        }
    }
    else {
        cout << "未找到目标目录节点！" << endl;
    }
    cout << endl;
}

void printLoop(DirectoryInfo*& root) {
    string inputPath;
    while (true) {
        // 获取用户输入的目录路径
        cout << "请输入带路径的目录名（输入0退出查询）: ";
        cin >> inputPath;

        // 如果用户输入"0"则退出查询
        if (inputPath == "0") {
            cout << "查询结束。" << endl;
            break;
        }

        // 调用函数打印目录信息
        printDir(inputPath, root);
    }
}

void dirop(DirectoryInfo*& root, const string& targetPath) {
    if (!root)
        return;

    queue<DirectoryInfo*> q;
    q.push(root);

    DirectoryInfo* parentNode = nullptr;
    DirectoryInfo* currentNode = nullptr;
    int flag = 0;

    while (!q.empty()) {
        currentNode = q.front();
        q.pop();

        if (currentNode->pathname == targetPath) {
            // 将目录节点的信息写入文件
            ofstream outFile("D:/dirbijiao.txt",ios::app);
            if (outFile.is_open()) {
                outFile << "修改前目录信息: " << endl;
                outFile << "带路径的目录名: " << currentNode->pathname << endl;
                outFile << "文件总数: " << currentNode->file_count << endl;

                outFile << "最早文件信息: " << endl;
                outFile << "文件名: " << currentNode->earliest_file.filename << endl;
                outFile << "文件大小: " << currentNode->earliest_file.file_size << " bytes" << endl;

                tm* earliest_time = std::localtime(&currentNode->earliest_file.last_write_time);
                if (earliest_time) {
                    stringstream se;
                    se << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
                    outFile << "最后修改时间: " << se.str() << endl;
                }
                outFile << "最晚文件信息: " << endl;
                outFile << "文件名: " << currentNode->latest_file.filename << endl;
                outFile << "文件大小: " << currentNode->latest_file.file_size << " bytes" << endl;

                tm* latest_time = std::localtime(&currentNode->latest_file.last_write_time);
                if (latest_time) {
                    stringstream sl;
                    sl << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
                    outFile << "最后修改时间: " << sl.str() << endl;
                }
                outFile << endl;            
            }
            // 删除目标节点以及其子节点
            if (parentNode)
                parentNode->right_sibling = currentNode->right_sibling;
            else
                root = currentNode->right_sibling; // 更新根节点指针

            delete currentNode;
            flag = 1;
            cout << "成功删除目录 " << targetPath << " 及其子目录" << endl;
            cout << endl;
            if (outFile.is_open()) {
                outFile << "修改后未找到目录节点信息！" << endl;
                outFile <<"----------------------------------------" << endl;
            }
            if (outFile.is_open())outFile.close();
            return;
        }

        DirectoryInfo* childNode = currentNode->left_child;
        while (childNode) {
            q.push(childNode);
            childNode = childNode->right_sibling;
        }

        parentNode = currentNode;
    }
    if (!flag) {
        cout << "未找到目标目录节点！" << endl;
    }
}

void fileop(vector<FileInfo>& files, const string& fullFilename, const string& operation, const string& lastWriteTime, const string& size) {
    // 拆分带路径的文件名
    size_t lastBackslashPos = fullFilename.find_last_of("\\");
    string path = fullFilename.substr(0, lastBackslashPos + 1); // 文件路径
    string filename = fullFilename.substr(lastBackslashPos + 1); // 文件名

    ofstream outputFile("D:/filebijiao.txt", ios::app);

    int dep = count(path.begin(), path.end(), '\\');
    vector<FileInfo> matchedFiles;

    // 找到深度相同的文件
    for (const auto& file : files) {
        if (file.depth == dep) {
            matchedFiles.push_back(file);
        }
    }
    // 判断操作类型
    if (operation == "D") {
        // 删除文件
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << endl;
            }
        }
        bool deleted = false;
        for (auto it = files.begin(); it != files.end(); ++it) {
            if (it->depth == dep && it->filename == filename) {
                files.erase(it);
                deleted = true;
                break;
            }
        }
        for (auto mit = matchedFiles.begin(); mit != matchedFiles.end(); ++mit) {
            if ( mit->filename == filename) {
                matchedFiles.erase(mit);
                break;
            }
        }
        if (deleted) {
            cout << "文件删除成功" << endl;
            cout << endl;
        }
        else {
            cout << "文件不存在，删除失败" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile <<"---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }
        
        
    }
    else if (operation == "M") {
        // 修改文件属性
        
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << endl;
            }
        }
        bool modified = false;
        for (auto& file : files) {
            if (file.depth == dep && file.filename == filename) {
                // 修改最后修改时间和大小
                file.last_write_time = stol(lastWriteTime);
                file.file_size = stoul(size);
                modified = true;
                break;
            }
        }
        for (auto& mfile : matchedFiles) {
            if ( mfile.filename == filename) {
                // 修改最后修改时间和大小
                mfile.last_write_time = stol(lastWriteTime);
                mfile.file_size = stoul(size);
                break;
            }
        }
        if (modified) {
            cout << "文件属性修改成功" << endl;
            cout << endl;
        }
        else {
            cout << "文件不存在，修改失败" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << "---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }
        
    }
    else if (operation == "A") {
        // 新增文件
        
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << endl;
            }
        }
        bool exists = false;
        for (const auto& file : matchedFiles) {
            if (file.filename == filename) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            // 文件不存在，执行新增操作
            FileInfo newFile;
            newFile.filename = filename;
            newFile.path = path; // 将路径赋值给文件信息结构体中的path成员
            newFile.depth = dep;
            newFile.last_write_time = stol(lastWriteTime);
            newFile.file_size = stoul(size);
            files.push_back(newFile);
            matchedFiles.push_back(newFile);
            cout << "文件新增成功" << endl;
            cout << endl;
        }
        else {
            cout << "文件已存在，新增失败" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << "---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }
        
        
    }
    else {
        cout << "无效的操作类型" << endl;
        cout << endl;
    }
    if (outputFile.is_open())outputFile.close();
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
    scan("C:\\Windows", file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories); // 扫描目录
    //// 写入文件信息到文件
    //writeFile("D:/myfile.txt", files);
    //// 写入目录信息到文件
    //writeDir("D:/mydir.txt", directories);
    cout << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;
    cout << "深度最深的文件信息：" << endl;
    cout << "最大深度: " << deepest_file_depth << endl;
    cout << "文件路径及名字: " << deepest_file_path << endl;
    cout << endl;

    ofstream filetjt("D:/tongji.txt", ios::trunc);
    if (filetjt.is_open())filetjt.close();
    ofstream filetj("D:/tongji.txt", ios::app);//一共三次统计,第一次
    if (filetj.is_open()) {
        filetj << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;
        filetj << "深度最深的文件信息：" << endl;
        filetj << "最大深度: " << deepest_file_depth << endl;
        filetj << "文件路径及名字: " << deepest_file_path << endl;
        filetj <<"----------------------------------------" << endl;
    }
    if (filetj.is_open())filetj.close();


    cout << "正在建树。" << endl;
    // 构建二叉树
    DirectoryInfo* root = new DirectoryInfo(); // 创建根节点
    root->pathname = "C:\\Windows\\";
    root->depth = 0;
    root->td = 0;
    root->file_count = 0;
    root->parent_directory = "";
    root->total_file_size = 0;
    root->earliest_file.last_write_time = std::numeric_limits<time_t>::max(); // 初始化为最大时间
    root->latest_file.last_write_time = 0; // 初始化为最小时间
    root->left_child = nullptr;
    root->right_sibling = nullptr;
    //buildTree("C:\\Windows", root); // 构建二叉树
    //cout << "二叉树构建完成。" << endl;
    //int maxTd = findMaxTd(root); // 找出所有节点中最大的 td 值
    //cout << "所有节点中最大的 td 值为: " << maxTd<< endl;

    ofstream filet("D:/filebijiao.txt", ios::trunc);
    if (filet.is_open())filet.close();

    ofstream file("D:/dirbijiao.txt", ios::trunc);
    if (file.is_open())file.close();
    
    while (1) {
        UI();
        cout << "请输入指令:";
        int zhiling;
        cin >> zhiling;
        switch (zhiling)
        {
         case 0:
            return 0;
         case 1:
             // 调用循环函数查询目录信息
             printLoop(root);
             break;
         case 2: {
           while(true){
             string input;
             cout << "请输入文件名,操作,时间,大小(按0退出):";
             cin >> input;

             // 如果用户输入"0"则返回功能选择界面
             if (input == "0") {
                 break;
             }

             // 将用户输入的字符串分割成文件名、操作、时间和大小
             stringstream ss(input);
             string filePath, operation, lastWriteTimeStr, sizeStr;
             getline(ss, filePath, ',');
             getline(ss, operation, ',');
             getline(ss, lastWriteTimeStr, ',');
             getline(ss, sizeStr, ',');

             // 调用文件操作函数
             fileop(files, filePath, operation, lastWriteTimeStr, sizeStr);
            }
             break;
         }

         case 3: {
             // 模拟目录操作
             while (true) {
                 string input;
                 cout << "请输入目录名,操作,时间,大小(按0退出):";
                 cin >> input;

                 // 如果用户输入"0"则返回功能选择界面
                 if (input == "0") {
                     break;
                 }

                 // 将用户输入的字符串分割成目录名和操作
                 size_t pos = input.find(',');
                 if (pos != string::npos) {
                     string pathname = input.substr(0, pos);
                     string operation = input.substr(pos + 1);
                     dirop(root, pathname);
                 }
                 else {
                     cout << "输入无效，请重新输入！" << endl;
                 }
             }
             break;
         }

         default:
            cout << "无效指令，请重新输入！" << endl;
            break;
        }

    }
    if (filetj.is_open())filetj.close();
    return 0;
}