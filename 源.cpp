#include "func.h"



int main() {

    vector<FileInfo> files;
    int max_depth = 0;
    int deepest_file_depth = 0;
    string deepest_file_path;
    vector<DirectoryInfo> directories;
    int file_count = 0, dir_count = 0;
    cout << "正在扫描..." << endl;
    scan("C:\\Windows", files, max_depth, deepest_file_depth, deepest_file_path, directories, file_count, dir_count ); // 扫描目录
    // 写入文件信息到文件
    writeFile("D:/myfile.txt", files);
    // 写入目录信息到文件
    writeDir("D:/mydir.txt", directories);
    cout << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;
    cout << "深度最深的文件信息：" << endl;
    cout << "最大深度: " << deepest_file_depth << endl;
    cout << "文件路径及名字: " << deepest_file_path << endl;
    cout << endl;

    ofstream filetjt("D:/tongji.txt", ios::trunc);
    if (filetjt.is_open())filetjt.close();
    ofstream filetj1("D:/tongji.txt", ios::app);//一共三次统计,第一次
    if (filetj1.is_open()) {
        filetj1 << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;
        filetj1 << "深度最深的文件信息：" << endl;
        filetj1 << "最大深度: " << deepest_file_depth << endl;
        filetj1 << "文件路径及名字: " << deepest_file_path << endl;
        filetj1 <<"-------------------------------------------------------------------------------" << endl;
    }
    if (filetj1.is_open())filetj1.close();


    cout << "正在建树。" << endl;
    // 构建二叉树
    DirectoryInfo* root = new DirectoryInfo(); // 创建根节点
    root->pathname = "C:\\Windows\\";
    root->depth = 0;
    root->td = 0;
    root->file_count = 0;
    root->parent_directory = "";
    root->total_file_size = 0;
    root->earliest_file.last_write_time = std::numeric_limits<time_t>::max(); // 初始化为最大时间
    root->latest_file.last_write_time = 0; // 初始化为最小时间
    root->left_child = nullptr;
    root->right_sibling = nullptr;
    buildTree("C:\\Windows", root); // 构建二叉树
    cout << "二叉树构建完成。" << endl;
    int maxTd = findMaxTd(root); // 找出所有节点中最大的 td 值
    cout << "所有节点中最大的 td 值为: " << maxTd<< endl;

    ofstream filet("D:/filebijiao.txt", ios::trunc);
    if (filet.is_open())filet.close();

    ofstream file("D:/dirbijiao.txt", ios::trunc);
    if (file.is_open())file.close();
    
    while (1) {
        UI();
        cout << "请输入指令:";
        int zhiling;
        cin >> zhiling;
        switch (zhiling)
        {
         case 0:
            return 0;
         case 1:
             // 调用循环函数查询目录信息
             printLoop(root);
             break;
         case 2: {
           while(true){
             string input;
             cout << "请输入文件名,操作,时间,大小(按0退出):";
             cin >> input;

             // 如果用户输入"0"则返回功能选择界面
             if (input == "0") {
                 break;
             }

             // 将用户输入的字符串分割成文件名、操作、时间和大小
             stringstream ss(input);
             string filePath, operation, lastWriteTimeStr, sizeStr;
             getline(ss, filePath, ',');
             getline(ss, operation, ',');
             getline(ss, lastWriteTimeStr, ',');
             getline(ss, sizeStr, ',');

             // 调用文件操作函数
             fileop(files, filePath, operation, lastWriteTimeStr, sizeStr, file_count);
           }
           ofstream filetj2("D:/tongji.txt", ios::app);//一共三次统计,第二次
           if (filetj2.is_open()) {
               filetj2 << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;
               filetj2 << "深度最深的文件信息：" << endl;
               filetj2 << "最大深度: " << deepest_file_depth << endl;
               filetj2 << "文件路径及名字: " << deepest_file_path << endl;
               filetj2 << "-------------------------------------------------------------------------------" << endl;
           }
           if (filetj2.is_open())filetj2.close();
             break;
         }

         case 3: {
             // 模拟目录操作
             while (true) {
                 string input;
                 cout << "请输入目录名,操作,时间,大小(按0退出):";
                 cin >> input;

                 // 如果用户输入"0"则返回功能选择界面
                 if (input == "0") {
                     break;
                 }

                 // 将用户输入的字符串分割成目录名和操作
                 size_t pos = input.find(',');
                 if (pos != string::npos) {
                     string pathname = input.substr(0, pos);
                     string operation = input.substr(pos + 1);
                     dirop(root, pathname, file_count, dir_count);
                 }
                 else {
                     cout << "输入无效，请重新输入！" << endl;
                 }
             }
             ofstream filetj3("D:/tongji.txt", ios::app);//一共三次统计,第三次
             if (filetj3.is_open()) {
                 filetj3 << "总共有 " << file_count << " 个文件和 " << dir_count << " 个目录。" << endl;
                 filetj3 << "深度最深的文件信息：" << endl;
                 filetj3 << "最大深度: " << deepest_file_depth << endl;
                 filetj3 << "文件路径及名字: " << deepest_file_path << endl;
                 filetj3 << "-------------------------------------------------------------------------------" << endl;
             }
             if (filetj3.is_open())filetj3.close();
             break;
         }

         default:
            cout << "无效指令，请重新输入！" << endl;
            break;
        }

    }
    return 0;
}