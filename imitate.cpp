#include "func.h"

//ģ��Ŀ¼�������ں����ֵ����в��Ҳ�ɾ��Ŀ��Ŀ¼������Ŀ¼
void dirop(DirectoryInfo*& root, const string& targetPath, int& file_count, int& dir_count) {
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
            ofstream outFile("D:/dirbijiao.txt", ios::app);
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
            file_count -= currentNode->file_count;
            delete currentNode;
            dir_count--;

            flag = 1;
            cout << "�ɹ�ɾ��Ŀ¼ " << targetPath << " ������Ŀ¼" << endl;
            cout << endl;
            if (outFile.is_open()) {
                outFile << "�޸ĺ�δ�ҵ�Ŀ¼�ڵ���Ϣ��" << endl;
                outFile << "----------------------------------------" << endl;
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
// ģ���ļ�������ɾ�����޸ĺ������ļ�
void fileop(vector<FileInfo>& files, const string& fullFilename, const string& operation, const string& lastWriteTime, const string& size, int& file_count) {
    // ��ִ�·�����ļ���
    size_t lastBackslashPos = fullFilename.find_last_of("\\");
    string path = fullFilename.substr(0, lastBackslashPos + 1); // �ļ�·��
    string filename = fullFilename.substr(lastBackslashPos + 1); // �ļ���
    int flag = 0;
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
        flag = 1;
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
            if (mit->filename == filename) {
                matchedFiles.erase(mit);
                break;
            }
        }
        if (deleted) {
            cout << "�ļ�ɾ���ɹ�" << endl;
            file_count--;
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
    else if (operation == "M") {
        // �޸��ļ�����
        flag = 1;
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
            if (mfile.filename == filename) {
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
        flag = 1;
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
            file_count++;
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
        if (!flag)cout << "��Ч�Ĳ�������" << endl;
        cout << endl;
    }
    if (outputFile.is_open())outputFile.close();
}
