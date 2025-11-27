#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "FileManager.h"

#pragma pack(push,1)
struct StoredStudent {
    int id;
    char name[50];
    unsigned char isActive; // 1 active, 0 deleted
    double averageGrade;
    int cours;
};

class Database {
    private:
        FileManager fm;
        std::string dbFilename;
        std::string idxFilename;
        bool openFlag;
        std::unordered_map<int, long long> index;
        bool loadIndex(); //открывает, читает, заполняет, возвращает
        bool persistIndex();//открывает, записывает в файл, возвращает
        long long appendRecordToFile(const StoredStudent &rs); //добавляет запись в окнец
        bool markRecordDeleted(long long offset); //помечает запись как удаленное
        bool readRecordAt(long long offset, StoredStudent &out);//чтение на указанной позиции
    public:
        Database();
        ~Database();
        bool create(const std::string &filename);
        bool open(const std::string &filename);
        bool close();
        bool removeDB(const std::string &filename);
        bool clear();
        bool save();

        bool addRecord(const Student &s, std::string &err);
        size_t deleteByField(const std::string &field, const std::string &value);
        std::vector<Student> searchByField(const std::string &field, const std::string &value);
        bool editRecordByKey(int keyId, const Student &newS);
        bool backup(const std::string &backupFile);
        bool restoreFromBackup(const std::string &backupFile);
        bool exportCSV(const std::string &csvFile);
        bool isOpen() const { return openFlag; }
        std::string getFilename() const { return dbFilename; }
        std::vector<Student> getAll();
        bool checkIntegrity(); 
        void debugIndex() { // добавить в публичную секцию
            std::cout << "=== INDEX DEBUG ===" << std::endl;
            std::cout << "Index size: " << index.size() << std::endl;
            for (const auto& pair : index) {
                std::cout << "ID: " << pair.first << " -> Offset: " << pair.second << std::endl;
        }
    }
};

#endif