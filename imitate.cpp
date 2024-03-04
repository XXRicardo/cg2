#include "func.h"

//模拟目录操作：在孩子兄弟树中查找并删除目标目录及其子目录
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
            // 将目录节点的信息写入文件
            ofstream outFile("D:/dirbijiao.txt", ios::app);
            if (outFile.is_open()) {
                outFile << "修改前目录信息: " << endl;
                outFile << "带路径的目录名: " << currentNode->pathname << endl;
                outFile << "文件总数: " << currentNode->file_count << endl;

                outFile << "最早文件信息: " << endl;
                outFile << "文件名: " << currentNode->earliest_file.filename << endl;
                outFile << "文件大小: " << currentNode->earliest_file.file_size << " bytes" << endl;

                tm* earliest_time = std::localtime(&currentNode->earliest_file.last_write_time);
                if (earliest_time) {
                    stringstream se;
                    se << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
                    outFile << "最后修改时间: " << se.str() << endl;
                }
                outFile << "最晚文件信息: " << endl;
                outFile << "文件名: " << currentNode->latest_file.filename << endl;
                outFile << "文件大小: " << currentNode->latest_file.file_size << " bytes" << endl;

                tm* latest_time = std::localtime(&currentNode->latest_file.last_write_time);
                if (latest_time) {
                    stringstream sl;
                    sl << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
                    outFile << "最后修改时间: " << sl.str() << endl;
                }
                outFile << endl;
            }
            // 删除目标节点以及其子节点
            if (parentNode)
                parentNode->right_sibling = currentNode->right_sibling;
            else
                root = currentNode->right_sibling; // 更新根节点指针
            file_count -= currentNode->file_count;
            delete currentNode;
            dir_count--;

            flag = 1;
            cout << "成功删除目录 " << targetPath << " 及其子目录" << endl;
            cout << endl;
            if (outFile.is_open()) {
                outFile << "修改后未找到目录节点信息！" << endl;
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
        cout << "未找到目标目录节点！" << endl;
    }
}
// 模拟文件操作：删除、修改和新增文件
void fileop(vector<FileInfo>& files, const string& fullFilename, const string& operation, const string& lastWriteTime, const string& size, int& file_count) {
    // 拆分带路径的文件名
    size_t lastBackslashPos = fullFilename.find_last_of("\\");
    string path = fullFilename.substr(0, lastBackslashPos + 1); // 文件路径
    string filename = fullFilename.substr(lastBackslashPos + 1); // 文件名
    int flag = 0;
    ofstream outputFile("D:/filebijiao.txt", ios::app);

    int dep = count(path.begin(), path.end(), '\\');
    vector<FileInfo> matchedFiles;

    // 找到深度相同的文件
    for (const auto& file : files) {
        if (file.depth == dep) {
            matchedFiles.push_back(file);
        }
    }
    // 判断操作类型
    if (operation == "D") {
        // 删除文件
        flag = 1;
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
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
            cout << "文件删除成功" << endl;
            file_count--;
            cout << endl;
        }
        else {
            cout << "文件不存在，删除失败" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << "---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }


    }
    else if (operation == "M") {
        // 修改文件属性
        flag = 1;
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << endl;
            }
        }
        bool modified = false;
        for (auto& file : files) {
            if (file.depth == dep && file.filename == filename) {
                // 修改最后修改时间和大小
                file.last_write_time = stol(lastWriteTime);
                file.file_size = stoul(size);
                modified = true;
                break;
            }
        }
        for (auto& mfile : matchedFiles) {
            if (mfile.filename == filename) {
                // 修改最后修改时间和大小
                mfile.last_write_time = stol(lastWriteTime);
                mfile.file_size = stoul(size);
                break;
            }
        }
        if (modified) {
            cout << "文件属性修改成功" << endl;
            cout << endl;
        }
        else {
            cout << "文件不存在，修改失败" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << "---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }

    }
    else if (operation == "A") {
        // 新增文件
        flag = 1;
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
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
            // 文件不存在，执行新增操作
            FileInfo newFile;
            newFile.filename = filename;
            newFile.path = path; // 将路径赋值给文件信息结构体中的path成员
            newFile.depth = dep;
            newFile.last_write_time = stol(lastWriteTime);
            newFile.file_size = stoul(size);
            files.push_back(newFile);
            matchedFiles.push_back(newFile);
            cout << "文件新增成功" << endl;
            file_count++;
            cout << endl;
        }
        else {
            cout << "文件已存在，新增失败" << endl;
            cout << endl;
        }
        if (outputFile.is_open()) {
            // 在 matchedFiles 中查找对应的文件
            bool found = false;
            for (const auto file : matchedFiles) {
                if (file.filename == filename) {
                    // 找到文件，将五个信息追加写入文件
                    outputFile << "文件名: " << file.filename << endl;
                    outputFile << "文件路径: " << file.path << endl;
                    outputFile << "文件最后修改时间: " << file.last_write_time << endl;
                    outputFile << "文件大小: " << file.file_size << endl;
                    outputFile << "文件深度: " << file.depth << endl;
                    outputFile << "---------------------------------------------" << endl;
                    found = true;
                    break;
                }
            }

            // 如果未找到文件，则写入 "文件不存在" 到文件中
            if (!found) {
                outputFile << "文件不存在" << endl;
                outputFile << "---------------------------------------------" << endl;
            }
        }


    }
    else {
        if (!flag)cout << "无效的操作类型" << endl;
        cout << endl;
    }
    if (outputFile.is_open())outputFile.close();
}
