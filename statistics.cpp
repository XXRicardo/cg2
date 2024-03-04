#include "func.h"

//用于统计某个目录的信息,根据输入的目录路径查找对应的目录节点并打印信息
void printDir(const string& inputPath, DirectoryInfo* root) {
    string targetPath = inputPath;
    // 找到对应的节点
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
    int flag = 0;
    // 如果找到了目标节点，打印信息
    if (targetNode) {
        cout << "目录搜寻成功" << endl;
        cout << "文件总数为: " << targetNode->file_count << endl;
        cout << "文件总大小: " << targetNode->total_file_size << " 字节" << endl;
        flag = 1;
        // 打印最早修改时间的文件信息
        tm* earliest_time = std::localtime(&targetNode->earliest_file.last_write_time);
        if (earliest_time) {
            cout << "最早文件信息: " << endl;
            stringstream ss;
            ss << put_time(earliest_time, "%Y-%m-%d %H:%M:%S");
            cout << "最后修改时间: " << ss.str() << endl;
            cout << "文件名: " << targetNode->earliest_file.filename << endl;
            cout << "文件大小: " << targetNode->earliest_file.file_size << " 字节" << endl;
        }
        else {
            cout << "最早文件信息获取失败" << endl;
        }

        // 打印最晚修改时间的文件信息
        tm* latest_time = std::localtime(&targetNode->latest_file.last_write_time);
        if (latest_time) {
            cout << "最晚文件信息: " << endl;
            stringstream ss;
            ss << put_time(latest_time, "%Y-%m-%d %H:%M:%S");
            cout << "最后修改时间: " << ss.str() << endl;
            cout << "文件名: " << targetNode->latest_file.filename << endl;
            cout << "文件大小: " << targetNode->latest_file.file_size << " 字节" << endl;
        }
        else {
            cout << "最晚文件信息获取失败" << endl;
        }
    }
    else {
        if (!flag)cout << "未找到目标目录节点！" << endl;
    }
    cout << endl;
}
// 循环读取用户输入的目录路径，并调用打印目录信息函数
void printLoop(DirectoryInfo*& root) {
    string inputPath;
    while (true) {
        // 获取用户输入的目录路径
        cout << "请输入带路径的目录名（输入0退出查询）: ";
        cin >> inputPath;

        // 如果用户输入"0"则退出查询
        if (inputPath == "0") {
            cout << "查询结束。" << endl;
            break;
        }

        // 调用函数打印目录信息
        printDir(inputPath, root);
    }
}