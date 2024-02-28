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
    string filename;    //�ļ���
    string path;        //�ļ�·��
    uintmax_t file_size;//�ļ���С
    time_t last_write_time; //�ļ�����޸�ʱ��
    int depth;          // �ļ�����Ŀ¼�����
};

// ����Ŀ¼��Ϣ�ṹ
struct DirectoryInfo {
    string pathname;            //��ȫ·����Ŀ¼��
    int depth;              //Ŀ¼���
    int file_count;         //Ŀ¼�µ��ļ�����
    string parent_directory;//��Ŀ¼��
    FileInfo earliest_file; // �����޸�ʱ����ļ���Ϣ
    FileInfo latest_file;   // �����޸�ʱ����ļ���Ϣ
    uintmax_t total_file_size; // �ܵ��ļ���С
    int td;                 //����
    // Binary tree pointers
    DirectoryInfo* left_child;  // ����ָ��
    DirectoryInfo* right_sibling; // ���ֵ�ָ��

    DirectoryInfo() : left_child(nullptr), right_sibling(nullptr),td(0) {} // ���캯����ʼ��ָ��
};

// ԭ�еĺ���������д���ļ���Ϣ
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

// �޸ĺ�ĺ��������ڽ�Ŀ¼��Ϣд���ļ�������������ʱ�����ʽ���޸�ʱ��
void writeDirToFile(const string& filename, const vector<DirectoryInfo>& dirs) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        for (const auto& dir : dirs) {
            // ��ʽ������ʱ����ļ����޸�ʱ�䣨������ʱ���룩
            std::tm* earliest_time = std::localtime(&dir.earliest_file.last_write_time);
            std::stringstream earliest_time_str;
            if (earliest_time) {
                earliest_time_str << std::put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
            }
            else {
                earliest_time_str << "Invalid Time"; // ���ʱ����Ч�������������Ϣ
            }
            // ��ʽ������ʱ����ļ����޸�ʱ�䣨������ʱ���룩
            std::tm* latest_time = std::localtime(&dir.latest_file.last_write_time);
            std::stringstream latest_time_str;
            if (latest_time) {
                latest_time_str << std::put_time(latest_time, "%Y-%m-%d %H:%M:%S");
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
                << dir.parent_directory  << endl;
        }
        outFile.close();
    }
}


// �Ƚ�����ʱ�����Ⱥ�˳��
bool compareTime(const time_t& time1, const time_t& time2) {
    return difftime(time1, time2) < 0;
}

void buildBinaryTree(const fs::path& rootDirectory, DirectoryInfo* root) {
    stack<pair<fs::path, DirectoryInfo*>> nodeStack;
    nodeStack.push({ rootDirectory, root });
    while (!nodeStack.empty()) {
        fs::path currentPath = nodeStack.top().first;
        DirectoryInfo* currentNode = nodeStack.top().second;
        nodeStack.pop();
        DirectoryInfo* prevSibling = nullptr;
        int parent_td = currentNode->td; // ��¼���ڵ�� td ֵ

        try {
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (fs::is_directory(entry)) {
                    DirectoryInfo* newDirectory = new DirectoryInfo();
                    newDirectory->pathname = entry.path().string()+"\\";
                    newDirectory->depth = currentNode->depth + 1;
                    newDirectory->file_count = 0;
                    newDirectory->parent_directory = currentNode->pathname;
                    newDirectory->total_file_size = 0;
                    newDirectory->td = parent_td + 1;  // �ӽڵ�� td ֵ�ȸ��ڵ�� td ֵ���� 1

                    if (prevSibling) {
                        newDirectory->td = prevSibling->td + 1;//�ǳ��ؼ�
                        prevSibling->right_sibling = newDirectory;
                    }
                    else {
                        currentNode->left_child = newDirectory;
                    }

                    prevSibling = newDirectory;

                    nodeStack.push({ entry, newDirectory });
                }
                else {
                    ++currentNode->file_count;
                    currentNode->total_file_size += fs::file_size(entry.path());
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            // ����Ȩ�޲�����쳣��ֱ�Ӻ���
        }
    }
}


int findMaxTd(DirectoryInfo* root) {
    if (!root)
        return -1;

    int maxTd = root->td;

    // ʹ��ջ������������ȱ���
    stack<DirectoryInfo*> nodeStack;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
        DirectoryInfo* currentNode = nodeStack.top();
        nodeStack.pop();

        // ��鵱ǰ�ڵ�� td ֵ�Ƿ�����ֵ��
        if (currentNode->td > maxTd)
            maxTd = currentNode->td;

        // ����ǰ�ڵ�������ӽڵ�ѹ��ջ��
        DirectoryInfo* childNode = currentNode->left_child;
        while (childNode) {
            nodeStack.push(childNode);
            childNode = childNode->right_sibling;
        }
    }

    return maxTd*3;
}


// ԭ�еĺ��������ڵݹ����Ŀ¼���ռ���Ϣ
void traverse(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories) {
    stack<pair<fs::path, int>> dirs; // pair�еĵڶ����ʾĿ¼�����
    dirs.push({ directory, 0 });

    while (!dirs.empty()) {
        fs::path current_directory = dirs.top().first;
        int current_depth = dirs.top().second;
        dirs.pop();

        DirectoryInfo dir_info;
        dir_info.pathname = current_directory.string()+"\\";
        dir_info.depth = current_depth;
        dir_info.file_count = 0;
        dir_info.parent_directory = current_directory.parent_path().string()+"\\";
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
    //traverse("C:\\Windows", file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories); // ɨ��Ŀ¼
    //// д���ļ���Ϣ���ļ�
    //writeToFile("D:/myfile.txt", files);
    //// д��Ŀ¼��Ϣ���ļ�
    //writeDirToFile("D:/mydir.txt", directories);
    //cout << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;
    //cout << "���������ļ���Ϣ��" << endl;
    //cout << "������: " << deepest_file_depth << endl;
    //cout << "�ļ�·��������: " << deepest_file_path << endl;

    cout << "���ڽ�����" << endl;
    // ����������
    DirectoryInfo* root = new DirectoryInfo(); // �������ڵ�
    root->pathname = "C:\\Windows\\";
    root->depth = 0;
    root->td = 0;
    root->file_count = 0;
    root->parent_directory = "";
    root->total_file_size = 0;
    root->left_child = nullptr;
    root->right_sibling = nullptr;
    buildBinaryTree("C:\\Windows", root); // ����������
    cout << "������������ɡ�" << endl;
    int maxTd = findMaxTd(root); // �ҳ����нڵ������� td ֵ
    cout << "���нڵ������� td ֵΪ: " << maxTd<< endl;
    return 0;
}