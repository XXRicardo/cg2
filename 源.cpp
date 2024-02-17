#include <iostream>
#include <filesystem>
#include <stack>
#include <system_error>

namespace fs = std::filesystem;
using namespace std;

void traverse(const fs::path& directory) {
    stack<fs::path> directories;
    directories.push(directory);

    while (!directories.empty()) {
        fs::path current_directory = directories.top();
        directories.pop();

        try {
            for (const auto& entry : fs::directory_iterator(current_directory)) {
                if (fs::is_directory(entry)) {
                    directories.push(entry.path());
                }
                else {
                    // ������ļ�������ļ�����·�����ļ���С
                    //cout << "�ļ���: " << entry.path().filename() << endl;
                    //cout << "·��: " << entry.path() << endl;
                    //cout << "�ļ���С: " << fs::file_size(entry.path()) << " �ֽ�" << endl;
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // ����Ȩ�޲�����쳣��ֱ�Ӻ���
        }
    }
}

int main() {
    // �ݹ���� C:\Windows Ŀ¼
    traverse("C:\\Windows");
    return 0;
}
