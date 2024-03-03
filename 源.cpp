#include <iostream>
#include <filesystem>
#include <stack>
#include <queue>
#include <ctime>
#include <chrono>
#include <vector>
#include <fstream>
#include <iomanip> 
#include <algorithm> 

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
void writeFile(const string& filename, const vector<FileInfo>& files) {
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
// �޸ĺ�ĺ��������ڽ�Ŀ¼��Ϣд���ļ�������������ʱ�����ʽ���޸�ʱ��
void writeDir(const string& filename, const vector<DirectoryInfo>& dirs) {
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
                << dir.parent_directory  << endl;
        }
        outFile.close();
    }
}

// �Ƚ�����ʱ�����Ⱥ�˳��
bool compareTime(const time_t& time1, const time_t& time2) {
    return difftime(time1, time2) < 0;
}

void buildTree(const fs::path& rootDirectory, DirectoryInfo* root) {
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
                    newDirectory->pathname = entry.path().string() + "\\";
                    newDirectory->depth = currentNode->depth + 1;
                    newDirectory->file_count = 0;
                    newDirectory->parent_directory = currentNode->pathname;
                    newDirectory->total_file_size = 0;
                    newDirectory->earliest_file.last_write_time = std::numeric_limits<time_t>::max(); // ��ʼ��Ϊ���ʱ��
                    newDirectory->latest_file.last_write_time = 0; // ��ʼ��Ϊ��Сʱ��

                    if (prevSibling) {
                        newDirectory->td = prevSibling->td + 1; //�ǳ��ؼ�
                        prevSibling->right_sibling = newDirectory;
                    }
                    else {
                        newDirectory->td = parent_td + 1;  // �ӽڵ�� td ֵ�ȸ��ڵ�� td ֵ���� 1
                        currentNode->left_child = newDirectory;
                    }

                    prevSibling = newDirectory;

                    nodeStack.push({ entry, newDirectory });
                }
                else {
                    ++currentNode->file_count;
                    currentNode->total_file_size += fs::file_size(entry.path());

                    // ��������ʱ����ļ�������ʱ����ļ�����Ϣ
                    FileInfo file_info;
                    file_info.filename = entry.path().filename().string();
                    file_info.path = entry.path().string();
                    file_info.file_size = fs::file_size(entry.path());

                    auto last_write_time = fs::last_write_time(entry.path());
                    auto last_write_time_point = chrono::time_point_cast<chrono::system_clock::duration>(last_write_time - fs::file_time_type::clock::now() + chrono::system_clock::now());
                    file_info.last_write_time = chrono::system_clock::to_time_t(last_write_time_point);

                    if (compareTime(file_info.last_write_time, currentNode->earliest_file.last_write_time)) {
                        currentNode->earliest_file = file_info;
                    }
                    if (compareTime(currentNode->latest_file.last_write_time, file_info.last_write_time)) {
                        currentNode->latest_file = file_info;
                    }
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

// ���ڱ���Ŀ¼���ռ���Ϣ
void scan(const fs::path& directory, int& file_count, int& dir_count, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories) {
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

void printDir(const string& inputPath, DirectoryInfo* root) {
    string targetPath = inputPath;
    // �ҵ���Ӧ�Ľڵ�
    DirectoryInfo* targetNode = nullptr;
    stack<DirectoryInfo*> nodeStack;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
        DirectoryInfo* currentNode = nodeStack.top();
        nodeStack.pop();

        if (currentNode->pathname == targetPath) {
            targetNode = currentNode;
            break;
        }

        DirectoryInfo* childNode = currentNode->left_child;
        while (childNode) {
            nodeStack.push(childNode);
            childNode = childNode->right_sibling;
        }
    }

    // ����ҵ���Ŀ��ڵ㣬��ӡ��Ϣ
    if (targetNode) {
        cout << "Ŀ¼��Ѱ�ɹ�" << endl;
        cout << "�ļ�����Ϊ: " << targetNode->file_count << endl;
        cout << "�ļ��ܴ�С: " << targetNode->total_file_size << " �ֽ�" << endl;

        // ��ӡ�����޸�ʱ����ļ���Ϣ
        tm* earliest_time = std::localtime(&targetNode->earliest_file.last_write_time);
        if (earliest_time) {
            cout << "�����ļ���Ϣ: " << endl;
            stringstream ss;
            ss << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
            cout << "����޸�ʱ��: " << ss.str() << endl;
            cout << "�ļ���: " << targetNode->earliest_file.filename << endl;
            cout << "�ļ���С: " << targetNode->earliest_file.file_size << " �ֽ�" << endl;
        }
        else {
            cout << "�����ļ���Ϣ��ȡʧ��" << endl;
        }

        // ��ӡ�����޸�ʱ����ļ���Ϣ
        tm* latest_time = std::localtime(&targetNode->latest_file.last_write_time);
        if (latest_time) {
            cout << "�����ļ���Ϣ: " << endl;
            stringstream ss;
            ss << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
            cout << "����޸�ʱ��: " << ss.str() << endl;
            cout << "�ļ���: " << targetNode->latest_file.filename << endl;
            cout << "�ļ���С: " << targetNode->latest_file.file_size << " �ֽ�" << endl;
        }
        else {
            cout << "�����ļ���Ϣ��ȡʧ��" << endl;
        }
    }
    else {
        cout << "δ�ҵ�Ŀ��Ŀ¼�ڵ㣡" << endl;
    }
    cout << endl;
}

void printLoop(DirectoryInfo*& root) {
    string inputPath;
    while (true) {
        // ��ȡ�û������Ŀ¼·��
        cout << "�������·����Ŀ¼��������0�˳���ѯ��: ";
        cin >> inputPath;

        // ����û�����"0"���˳���ѯ
        if (inputPath == "0") {
            cout << "��ѯ������" << endl;
            break;
        }

        // ���ú�����ӡĿ¼��Ϣ
        printDir(inputPath, root);
    }
}

void dirop(DirectoryInfo*& root, const string& targetPath) {
    if (!root)
        return;

    queue<DirectoryInfo*> q;
    q.push(root);

    DirectoryInfo* parentNode = nullptr;
    DirectoryInfo* currentNode = nullptr;
    int flag = 0;

    while (!q.empty()) {
        currentNode = q.front();
        q.pop();

        if (currentNode->pathname == targetPath) {
            // ��Ŀ¼�ڵ����Ϣд���ļ�
            ofstream outFile("D:/dirbijiao.txt",ios::app);
            if (outFile.is_open()) {
                outFile << "�޸�ǰĿ¼��Ϣ: " << endl;
                outFile << "��·����Ŀ¼��: " << currentNode->pathname << endl;
                outFile << "�ļ�����: " << currentNode->file_count << endl;

                outFile << "�����ļ���Ϣ: " << endl;
                outFile << "�ļ���: " << currentNode->earliest_file.filename << endl;
                outFile << "�ļ���С: " << currentNode->earliest_file.file_size << " bytes" << endl;

                tm* earliest_time = std::localtime(&currentNode->earliest_file.last_write_time);
                if (earliest_time) {
                    stringstream se;
                    se << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
                    outFile << "����޸�ʱ��: " << se.str() << endl;
                }
                outFile << "�����ļ���Ϣ: " << endl;
                outFile << "�ļ���: " << currentNode->latest_file.filename << endl;
                outFile << "�ļ���С: " << currentNode->latest_file.file_size << " bytes" << endl;

                tm* latest_time = std::localtime(&currentNode->latest_file.last_write_time);
                if (latest_time) {
                    stringstream sl;
                    sl << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
                    outFile << "����޸�ʱ��: " << sl.str() << endl;
                }
                outFile << endl;            
            }
            // ɾ��Ŀ��ڵ��Լ����ӽڵ�
            if (parentNode)
                parentNode->right_sibling = currentNode->right_sibling;
            else
                root = currentNode->right_sibling; // ���¸��ڵ�ָ��

            delete currentNode;
            flag = 1;
            cout << "�ɹ�ɾ��Ŀ¼ " << targetPath << " ������Ŀ¼" << endl;
            cout << endl;
            if (outFile.is_open()) {
                outFile << "�޸ĺ�δ�ҵ�Ŀ¼�ڵ���Ϣ��" << endl;
                outFile <<"----------------------------------------" << endl;
            }
            if (outFile.is_open())outFile.close();
            return;
        }

        DirectoryInfo* childNode = currentNode->left_child;
        while (childNode) {
            q.push(childNode);
            childNode = childNode->right_sibling;
        }

        parentNode = currentNode;
    }
    if (!flag) {
        cout << "δ�ҵ�Ŀ��Ŀ¼�ڵ㣡" << endl;
    }
}

void fileop(vector<FileInfo>& files, const string& fullFilename, const string& operation, const string& lastWriteTime, const string& size) {
    // ��ִ�·�����ļ���
    size_t lastBackslashPos = fullFilename.find_last_of("\\");
    string path = fullFilename.substr(0, lastBackslashPos + 1); // �ļ�·��
    string filename = fullFilename.substr(lastBackslashPos + 1); // �ļ���

    ofstream outputFile("D:/filebijiao.txt", ios::app);

    int dep = count(path.begin(), path.end(), '\\');
    vector<FileInfo> matchedFiles;

    // �ҵ������ͬ���ļ�
    for (const auto& file : files) {
        if (file.depth == dep) {
            matchedFiles.push_back(file);
        }
    }
    // �жϲ�������
    if (operation == "D") {
        // ɾ���ļ�
        if (outputFile.is_open()) {
            // �� matchedFiles �в��Ҷ�Ӧ���ļ�
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // �ҵ��ļ����������Ϣ׷��д���ļ�
                    outputFile << "�ļ���: " << file.filename << endl;
                    outputFile << "�ļ�·��: " << file.path << endl;
                    outputFile << "�ļ�����޸�ʱ��: " << file.last_write_time << endl;
                    outputFile << "�ļ���С: " << file.file_size << endl;
                    outputFile << "�ļ����: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // ���δ�ҵ��ļ�����д�� "�ļ�������" ���ļ���
            if (!found) {
                outputFile << "�ļ�������" << endl;
                outputFile << endl;
            }
        }
        bool deleted = false;
        for (auto it = files.begin(); it != files.end(); ++it) {
            if (it->depth == dep && it->filename == filename) {
                files.erase(it);
                deleted = true;
                break;
            }
        }
        for (auto mit = matchedFiles.begin(); mit != matchedFiles.end(); ++mit) {
            if ( mit->filename == filename) {
                matchedFiles.erase(mit);
                break;
            }
        }
        if (deleted) {
            cout << "�ļ�ɾ���ɹ�" << endl;
            cout << endl;
        }
        else {
            cout << "�ļ������ڣ�ɾ��ʧ��" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // �� matchedFiles �в��Ҷ�Ӧ���ļ�
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // �ҵ��ļ����������Ϣ׷��д���ļ�
                    outputFile << "�ļ���: " << file.filename << endl;
                    outputFile << "�ļ�·��: " << file.path << endl;
                    outputFile << "�ļ�����޸�ʱ��: " << file.last_write_time << endl;
                    outputFile << "�ļ���С: " << file.file_size << endl;
                    outputFile << "�ļ����: " << file.depth << endl;
                    outputFile <<"---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // ���δ�ҵ��ļ�����д�� "�ļ�������" ���ļ���
            if (!found) {
                outputFile << "�ļ�������" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }
        
        
    }
    else if (operation == "M") {
        // �޸��ļ�����
        
        if (outputFile.is_open()) {
            // �� matchedFiles �в��Ҷ�Ӧ���ļ�
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // �ҵ��ļ����������Ϣ׷��д���ļ�
                    outputFile << "�ļ���: " << file.filename << endl;
                    outputFile << "�ļ�·��: " << file.path << endl;
                    outputFile << "�ļ�����޸�ʱ��: " << file.last_write_time << endl;
                    outputFile << "�ļ���С: " << file.file_size << endl;
                    outputFile << "�ļ����: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // ���δ�ҵ��ļ�����д�� "�ļ�������" ���ļ���
            if (!found) {
                outputFile << "�ļ�������" << endl;
                outputFile << endl;
            }
        }
        bool modified = false;
        for (auto& file : files) {
            if (file.depth == dep && file.filename == filename) {
                // �޸�����޸�ʱ��ʹ�С
                file.last_write_time = stol(lastWriteTime);
                file.file_size = stoul(size);
                modified = true;
                break;
            }
        }
        for (auto& mfile : matchedFiles) {
            if ( mfile.filename == filename) {
                // �޸�����޸�ʱ��ʹ�С
                mfile.last_write_time = stol(lastWriteTime);
                mfile.file_size = stoul(size);
                break;
            }
        }
        if (modified) {
            cout << "�ļ������޸ĳɹ�" << endl;
            cout << endl;
        }
        else {
            cout << "�ļ������ڣ��޸�ʧ��" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // �� matchedFiles �в��Ҷ�Ӧ���ļ�
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // �ҵ��ļ����������Ϣ׷��д���ļ�
                    outputFile << "�ļ���: " << file.filename << endl;
                    outputFile << "�ļ�·��: " << file.path << endl;
                    outputFile << "�ļ�����޸�ʱ��: " << file.last_write_time << endl;
                    outputFile << "�ļ���С: " << file.file_size << endl;
                    outputFile << "�ļ����: " << file.depth << endl;
                    outputFile << "---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // ���δ�ҵ��ļ�����д�� "�ļ�������" ���ļ���
            if (!found) {
                outputFile << "�ļ�������" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }
        
    }
    else if (operation == "A") {
        // �����ļ�
        
        if (outputFile.is_open()) {
            // �� matchedFiles �в��Ҷ�Ӧ���ļ�
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // �ҵ��ļ����������Ϣ׷��д���ļ�
                    outputFile << "�ļ���: " << file.filename << endl;
                    outputFile << "�ļ�·��: " << file.path << endl;
                    outputFile << "�ļ�����޸�ʱ��: " << file.last_write_time << endl;
                    outputFile << "�ļ���С: " << file.file_size << endl;
                    outputFile << "�ļ����: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // ���δ�ҵ��ļ�����д�� "�ļ�������" ���ļ���
            if (!found) {
                outputFile << "�ļ�������" << endl;
                outputFile << endl;
            }
        }
        bool exists = false;
        for (const auto& file : matchedFiles) {
            if (file.filename == filename) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            // �ļ������ڣ�ִ����������
            FileInfo newFile;
            newFile.filename = filename;
            newFile.path = path; // ��·����ֵ���ļ���Ϣ�ṹ���е�path��Ա
            newFile.depth = dep;
            newFile.last_write_time = stol(lastWriteTime);
            newFile.file_size = stoul(size);
            files.push_back(newFile);
            matchedFiles.push_back(newFile);
            cout << "�ļ������ɹ�" << endl;
            cout << endl;
        }
        else {
            cout << "�ļ��Ѵ��ڣ�����ʧ��" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // �� matchedFiles �в��Ҷ�Ӧ���ļ�
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // �ҵ��ļ����������Ϣ׷��д���ļ�
                    outputFile << "�ļ���: " << file.filename << endl;
                    outputFile << "�ļ�·��: " << file.path << endl;
                    outputFile << "�ļ�����޸�ʱ��: " << file.last_write_time << endl;
                    outputFile << "�ļ���С: " << file.file_size << endl;
                    outputFile << "�ļ����: " << file.depth << endl;
                    outputFile << "---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // ���δ�ҵ��ļ�����д�� "�ļ�������" ���ļ���
            if (!found) {
                outputFile << "�ļ�������" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }
        
        
    }
    else {
        cout << "��Ч�Ĳ�������" << endl;
        cout << endl;
    }
    if (outputFile.is_open())outputFile.close();
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
    scan("C:\\Windows", file_count, dir_count, files, max_depth, deepest_file_depth, deepest_file_path, directories); // ɨ��Ŀ¼
    //// д���ļ���Ϣ���ļ�
    //writeFile("D:/myfile.txt", files);
    //// д��Ŀ¼��Ϣ���ļ�
    //writeDir("D:/mydir.txt", directories);
    cout << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;
    cout << "���������ļ���Ϣ��" << endl;
    cout << "������: " << deepest_file_depth << endl;
    cout << "�ļ�·��������: " << deepest_file_path << endl;
    cout << endl;

    ofstream filetjt("D:/tongji.txt", ios::trunc);
    if (filetjt.is_open())filetjt.close();
    ofstream filetj("D:/tongji.txt", ios::app);//һ������ͳ��,��һ��
    if (filetj.is_open()) {
        filetj << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;
        filetj << "���������ļ���Ϣ��" << endl;
        filetj << "������: " << deepest_file_depth << endl;
        filetj << "�ļ�·��������: " << deepest_file_path << endl;
        filetj <<"----------------------------------------" << endl;
    }
    if (filetj.is_open())filetj.close();


    cout << "���ڽ�����" << endl;
    // ����������
    DirectoryInfo* root = new DirectoryInfo(); // �������ڵ�
    root->pathname = "C:\\Windows\\";
    root->depth = 0;
    root->td = 0;
    root->file_count = 0;
    root->parent_directory = "";
    root->total_file_size = 0;
    root->earliest_file.last_write_time = std::numeric_limits<time_t>::max(); // ��ʼ��Ϊ���ʱ��
    root->latest_file.last_write_time = 0; // ��ʼ��Ϊ��Сʱ��
    root->left_child = nullptr;
    root->right_sibling = nullptr;
    //buildTree("C:\\Windows", root); // ����������
    //cout << "������������ɡ�" << endl;
    //int maxTd = findMaxTd(root); // �ҳ����нڵ������� td ֵ
    //cout << "���нڵ������� td ֵΪ: " << maxTd<< endl;

    ofstream filet("D:/filebijiao.txt", ios::trunc);
    if (filet.is_open())filet.close();

    ofstream file("D:/dirbijiao.txt", ios::trunc);
    if (file.is_open())file.close();
    
    while (1) {
        UI();
        cout << "������ָ��:";
        int zhiling;
        cin >> zhiling;
        switch (zhiling)
        {
         case 0:
            return 0;
         case 1:
             // ����ѭ��������ѯĿ¼��Ϣ
             printLoop(root);
             break;
         case 2: {
           while(true){
             string input;
             cout << "�������ļ���,����,ʱ��,��С(��0�˳�):";
             cin >> input;

             // ����û�����"0"�򷵻ع���ѡ�����
             if (input == "0") {
                 break;
             }

             // ���û�������ַ����ָ���ļ�����������ʱ��ʹ�С
             stringstream ss(input);
             string filePath, operation, lastWriteTimeStr, sizeStr;
             getline(ss, filePath, ',');
             getline(ss, operation, ',');
             getline(ss, lastWriteTimeStr, ',');
             getline(ss, sizeStr, ',');

             // �����ļ���������
             fileop(files, filePath, operation, lastWriteTimeStr, sizeStr);
            }
             break;
         }

         case 3: {
             // ģ��Ŀ¼����
             while (true) {
                 string input;
                 cout << "������Ŀ¼��,����,ʱ��,��С(��0�˳�):";
                 cin >> input;

                 // ����û�����"0"�򷵻ع���ѡ�����
                 if (input == "0") {
                     break;
                 }

                 // ���û�������ַ����ָ��Ŀ¼���Ͳ���
                 size_t pos = input.find(',');
                 if (pos != string::npos) {
                     string pathname = input.substr(0, pos);
                     string operation = input.substr(pos + 1);
                     dirop(root, pathname);
                 }
                 else {
                     cout << "������Ч�����������룡" << endl;
                 }
             }
             break;
         }

         default:
            cout << "��Чָ����������룡" << endl;
            break;
        }

    }
    if (filetj.is_open())filetj.close();
    return 0;
}