#include "FileManager.h"
#include<cstdio>
#include<sys/stat.h>
#include <cstring>


FileManager::FileManager(){}
FileManager::~FileManager(){
    closeFile();
}

bool FileManager::createFile(const std::string &filename_) {
    closeFile();

    std::ofstream ofs(filename_, std::ios::binary | std::ios::trunc);
    if (!ofs) return false;
    ofs.close();

    filename = filename_;

    fs.open(filename_, std::ios::in |
                       std::ios::out |
                       std::ios::binary |
                       std::ios::ate);

    return fs.is_open();
}



bool FileManager::openFile(const std::string &filename_) {
    closeFile();

    filename = filename_;


    std::ifstream test(filename_);
    bool exists = test.good();
    test.close();


    if (!exists) {
        std::ofstream ofs(filename_, std::ios::binary | std::ios::trunc);
        if (!ofs) return false;
        ofs.close();
    }


    fs.open(filename_, std::ios::in |
                       std::ios::out |
                       std::ios::binary |
                       std::ios::ate);

    if (!fs.is_open())
        return false;


    fs.seekg(0);

    return true;
}




void FileManager::closeFile(){
    if(fs.is_open()){
        fs.close();
    }
}

bool FileManager::truncate(){
    closeFile();
    std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
    if(!ofs){return false;}
    ofs.close();
    fs.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    return fs.good();
}

long long FileManager::append(const char *buf, size_t size){
    if(!fs.is_open()){
        std::cerr << "File not open in append!" << std::endl;
        return -1;
    }

    if (fs.fail()) {
        std::cerr << "File stream in fail state!" << std::endl;
        fs.clear(); 
    }
    
    fs.seekp(0, std::ios::end);
    long long pos = fs.tellp();
    
    if (fs.fail()) {
        std::cerr << "seekp/tellp failed!" << std::endl;
        return -1;
    }
    
    fs.write(buf, size);
    
    if (fs.fail()) {
        std::cerr << "write failed!" << std::endl;
        return -1;
    }
    
    fs.flush();
    return pos;
}

bool FileManager::writeAt(long long offset, const char *buf, size_t size){
    if(!fs.is_open()){
        return false;
    }
    fs.seekp(offset);
    fs.write(buf, size);
    fs.flush();
    return true;
}

bool FileManager::readAt(long long offset, char *buf, size_t size){
    if(!fs.is_open()){
        return false;
    }
    fs.seekg(offset);
    fs.read(buf, size);
    return fs.good();
}

void FileManager::seekToBegin(){
    if(fs.is_open()){
        fs.seekg(0, std::ios::beg);
    }
}

bool FileManager::readNext(char* buf, size_t size){
    if(!fs.is_open()){
        return false;
    }
    

    if (fs.eof()) {
        fs.clear();
    }
    

    std::streampos current = fs.tellg();
    fs.seekg(0, std::ios::end);
    std::streampos end = fs.tellg();
    fs.seekg(current); 
    
    if (current == end) {
        return false; 
    }
    
    if (end - current < static_cast<std::streampos>(size)) {
        return false; 
    }

    fs.read(buf, size);
    

    if (fs.gcount() != static_cast<std::streamsize>(size)) {
        return false;
    }
    
    return true;
}
bool FileManager::copyTo(const std::string &dest){
    return copyFile(filename, dest);
}
bool FileManager::copyFile(const std::string &src, const std::string &dest){
    std::ifstream ifs(src, std::ios::binary);
    if(!ifs){
        std::cerr << "Cannot open source file: " << src << std::endl;
        return false;
    }
    
    std::ofstream ofs(dest, std::ios::binary | std::ios::trunc);
    if(!ofs){
        std::cerr << "Cannot create destination file: " << dest << std::endl;
        ifs.close();
        return false;
    }
    
    ofs << ifs.rdbuf();
    
    bool success = !ifs.fail() && !ofs.fail();
    
    ifs.close();
    ofs.close();
    
    if(!success){
        std::cerr << "File copy failed from " << src << " to " << dest << std::endl;
        std::remove(dest.c_str());
    }
    
    return success;
}

