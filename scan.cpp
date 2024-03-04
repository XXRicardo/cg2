#include "func.h"

// 用于写入文件信息
void writeFile(const string& filename, const vector<FileInfo>& files) {
    // 遍历文件信息并将每个文件的相关信息写入文件
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
    cout << "*|    请选择您要执行的功能：                 |*" << endl;
    cout << "*|    1.统计指定目录信息                     |*" << endl;
    cout << "*|    2.模拟文件操作                         |*" << endl;
    cout << "*|    3.模拟目录操作                         |*" << endl;
    cout << "*|    0.退出程序                             |*" << endl;
    cout << "* ------------------------------------------- *" << endl;
    cout << "***********************************************" << endl;
    cout << endl;
}
// 用于将目录信息写入文件
void writeDir(const string& filename, const vector<DirectoryInfo>& dirs) {
    // 遍历目录信息并将每个目录的相关信息写入文件
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& dir : dirs) {
            // 格式化最早时间的文件的修改时间（年月日时分秒）
            tm* earliest_time = localtime(&dir.earliest_file.last_write_time);
            stringstream earliest_time_str;
            if (earliest_time) {
                earliest_time_str << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
            }
            else {
                earliest_time_str << "Invalid Time"; // 如果时间无效，则输出错误信息
            }
            // 格式化最晚时间的文件的修改时间（年月日时分秒）
            tm* latest_time = localtime(&dir.latest_file.last_write_time);
            stringstream latest_time_str;
            if (latest_time) {
                latest_time_str << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
            }
            else {
                latest_time_str << "Invalid Time"; // 如果时间无效，则输出错误信息
            }
            outFile << dir.pathname << ","
                << dir.depth << ","
                << dir.file_count << ","
                << dir.total_file_size << ","
                << earliest_time_str.str() << "," // 输出最早时间的文件的修改时间
                << latest_time_str.str() << ","   // 输出最晚时间的文件的修改时间
                << dir.parent_directory << endl;
        }
        outFile.close();
    }
}
// 比较两个时间点的先后顺序
bool compareTime(const time_t& time1, const time_t& time2) {
    return difftime(time1, time2) < 0;
}
// 用于遍历目录并收集信息
void scan(const fs::path& directory, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories,int& file_count,int& dir_count) {
    stack<pair<fs::path, int>> dirs; // pair中的第二项表示目录的深度
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
        dir_info.total_file_size = 0; // 初始化总的文件大小为 0

        // 初始化最早时间的文件和最晚时间的文件的信息
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
                    // 如果是文件，将文件信息存入 FileInfo 结构中
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.path = entry.path().string();
                    file_info.file_size = fs::file_size(entry.path());

                    // 获取文件最后修改时间并转换为 time_t 类型
                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_point = chrono::time_point_cast<chrono::system_clock::duration>(last_write_time - fs::file_time_type::clock::now() + chrono::system_clock::now());
                    file_info.last_write_time = chrono::system_clock::to_time_t(last_write_time_point);

                    file_info.depth = current_depth;

                    files.push_back(file_info);
                    ++file_count;

                    // 更新最大深度
                    max_depth = max(max_depth, current_depth);

                    // 更新深度最深的文件信息
                    if (current_depth > deepest_file_depth) {
                        deepest_file_depth = current_depth;
                        deepest_file_path = entry.path().string();
                    }

                    // 更新目录中的文件数量
                    ++directories.back().file_count;

                    // 更新最早时间的文件和最晚时间的文件的信息
                    if (compareTime(file_info.last_write_time, directories.back().earliest_file.last_write_time)) {
                        directories.back().earliest_file = file_info;
                    }
                    if (compareTime(directories.back().latest_file.last_write_time, file_info.last_write_time)) {
                        directories.back().latest_file = file_info;
                    }

                    // 累加总的文件大小
                    directories.back().total_file_size += file_info.file_size;
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // 捕获权限不足的异常，直接忽略
        }
    }
}


