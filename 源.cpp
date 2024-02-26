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
    vector<shared_ptr<DirectoryInfo>> children; // ��Ŀ¼�б�
};

// �Ƚ�����ʱ�����Ⱥ�˳��
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

void buildTree(const fs::path& root_directory, vector<shared_ptr<DirectoryInfo>>& nodes) {
    stack<pair<fs::path, shared_ptr<DirectoryInfo>>> dirs;
    dirs.push({ root_directory, nullptr });

    while (!dirs.empty()) {
        fs::path current_directory = dirs.top().first;
        shared_ptr<DirectoryInfo> parent_node = dirs.top().second;
        dirs.pop();

        // ������ǰĿ¼�ڵ�
        shared_ptr<DirectoryInfo> current_node = make_shared<DirectoryInfo>();
        current_node->name = current_directory.filename().string();
        current_node->depth = current_directory.relative_path().string().size();
        current_node->file_count = 0;
        current_node->parent_directory = current_directory.parent_path().string();
        current_node->total_file_size = 0;

        // ��ʼ������ʱ����ļ�������ʱ����ļ�����Ϣ
        current_node->earliest_file.last_write_time = std::numeric_limits<time_t>::max();
        current_node->latest_file.last_write_time = 0;

        // ����и��ڵ㣬����ǰ�ڵ���븸�ڵ�ĺ����б���
        if (parent_node != nullptr) {
            parent_node->children.push_back(current_node);
        }

        nodes.push_back(current_node);

        try {
            for (const auto& entry : fs::directory_iterator(current_directory)) {
                if (fs::is_directory(entry)) {
                    // ����Ŀ¼�ͽڵ����ջ�У�׼��������Ŀ¼
                    dirs.push({ entry.path(), current_node });
                }
                else {
                    // ������ļ������µ�ǰ�ڵ���ļ���Ϣ
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.path = entry.path().string();
                    file_info.file_size = fs::file_size(entry.path());

                    // ��ȡ�ļ�����޸�ʱ�䲢ת��Ϊ time_t ����
                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_point = chrono::time_point_cast<chrono::system_clock::duration>(last_write_time - fs::file_time_type::clock::now() + chrono::system_clock::now());
                    file_info.last_write_time = chrono::system_clock::to_time_t(last_write_time_point);

                    // ���µ�ǰ�ڵ���ļ����������ļ���С
                    current_node->file_count++;
                    current_node->total_file_size += file_info.file_size;

                    // ��������ʱ����ļ�������ʱ����ļ�����Ϣ
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

    // д���ļ���Ϣ���ļ�
    writeToFile("D:/myfile.txt", files);

    // д��Ŀ¼��Ϣ���ļ�
    writeDirToFile("D:/mydir.txt", directories);

    cout << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;

    cout << "���������ļ���Ϣ��" << endl;
    cout << "������: " << deepest_file_depth << endl;
    cout << "�ļ�·��������: " << deepest_file_path << endl;

    // ����Ŀ¼��
    vector<shared_ptr<DirectoryInfo>> nodes;
    buildTree("C:\\Windows", nodes);

    //// ��ӡĿ¼��
    //cout << "Ŀ¼����" << endl;
    //for (const auto& node : nodes) {
    //    cout << "Ŀ¼��: " << node->name << endl;
    //    cout << "���: " << node->depth << endl;
    //    cout << "�ļ�����: " << node->file_count << endl;
    //    cout << "�ܵ��ļ���С: " << node->total_file_size << " �ֽ�" << endl;
    //    cout << "��Ŀ¼: " << node->parent_directory << endl;
    //}

    return 0;
}
