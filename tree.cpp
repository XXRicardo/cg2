#include "func.h"

//构建孩子兄弟树
void buildTree(const fs::path& rootDirectory, DirectoryInfo* root) {
    // 使用栈进行深度优先遍历
    stack<pair<fs::path, DirectoryInfo*>> nodeStack;
    nodeStack.push({ rootDirectory, root });

    while (!nodeStack.empty()) {
        fs::path currentPath = nodeStack.top().first;
        DirectoryInfo* currentNode = nodeStack.top().second;
        nodeStack.pop();
        DirectoryInfo* prevSibling = nullptr;
        int parent_td = currentNode->td; // 记录父节点的 td 值

        try {
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (fs::is_directory(entry)) {
                    DirectoryInfo* newDirectory = new DirectoryInfo();
                    newDirectory->pathname = entry.path().string() + "\\";
                    newDirectory->depth = currentNode->depth + 1;
                    newDirectory->file_count = 0;
                    newDirectory->parent_directory = currentNode->pathname;
                    newDirectory->total_file_size = 0;
                    newDirectory->earliest_file.last_write_time = std::numeric_limits<time_t>::max(); // 初始化为最大时间
                    newDirectory->latest_file.last_write_time = 0; // 初始化为最小时间

                    if (prevSibling) {
                        newDirectory->td = prevSibling->td + 1; //非常关键
                        prevSibling->right_sibling = newDirectory;
                    }
                    else {
                        newDirectory->td = parent_td + 1;  // 子节点的 td 值比父节点的 td 值增加 1
                        currentNode->left_child = newDirectory;
                    }

                    prevSibling = newDirectory;

                    nodeStack.push({ entry, newDirectory });
                }
                else {
                    ++currentNode->file_count;
                    currentNode->total_file_size += fs::file_size(entry.path());

                    // 更新最早时间的文件和最晚时间的文件的信息
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
            // 捕获权限不足的异常，直接忽略
        }
    }
}
//用于寻找最大树深
int findMaxTd(DirectoryInfo* root) {
    if (!root)
        return -1;

    int maxTd = root->td;

    // 使用栈来进行深度优先遍历
    stack<DirectoryInfo*> nodeStack;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
        DirectoryInfo* currentNode = nodeStack.top();
        nodeStack.pop();

        // 检查当前节点的 td 值是否比最大值大
        if (currentNode->td > maxTd)
            maxTd = currentNode->td;

        // 将当前节点的所有子节点压入栈中
        DirectoryInfo* childNode = currentNode->left_child;
        while (childNode) {
            nodeStack.push(childNode);
            childNode = childNode->right_sibling;
        }
    }

    return maxTd * 3;
}