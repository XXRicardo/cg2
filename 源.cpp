#include <iostream>
#include <filesystem>
#include <stack>
#include <ctime>
#include <chrono>
#include <vector>
#include <fstream>
#include <iomanip> // ���ͷ�ļ���ʹ�� std::put_time

namespace fs = std::filesystem;
using namespace std;

// �����ļ���Ϣ�ṹ
struct FileInfo {
    string filename;
    string path;
    uintmax_t file_size;
    time_t last_write_time;
    int depth; // �ļ�����Ŀ¼�����
};

// ����Ŀ¼��Ϣ�ṹ
struct DirectoryInfo {
    string name;
    int depth;
    int file_count;
    string parent_directory;
    FileInfo earliest_file; // ����ʱ����ļ���Ϣ
    FileInfo latest_file;   // ����ʱ����ļ���Ϣ
    uintmax_t total_file_size; // �ܵ��ļ���С

    DirectoryInfo* first_child; // ��һ����Ŀ¼
    DirectoryInfo* next_sibling; // ��һ���ֵܽڵ�

    DirectoryInfo() : name(""), depth(0), file_count(0), parent_directory(""), total_file_size(0), first_child(nullptr), next_sibling(nullptr) {}

    DirectoryInfo(const string& n, int d, const string& parent)
        : name(n), depth(d), parent_directory(parent),
        file_count(0), total_file_size(0),
        first_child(nullptr), next_sibling(nullptr) {}
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
            // ��ʽ������ʱ����ļ����޸�ʱ��
            std::tm* earliest_time = std::localtime(&dir.earliest_file.last_write_time);
            std::stringstream earliest_time_str;
            earliest_time_str << std::put_time(earliest_time, "%Y-%m-%d");

            // ��ʽ������ʱ����ļ����޸�ʱ��
            std::tm* latest_time = std::localtime(&dir.latest_file.last_write_time);
            std::stringstream latest_time_str;
            latest_time_str << std::put_time(latest_time, "%Y-%m-%d");

            outFile << dir.name << ","
                << dir.depth << ","
                << dir.file_count << ","
                << dir.total_file_size << ","
                << earliest_time_str.str() << "," // �������ʱ����ļ����޸�ʱ��
                << latest_time_str.str() << ","   // �������ʱ����ļ����޸�ʱ��
                << dir.parent_directory << endl;
        }
        outFile.close();
    }
}

// �Ƚ�����ʱ�����Ⱥ�˳��
bool compareTime(const time_t& time1, const time_t& time2) {
    return difftime(time1, time2) < 0;
}

void buildDirectoryTree(const fs::path& directory, DirectoryInfo* parent) {
    DirectoryInfo* current = parent;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_directory(entry)) {
                DirectoryInfo* child = new DirectoryInfo(entry.path().filename().string(), parent->depth + 1, entry.path().parent_path().string());
                buildDirectoryTree(entry, child);
                if (!current->first_child) {
                    current->first_child = child;
                }
                else {
                    DirectoryInfo* sibling = current->first_child;
                    while (sibling->next_sibling) {
                        sibling = sibling->next_sibling;
                    }
                    sibling->next_sibling = child;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error&) {
        // ����Ȩ�޲�����쳣��ֱ�Ӻ���
    }
}

void traverse(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories) {
    stack<pair<fs::path, int>> dirs; // pair�еĵڶ����ʾĿ¼�����
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
        dir_info.total_file_size = 0; // ��ʼ���ܵ��ļ���СΪ 0

        // ��ʼ������ʱ����ļ�������ʱ����ļ�����Ϣ
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
                    // ������ļ������ļ���Ϣ���� FileInfo �ṹ��
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.path = entry.path().string();
                    file_info.file_size = fs::file_size(entry.path());

                    // ��ȡ�ļ�����޸�ʱ�䲢ת��Ϊ time_t ����
                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_point = chrono::time_point_cast<chrono::system_clock::duration>(last_write_time - fs::file_time_type::clock::now() + chrono::system_clock::now());
                    file_info.last_write_time = chrono::system_clock::to_time_t(last_write_time_point);

                    file_info.depth = current_depth;

                    files.push_back(file_info);
                    ++file_count;

                    // ����������
                    max_depth = max(max_depth, current_depth);

                    // �������������ļ���Ϣ
                    if (current_depth > deepest_file_depth) {
                        deepest_file_depth = current_depth;
                        deepest_file_path = entry.path().string();
                    }

                    // ����Ŀ¼�е��ļ�����
                    ++directories.back().file_count;

                    // ��������ʱ����ļ�������ʱ����ļ�����Ϣ
                    if (compareTime(file_info.last_write_time, directories.back().earliest_file.last_write_time)) {
                        directories.back().earliest_file = file_info;
                    }
                    if (compareTime(directories.back().latest_file.last_write_time, file_info.last_write_time)) {
                        directories.back().latest_file = file_info;
                    }

                    // �ۼ��ܵ��ļ���С
                    directories.back().total_file_size += file_info.file_size;
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // ����Ȩ�޲�����쳣��ֱ�Ӻ���
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
    cout << "����ɨ��..." << endl;
    traverse("C:\\Windows", file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories);

    DirectoryInfo root("C:\\Windows", 0, "");
    buildDirectoryTree("C:\\Windows", &root);

    // д���ļ���Ϣ���ļ�
    writeToFile("D:/myfile.txt", files);

    // д��Ŀ¼��Ϣ���ļ�
    writeDirToFile("D:/mydir.txt", directories);

    cout << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;
    cout << "���������ļ���Ϣ��" << endl;
    cout << "������: " << deepest_file_depth << endl;
    cout << "�ļ�·��������: " << deepest_file_path << endl;

    return 0;
}
