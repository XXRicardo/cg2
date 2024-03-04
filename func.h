#pragma once
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

    DirectoryInfo() : left_child(nullptr), right_sibling(nullptr), td(0) {} // ���캯����ʼ��ָ��
};
// ����д���ļ���Ϣ
void writeFile(const string& filename, const vector<FileInfo>& files);

void UI();

// ���ڽ�Ŀ¼��Ϣд���ļ�
void writeDir(const string& filename, const vector<DirectoryInfo>& dirs);

// �Ƚ�����ʱ�����Ⱥ�˳��
bool compareTime(const time_t& time1, const time_t& time2);

//���������ֵ���
void buildTree(const fs::path& rootDirectory, DirectoryInfo* root);

//����Ѱ���������
int findMaxTd(DirectoryInfo* root);

// ���ڱ���Ŀ¼���ռ���Ϣ
void scan(const fs::path& directory, vector<FileInfo>& files, int& max_depth, int& deepest_file_depth, string& deepest_file_path, vector<DirectoryInfo>& directories, int& file_count, int& dir_count);

//����ͳ��ĳ��Ŀ¼����Ϣ,���������Ŀ¼·�����Ҷ�Ӧ��Ŀ¼�ڵ㲢��ӡ��Ϣ
void printDir(const string& inputPath, DirectoryInfo* root);

// ѭ����ȡ�û������Ŀ¼·���������ô�ӡĿ¼��Ϣ����
void printLoop(DirectoryInfo*& root);

//ģ��Ŀ¼�������ں����ֵ����в��Ҳ�ɾ��Ŀ��Ŀ¼������Ŀ¼
void dirop(DirectoryInfo*& root, const string& targetPath, int& file_count, int& dir_count);

// ģ���ļ�������ɾ�����޸ĺ������ļ�
void fileop(vector<FileInfo>& files, const string& fullFilename, const string& operation, const string& lastWriteTime, const string& size, int& file_count);