#include <iostream>
#include <filesystem>
#include <queue>
#include <system_error>

namespace fs = std::filesystem;
using namespace std;

void traverse(const fs::path& directory) {
    queue<fs::path> directories;
    directories.push(directory);

    while (!directories.empty()) {
        fs::path current_directory = directories.front();
        directories.pop();

        for (const auto& entry : fs::directory_iterator(current_directory)) {
            try {
                if (fs::is_directory(entry)) {
                    directories.push(entry.path());
                }
                else {
                    // ������ļ�������ļ�����·�����ļ���С
                    cout << "�ļ���: " << entry.path().filename() << endl;
                    cout << "·��: " << entry.path() << endl;
                    cout << "�ļ���С: " << fs::file_size(entry.path()) << " �ֽ�" << endl;
                }
            }
            catch (const std::filesystem::filesystem_error&) {
                // ����Ȩ�޲�����쳣��ֱ�Ӻ���
            }
        }
    }
}

int main() {
    traverse("C:\\Windows");
    cout << endl;
    return 0;
}
