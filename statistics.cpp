#include "func.h"

//����ͳ��ĳ��Ŀ¼����Ϣ,���������Ŀ¼·�����Ҷ�Ӧ��Ŀ¼�ڵ㲢��ӡ��Ϣ
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
    int flag = 0;
    // ����ҵ���Ŀ��ڵ㣬��ӡ��Ϣ
    if (targetNode) {
        cout << "Ŀ¼��Ѱ�ɹ�" << endl;
        cout << "�ļ�����Ϊ: " << targetNode->file_count << endl;
        cout << "�ļ��ܴ�С: " << targetNode->total_file_size << " �ֽ�" << endl;
        flag = 1;
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
        if (!flag)cout << "δ�ҵ�Ŀ��Ŀ¼�ڵ㣡" << endl;
    }
    cout << endl;
}
// ѭ����ȡ�û������Ŀ¼·���������ô�ӡĿ¼��Ϣ����
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