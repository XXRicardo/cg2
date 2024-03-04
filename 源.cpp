#include "func.h"



int main() {

    vector<FileInfo> files;
    int max_depth = 0;
    int deepest_file_depth = 0;
    string deepest_file_path;
    vector<DirectoryInfo> directories;
    int file_count = 0, dir_count = 0;
    cout << "����ɨ��..." << endl;
    scan("C:\\Windows", files, max_depth, deepest_file_depth, deepest_file_path, directories, file_count, dir_count ); // ɨ��Ŀ¼
    // д���ļ���Ϣ���ļ�
    writeFile("D:/myfile.txt", files);
    // д��Ŀ¼��Ϣ���ļ�
    writeDir("D:/mydir.txt", directories);
    cout << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;
    cout << "���������ļ���Ϣ��" << endl;
    cout << "������: " << deepest_file_depth << endl;
    cout << "�ļ�·��������: " << deepest_file_path << endl;
    cout << endl;

    ofstream filetjt("D:/tongji.txt", ios::trunc);
    if (filetjt.is_open())filetjt.close();
    ofstream filetj1("D:/tongji.txt", ios::app);//һ������ͳ��,��һ��
    if (filetj1.is_open()) {
        filetj1 << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;
        filetj1 << "���������ļ���Ϣ��" << endl;
        filetj1 << "������: " << deepest_file_depth << endl;
        filetj1 << "�ļ�·��������: " << deepest_file_path << endl;
        filetj1 <<"-------------------------------------------------------------------------------" << endl;
    }
    if (filetj1.is_open())filetj1.close();


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
    buildTree("C:\\Windows", root); // ����������
    cout << "������������ɡ�" << endl;
    int maxTd = findMaxTd(root); // �ҳ����нڵ������� td ֵ
    cout << "���нڵ������� td ֵΪ: " << maxTd<< endl;

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
             fileop(files, filePath, operation, lastWriteTimeStr, sizeStr, file_count);
           }
           ofstream filetj2("D:/tongji.txt", ios::app);//һ������ͳ��,�ڶ���
           if (filetj2.is_open()) {
               filetj2 << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;
               filetj2 << "���������ļ���Ϣ��" << endl;
               filetj2 << "������: " << deepest_file_depth << endl;
               filetj2 << "�ļ�·��������: " << deepest_file_path << endl;
               filetj2 << "-------------------------------------------------------------------------------" << endl;
           }
           if (filetj2.is_open())filetj2.close();
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
                     dirop(root, pathname, file_count, dir_count);
                 }
                 else {
                     cout << "������Ч�����������룡" << endl;
                 }
             }
             ofstream filetj3("D:/tongji.txt", ios::app);//һ������ͳ��,������
             if (filetj3.is_open()) {
                 filetj3 << "�ܹ��� " << file_count << " ���ļ��� " << dir_count << " ��Ŀ¼��" << endl;
                 filetj3 << "���������ļ���Ϣ��" << endl;
                 filetj3 << "������: " << deepest_file_depth << endl;
                 filetj3 << "�ļ�·��������: " << deepest_file_path << endl;
                 filetj3 << "-------------------------------------------------------------------------------" << endl;
             }
             if (filetj3.is_open())filetj3.close();
             break;
         }

         default:
            cout << "��Чָ����������룡" << endl;
            break;
        }

    }
    return 0;
}