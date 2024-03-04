#include "func.h"

// ����д���ļ���Ϣ
void writeFile(const string& filename, const vector<FileInfo>& files) {
    // �����ļ���Ϣ����ÿ���ļ��������Ϣд���ļ�
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
    cout << "*|    ��ѡ����Ҫִ�еĹ��ܣ�                 |*" << endl;
    cout << "*|    1.ͳ��ָ��Ŀ¼��Ϣ                     |*" << endl;
    cout << "*|    2.ģ���ļ�����                         |*" << endl;
    cout << "*|    3.ģ��Ŀ¼����                         |*" << endl;
    cout << "*|    0.�˳�����                             |*" << endl;
    cout << "* ------------------------------------------- *" << endl;
    cout << "***********************************************" << endl;
    cout << endl;
}
// ���ڽ�Ŀ¼��Ϣд���ļ�
void writeDir(const string& filename, const vector<DirectoryInfo>& dirs) {
    // ����Ŀ¼��Ϣ����ÿ��Ŀ¼�������Ϣд���ļ�
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& dir : dirs) {
            // ��ʽ������ʱ����ļ����޸�ʱ�䣨������ʱ���룩
            tm* earliest_time = localtime(&dir.earliest_file.last_write_time);
            stringstream earliest_time_str;
            if (earliest_time) {
                earliest_time_str << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
            }
            else {
                earliest_time_str << "Invalid Time"; // ���ʱ����Ч�������������Ϣ
            }
            // ��ʽ������ʱ����ļ����޸�ʱ�䣨������ʱ���룩
            tm* latest_time = localtime(&dir.latest_file.last_write_time);
            stringstream latest_time_str;
            if (latest_time) {
                latest_time_str << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
            }
            else {
                latest_time_str << "Invalid Time"; // ���ʱ����Ч�������������Ϣ
            }
            outFile << dir.pathname << ","
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
// ���ڱ���Ŀ¼���ռ���Ϣ
void scan(const fs::path& directory, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories,int& file_count,int& dir_count) {
    stack<pair<fs::path, int>> dirs; // pair�еĵڶ����ʾĿ¼�����
    dirs.push({ directory, 0 });

    while (!dirs.empty()) {
        fs::path current_directory = dirs.top().first;
        int current_depth = dirs.top().second;
        dirs.pop();

        DirectoryInfo dir_info;
        dir_info.pathname = current_directory.string() + "\\";
        dir_info.depth = current_depth;
        dir_info.file_count = 0;
        dir_info.parent_directory = current_directory.parent_path().string() + "\\";
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


