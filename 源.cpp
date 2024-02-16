#include <iostream>
#include <filesystem>
#include <queue>
#include <system_error>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;
using namespace std;

void traverse(const fs::path& directory, int& file_count, int& dir_count) {
    queue<fs::path> directories;
    directories.push(directory);

    while (!directories.empty()) {
        fs::path current_directory = directories.front();
        directories.pop();

        for (const auto& entry : fs::directory_iterator(current_directory)) {
            try {
                if (fs::is_directory(entry)) {
                    directories.push(entry.path());
                    ++dir_count; // Ŀ¼������һ
                }
                else {
                    // ������ļ�������ļ�����·�����ļ���С
                    cout << "�ļ���: " << entry.path().filename() << endl;
                    cout << "·��: " << entry.path() << endl;
                    cout << "�ļ���С: " << fs::file_size(entry.path()) << " �ֽ�" << endl;

                    // ��ȡ�ļ�����޸�ʱ�䲢ת��Ϊ�� 1970 �� 1 �� 1 �տ�ʼ�����ڵ�����
                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_seconds = chrono::duration_cast<chrono::seconds>(last_write_time.time_since_epoch()).count();
                    cout << "����޸�ʱ��: " << last_write_time_seconds << " ��" << endl;
                    cout << "*********************************************" << endl;
                    ++file_count; // �ļ�������һ
                }
            }
            catch (const std::filesystem::filesystem_error&) {
                // ����Ȩ�޲�����쳣��ֱ�Ӻ���
            }
        }
    }
}

int main() {
    int file_count = 0;
    int dir_count = 0;
    traverse("C:\\Windows", file_count, dir_count);

    cout << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;

    return 0;
}
