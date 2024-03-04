#pragma once
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

    DirectoryInfo() : left_child(nullptr), right_sibling(nullptr), td(0) {} // 构造函数初始化指针
};
// 用于写入文件信息
void writeFile(const string& filename, const vector<FileInfo>& files);

void UI();

// 用于将目录信息写入文件
void writeDir(const string& filename, const vector<DirectoryInfo>& dirs);

// 比较两个时间点的先后顺序
bool compareTime(const time_t& time1, const time_t& time2);

//构建孩子兄弟树
void buildTree(const fs::path& rootDirectory, DirectoryInfo* root);

//用于寻找最大树深
int findMaxTd(DirectoryInfo* root);

// 用于遍历目录并收集信息
void scan(const fs::path& directory, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories, int& file_count, int& dir_count);

//用于统计某个目录的信息,根据输入的目录路径查找对应的目录节点并打印信息
void printDir(const string& inputPath, DirectoryInfo* root);

// 循环读取用户输入的目录路径，并调用打印目录信息函数
void printLoop(DirectoryInfo*& root);

//模拟目录操作：在孩子兄弟树中查找并删除目标目录及其子目录
void dirop(DirectoryInfo*& root, const string& targetPath, int& file_count, int& dir_count);

// 模拟文件操作：删除、修改和新增文件
void fileop(vector<FileInfo>& files, const string& fullFilename, const string& operation, const string& lastWriteTime, const string& size, int& file_count);