#include <iostream>
#include <filesystem>
#include <string>
#include <system_error>

namespace fs = std::filesystem;
using namespace std;

void traverse(const fs::path& directory) {
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            try {
                if (fs::is_directory(entry)) {
                    // �����Ŀ¼���ݹ������Ŀ¼
                    traverse(entry.path());
                }
                else {
                    // ������ļ�������ļ�����·�����ļ���С
                    cout << "�ļ���: " << entry.path().filename() << endl;
                    cout << "·��: " << entry.path() << endl;
                    cout << "�ļ���С: " << fs::file_size(entry.path()) << " �ֽ�" << endl;
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                // ����Ȩ�޲�����쳣���������ļ����ļ���
                cerr << "Error: " << e.what() << endl;
                cerr << "Skipping directory: " << entry.path() << endl;
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // ����Ȩ�޲�����쳣��������Ŀ¼
        cerr << "Error: " << e.what() << endl;
        cerr << "Skipping directory: " << directory << endl;
    }
}

int main() {
    // �ݹ���� C:\Windows Ŀ¼
    traverse("C:\\Windows");
    return 0;
}
