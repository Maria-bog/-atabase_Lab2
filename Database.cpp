#include "Database.h"
#include <fstream>
#include <cstring>
#include <sys/stat.h>

Database::Database(): openFlag(false) {}
Database::~Database(){ close(); }

bool Database::create(const std::string &filename){
    if(openFlag) close();

    if(!fm.createFile(filename)) return false;

    dbFilename = filename;
    idxFilename = filename + ".idx";

    index.clear();
    persistIndex();

    openFlag = true;
    return true;
}


bool Database::open(const std::string &filename){
    if(openFlag) close();
    if(!fm.openFile(filename)){return false;}
    dbFilename = filename;
    idxFilename = filename + ".idx";
    if(!loadIndex()){
        index.clear();
        persistIndex();
    }
    openFlag = true;
    return true;
}

bool Database::close(){
    if(!openFlag) return true;
    persistIndex();
    fm.closeFile();
    index.clear();
    openFlag = false;
    return true;
}

bool Database::removeDB(const std::string &filename){
    close();
    std::remove(filename.c_str());
    std::remove((filename + ".idx").c_str());
    return true;
}

bool Database::clear(){
    if(!openFlag) return false;
    fm.truncate();
    index.clear();
    persistIndex();
    return true;
}

bool Database::save(){
    if(!openFlag) return false;
    return persistIndex();
}

long long Database::appendRecordToFile(const StoredStudent &rs){
    return fm.append((const char*)&rs, sizeof(StoredStudent));
}

bool Database::loadIndex(){
    index.clear();
    std::ifstream ifs(idxFilename, std::ios::binary);
    if(!ifs){return false;}
    while (true)
    {
        int id;
        long long off;
        ifs.read((char*)&id, sizeof(id));
        if(!ifs) break;
        ifs.read((char*)&off, sizeof(off));
        if(!ifs) break;
        index[id] = off;
    }
    return true;
}

bool Database::persistIndex(){
    std::ofstream ofs(idxFilename, std::ios::binary | std::ios::trunc);
    if(!ofs){return false;}
    for(auto &p: index){
        ofs.write((char*)&p.first, sizeof(p.first));
        ofs.write((char*)&p.second, sizeof(p.second));
    }

    return true;
}

bool Database::addRecord(const Student &s, std::string &err){
    if(!openFlag){ err = "DB is not open"; return false; }
    
    std::cout << "=== ADD RECORD ===" << std::endl;
    std::cout << "New record - ID: " << s.id << ", Name: " << s.name << std::endl;
    std::cout << "Current index size: " << index.size() << std::endl;
    
    if(index.find(s.id) != index.end()){ 
        std::cout << "DUPLICATE ID FOUND IN INDEX!" << std::endl;
        err = "duplicate key (id)"; 
        return false; 
    }
    
    StoredStudent rs;
    rs.id = s.id;
    strncpy(rs.name, s.name, sizeof(rs.name)-1);
    rs.name[sizeof(rs.name)-1] = '\0';
    rs.isActive = s.isActive ? 1 : 0;
    rs.averageGrade = s.averageGrade;
    rs.cours = s.cours;
    
    long long off = appendRecordToFile(rs);
    std::cout << "Record written at offset: " << off << std::endl;
    
    if(off < 0){err = "file write error"; return false;}
    
    index[s.id] = off;
    persistIndex();
    
    std::cout << "Record added successfully. New index size: " << index.size() << std::endl;
    return true;
}

bool Database::readRecordAt(long long offset, StoredStudent &out){
    if(!fm.readAt(offset, (char*)&out, sizeof(StoredStudent))){return false;}
    return true;
}

bool Database::markRecordDeleted(long long offset){
    StoredStudent rs;
    if(!readRecordAt(offset, rs)){return false;}
    if(rs.isActive == 0){return false;}
    rs.isActive = 0;
    if(!fm.writeAt(offset, (const char*)&rs, sizeof(StoredStudent))){return false;}
    return true;
}


std::vector<Student> Database::searchByField(const std::string &field, const std::string &value){
    std::vector<Student> res;

    if(field == "id"){
        int searchId = std::stoi(value);
        std::cout << "Searching for ID: " << searchId << " in index..." << std::endl;
        
        auto it = index.find(searchId);
        if(it != index.end()) {
            std::cout << "Found ID in index at offset: " << it->second << std::endl;
            
            StoredStudent rs;
            if(readRecordAt(it->second, rs)) {
                std::cout << "Read record - ID: " << rs.id << ", Active: " << (int)rs.isActive << std::endl;
                
                if(rs.isActive){
                    Student s;
                    s.id = rs.id;
                    strncpy(s.name, rs.name, sizeof(s.name));
                    s.name[sizeof(s.name)-1] = '\0';
                    s.isActive = (rs.isActive != 0);
                    s.averageGrade = rs.averageGrade;
                    s.cours = rs.cours;
                    res.push_back(s);
                    std::cout << "Successfully added record to results" << std::endl;
                }
            }
        } else {
            std::cout << "ID " << searchId << " not found in index" << std::endl;
        }
        return res;
    }
    
    std::cout << "Sequential search for field: " << field << " value: " << value << std::endl;
    
    fm.seekToBegin();
    StoredStudent rs;
    int recordsChecked = 0;
    
    while(fm.readNext((char*)&rs, sizeof(StoredStudent))){
        recordsChecked++;
        if(rs.isActive == 0) {
            continue;
        }
        
        std::cout << "Checking record #" << recordsChecked << " - ID: " << rs.id << ", Name: " << rs.name << std::endl;
        
        bool match = false;
        
        if(field == "name"){
            if(strcmp(value.c_str(), rs.name) == 0) {
                match = true;
                std::cout << "Name match found!" << std::endl;
            }
        } else if(field == "isActive"){
            bool val = (value == "1" || value == "true" || value == "True");
            if(rs.isActive == (val ? 1 : 0)) match = true;
        } else if(field == "averageGrade"){
            double v = std::stod(value);
            if(std::abs(rs.averageGrade - v) < 0.0001) match = true;
        } else if(field == "cours"){
            int v = std::stoi(value);
            if(rs.cours == v) match = true;
        }
        
        if(match){
            Student s;
            s.id = rs.id;
            strncpy(s.name, rs.name, sizeof(s.name));
            s.name[sizeof(s.name)-1] = '\0';
            s.isActive = (rs.isActive != 0);
            s.averageGrade = rs.averageGrade;
            s.cours = rs.cours;
            res.push_back(s);
            std::cout << "Record added to search results" << std::endl;
        }
    }
    
    std::cout << "Search completed. Checked " << recordsChecked << " records, found " << res.size() << " matches" << std::endl;
    return res;
}

size_t Database::deleteByField(const std::string &field, const std::string &value){
    size_t deleted = 0;
    
    if(field == "id"){
        int id = std::stoi(value);
        auto it = index.find(id);
        if(it == index.end()) return 0;
        if(markRecordDeleted(it->second)){
            index.erase(it);
            persistIndex();
            return 1;
        }
        return 0;
    }

    fm.seekToBegin();
    long long off = 0;
    StoredStudent rs;
    while (fm.readNext((char*)&rs, sizeof(StoredStudent))){
        if(rs.isActive == 0){ 
            off += sizeof(StoredStudent); 
            continue; 
        }
        
        bool match = false;
        if(field == "name"){
            if(strcmp(value.c_str(), rs.name) == 0) match = true;
        } else if(field == "isActive"){
            bool val = (value == "1" || value == "true" || value == "True");
            if(rs.isActive == (val ? 1 : 0)) match = true;
        } else if(field == "averageGrade"){
            double v = std::stod(value);
            if(std::abs(rs.averageGrade - v) < 0.0001) match = true;
        } else if(field == "cours"){
            int v = std::stoi(value);
            if(rs.cours == v) match = true;
        }
        
        if(match){
            if(markRecordDeleted(off)){
                auto it = index.find(rs.id);
                if(it != index.end()) {
                    index.erase(it);
                }
                deleted++;
            }
        }
        off += sizeof(StoredStudent);
    }
    
    if(deleted > 0) {
        persistIndex();
    }
    
    return deleted;
}

bool Database::editRecordByKey(int keyId, const Student &newS){
    auto it = index.find(keyId);
    if(it == index.end()) {return false;}
    StoredStudent rs;
    if(!readRecordAt(it->second, rs)) {return false;}
    if(rs.isActive==0) {return false;}
    StoredStudent ns;
    ns.id = newS.id;
    strncpy(ns.name, newS.name, sizeof(ns.name));
    ns.name[sizeof(ns.name)-1] = '\0';
    ns.isActive = newS.isActive ? 1 : 0;
    ns.averageGrade = newS.averageGrade;
    ns.cours = newS.cours;
    if(newS.id != keyId){
        if(index.find(newS.id) != index.end()){return false;}
        index.erase(it);
        index[newS.id] = it->second;
    }
    if(!fm.writeAt(it->second, (const char*)&ns, sizeof(ns))){return false;}
    persistIndex();
    return true;
}

bool Database::backup(const std::string &backupFile){
    if(!openFlag) {
        std::cout << "Database is not open for backup" << std::endl;
        return false;
    }
    

    if(!persistIndex()) {
        std::cout << "Failed to persist index for backup" << std::endl;
        return false;
    }
    
    std::cout << "Creating backup to: " << backupFile << std::endl;
    

    if(!fm.copyTo(backupFile)) {
        std::cout << "Failed to copy database file" << std::endl;
        return false;
    }

    std::string backupIdxFile = backupFile + ".idx";
    if(!FileManager::copyFile(idxFilename, backupIdxFile)) {
        std::cout << "Failed to copy index file" << std::endl;
        return false;
    }
    
    std::cout << "Backup completed successfully" << std::endl;
    return true;
}

bool Database::restoreFromBackup(const std::string &backupFile){
    close();
    
    std::cout << "Restoring from backup: " << backupFile << std::endl;
    struct stat buffer;
    if (stat(backupFile.c_str(), &buffer) != 0) {
        std::cout << "Backup file does not exist: " << backupFile << std::endl;
        return false;
    }

    std::string restoredName;
    size_t lastSlash = backupFile.find_last_of("/\\");
    std::string fileNameOnly = (lastSlash == std::string::npos) ? backupFile : backupFile.substr(lastSlash + 1);
    
    size_t lastDot = fileNameOnly.find_last_of('.');
    if (lastDot != std::string::npos) {
        restoredName = fileNameOnly.substr(0, lastDot) + "_restored.db";
    } else {
        restoredName = fileNameOnly + ".db";
    }
    
    dbFilename = restoredName;
    idxFilename = dbFilename + ".idx";
    
    std::cout << "Restoring to: " << dbFilename << std::endl;

    if(!FileManager::copyFile(backupFile, dbFilename)) {
        std::cout << "Failed to copy backup file to " << dbFilename << std::endl;
        return false;
    }
    

    std::string backupIdxFile = backupFile + ".idx";
    if (stat(backupIdxFile.c_str(), &buffer) == 0) {
        if(!FileManager::copyFile(backupIdxFile, idxFilename)) {
            std::cout << "Failed to copy index file, will rebuild index" << std::endl;
        } else {
            std::cout << "Index file copied successfully" << std::endl;
        }
    } else {
        std::cout << "No index file found, will rebuild index" << std::endl;
    }
    

    if(!open(dbFilename)) {
        std::cout << "Failed to open restored database" << std::endl;
        return false;
    }
    

    std::cout << "Rebuilding index..." << std::endl;
    index.clear();
    fm.seekToBegin();
    long long off = 0;
    StoredStudent rs;
    int recordsRebuilt = 0;
    
    while(fm.readNext((char*)&rs, sizeof(StoredStudent))){
        if(rs.isActive) {
            index[rs.id] = off;
            recordsRebuilt++;
        }
        off += sizeof(StoredStudent);
    }
    
    persistIndex();
    std::cout << "Index rebuilt with " << recordsRebuilt << " active records" << std::endl;
    std::cout << "Restore completed successfully" << std::endl;
    
    return true;
}

bool Database::exportCSV(const std::string &csvFile){
    if(!openFlag) return false;
    std::ofstream ofs(csvFile);
    if(!ofs) return false;
    ofs << "id,name,isActive,averageGrade,cours\n";
    fm.seekToBegin();
    StoredStudent rs;
    while(fm.readNext((char*)&rs, sizeof(StoredStudent))){
        if(rs.isActive==0) continue;
        ofs << rs.id << ",\"" << rs.name << "\"," << (int)rs.isActive << "," << rs.averageGrade << "," << rs.cours << "\n";
    }
    return true;
}

std::vector<Student> Database::getAll() {
    std::vector<Student> result;
    
    std::cout << "=== GET ALL RECORDS ===" << std::endl;
    std::cout << "Index size: " << index.size() << std::endl;
    
    fm.seekToBegin();
    StoredStudent rs;
    int recordsFound = 0;
    
    while(fm.readNext((char*)&rs, sizeof(StoredStudent))){
        if(rs.isActive != 0) {
            Student s;
            s.id = rs.id;
            strncpy(s.name, rs.name, sizeof(s.name));
            s.name[sizeof(s.name)-1] = '\0';
            s.isActive = (rs.isActive != 0);
            s.averageGrade = rs.averageGrade;
            s.cours = rs.cours;
            result.push_back(s);
            recordsFound++;
            std::cout << "Active record - ID: " << rs.id << ", Name: " << rs.name << std::endl;
        }
    }
    
    std::cout << "Total active records found: " << recordsFound << std::endl;
    return result;
}

bool Database::checkIntegrity() {
    if (!openFlag) return false;
    
    std::cout << "=== Database Integrity Check ===" << std::endl;
    std::cout << "Index entries: " << index.size() << std::endl;
    

    fm.seekToBegin();
    StoredStudent rs;
    long long offset = 0;
    int fileRecords = 0;
    int activeRecords = 0;
    
    while (fm.readNext((char*)&rs, sizeof(StoredStudent))) {
        fileRecords++;
        if (rs.isActive) {
            activeRecords++;
            std::cout << "Active record - ID: " << rs.id << ", Name: " << rs.name 
                      << ", Offset: " << offset << std::endl;
        }
        offset += sizeof(StoredStudent);
    }
    
    std::cout << "Total records in file: " << fileRecords << std::endl;
    std::cout << "Active records in file: " << activeRecords << std::endl;
    std::cout << "Records in index: " << index.size() << std::endl;
    
    return activeRecords == index.size();
}





