#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <string>
#include <vector>
#include <filesystem>

using namespace std;

#define RED "\033[31m"
#define GRN "\033[32m"
#define YEL "\033[33m"
#define BLU "\033[34m"
#define MAG "\033[35m"
#define CYN "\033[36m"
#define WHT "\033[37m"
#define END "\033[0m"

namespace fs =   filesystem;

class Repository {
private:
    fs::path vcsRoot;        // .Minivcs/
    fs::path stagingArea;    // .Minivcs/staging_area/
    fs::path commitsDir;     // .Minivcs/commits/
    fs::path headFile;       // .Minivcs/HEAD.txt
    
    // Helper functions
    void addSingleFile(const   string& filepath);
    bool isVcsDirectory(const fs::path& path) const;
    void copyRecursive(const fs::path& src, const fs::path& dest);

public:
    Repository();
    
    void init();
    void add(const   vector<  string>& files);
    void addAll();
    void checkout(const   string& commitID);
    
    bool isInitialized() const;
    void clearStaging();
      vector<  string> getStagedFiles() const;
    bool isStagingEmpty() const;
    
    fs::path getVcsRoot() const;
    fs::path getStagingArea() const;
    fs::path getCommitsDir() const;
    
      string getHead() const;
    void setHead(const   string& commitID);
};

#endif