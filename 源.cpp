#include <iostream>
#include <filesystem>
#include <string>
#include <queue>
#include <system_error>

namespace fs = std::filesystem;
using namespace std;

void traverse(const fs::path& directory) {
    // ʹ�ö��д洢�������Ŀ¼
    queue<fs::path> directories;
    directories.push(directory);

    while (!directories.empty()) {
        fs::path current_directory = directories.front();
        directories.pop();

        try {
            // ������ǰĿ¼�µ�������Ŀ
            for (const auto& entry : fs::directory_iterator(current_directory)) {
                if (fs::is_directory(entry)) {
                    // �����Ŀ¼���������������Ŀ¼������
                    directories.push(entry.path());
                }
                else {
                    // ������ļ�������ļ�����·�����ļ���С
                    cout << "�ļ���: " << entry.path().filename() << endl;
                    cout << "·��: " << entry.path() << endl;
                    cout << "�ļ���С: " << fs::file_size(entry.path()) << " �ֽ�" << endl;
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            // ����Ȩ�޲�����쳣��������Ŀ¼
            cerr << "Error: " << e.what() << endl;
            cerr << "Skipping directory: " << current_directory << endl;
        }
    }
}

int main() {
    // �ݹ���� C:\Windows Ŀ¼
    traverse("C:\\Windows");
    return 0;
}
