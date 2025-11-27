#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include<iostream>
#include<fstream>
#include <vector>

struct Student
{
    int id;
    char name[50];
    bool isActive;
    double averageGrade;
    int cours;

    Student(): id(0), isActive(true), averageGrade(0.0), cours(1){
        name[0] = '\0';
    }
};

class FileManager{
    private:
        std::string filename;
        std::fstream fs;
    public:
        FileManager();
        ~FileManager();

        bool createFile(const std::string &filename);
        bool openFile(const std::string &filename);
        void closeFile();
        bool truncate(); 

        long long append(const char *buf, size_t size);
        bool writeAt(long long offset, const char *buf, size_t size);
        bool readAt(long long offset, char *buf, size_t size);
        void seekToBegin();
        bool readNext(char* buf, size_t size);

        bool copyTo(const std::string &dest);
        static bool copyFile(const std::string &src, const std::string &dest);

};



#endif