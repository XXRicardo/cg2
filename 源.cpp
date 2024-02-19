#include <iostream>
#include <filesystem>
#include <stack>
#include <ctime>
#include <chrono>
#include <vector>
#include <fstream>

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

void createDirectory(const string& directory) {
    if (!fs::exists(directory)) {
        fs::create_directory(directory);
    }
}

void writeToFile(const string& prefix, const vector<FileInfo>& files, int file_index) {
    createDirectory("D:/myfile"); // ȷ���ļ��д���

    string filename = "D:/myfile/" + prefix + "_" + to_string(file_index) + ".txt";
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& file : files) {
            outFile << "�ļ�·��: " << file.path << endl;
            outFile << "�ļ���: " << file.filename << endl;
            outFile << "�ļ���С: " << file.file_size << " �ֽ�" << endl;
            outFile << "����޸�ʱ��: " << file.last_write_time << endl;
            outFile << "����Ŀ¼���: " << file.depth << endl;
            outFile << "*********************************************" << endl;
        }
        outFile.close();
    }
}

void traverse(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path) {
    stack<pair<fs::path, int>> directories; // pair�еĵڶ����ʾĿ¼�����
    directories.push({ directory, 0 });

    int file_index = 1;
    int file_limit = 10000; // ÿ�� txt �ļ�����ŵ��ļ�����

    while (!directories.empty()) {
        fs::path current_directory = directories.top().first;
        int current_depth = directories.top().second;
        directories.pop();

        try {
            for (const auto& entry : fs::directory_iterator(current_directory)) {
                if (fs::is_directory(entry)) {
                    directories.push({ entry.path(), current_depth + 1 });
                    ++dir_count;
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

                    // ����ﵽÿ���ļ����������ƣ����ļ���Ϣд�뵽 txt �ļ��в�����ļ���Ϣ�б�
                    if (file_count % file_limit == 0) {
                        writeToFile("file_info", files, file_index++);
                        files.clear();
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // ����Ȩ�޲�����쳣��ֱ�Ӻ���
        }
    }

    // ����ļ���Ϣ�б�Ϊ�գ���ʣ����ļ���Ϣд�뵽���һ�� txt �ļ���
    if (!files.empty()) {
        writeToFile("file_info", files, file_index);
    }
}

int main() {
    int file_count = 0;
    int dir_count = 0;
    vector<FileInfo> files;
    int max_depth = 0;
    int deepest_file_depth = 0;
    string deepest_file_path;
    traverse("C:\\Windows", file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path);

    cout << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;

    cout << "���������ļ���Ϣ��" << endl;
    cout << "���: " << deepest_file_depth << endl;
    cout << "�ļ�·��������: " << deepest_file_path << endl;

    return 0;
}
