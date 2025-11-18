#include "Repository.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace std;

Repository::Repository() {
    vcsRoot = fs::current_path() / ".Minivcs";
    stagingArea = vcsRoot / "staging_area";
    commitsDir = vcsRoot / "commits";
    headFile = vcsRoot / "HEAD.txt";
}

void Repository::init() {
    try {
        if (isInitialized()) {
            cout << YEL << "Repository already initialized in " << vcsRoot << END << endl;
            return;
        }

        fs::create_directories(stagingArea);
        fs::create_directories(commitsDir);

        // Create HEAD file pointing to no commit initially
        ofstream head(headFile);
        if (!head) {
            throw runtime_error("Failed to create HEAD file");
        }
        head << "NA";
        head.close();

        cout << GRN << "Initialized empty Minivcs repository in "
             << vcsRoot << END << endl;

    } catch (const fs::filesystem_error& e) {
        cerr << RED << "Error initializing repository: " << e.what() << END << endl;
        throw;
    }
}

bool Repository::isInitialized() const {
    return fs::exists(vcsRoot) &&
           fs::is_directory(vcsRoot) &&
           fs::exists(stagingArea) &&
           fs::exists(commitsDir) &&
           fs::exists(headFile);
}

void Repository::add(const vector<string>& files) {
    if (!isInitialized()) {
        cerr << RED << "fatal: not a Minivcs repository (or any parent up to mount point /)"
             << END << endl;
        cerr << YEL << "Hint: Use 'init' to create a repository" << END << endl;
        return;
    }

    if (files.empty()) {
        cout << YEL << "Nothing specified, nothing added." << END << endl;
        return;
    }

    int successCount = 0;
    int failCount = 0;

    for (const auto& file : files) {

        try {
            addSingleFile(file);
            cout << GRN << "add '" << file << "'" << END << endl;
            successCount++;
        } catch (const exception& e) {
            cerr << RED << "error: '" << file << "': " << e.what() << END << endl;
            failCount++;
        }
    }

    if (successCount > 0) {
        cout << GRN << "Successfully added " << successCount << " file(s)" << END << endl;
    }
}

void Repository::addSingleFile(const string& filepath) {
    fs::path sourcePath = fs::current_path() / filepath;

    // Check if file exists
    if (!fs::exists(sourcePath)) {
        throw runtime_error("pathspec '" + filepath + "' did not match any files");
    }

    // Don't allow adding the .Minivcs directory itself
    if (isVcsDirectory(sourcePath)) {
        throw runtime_error("cannot add '.Minivcs' directory");
    }

    // Compute destination path preserving directory structure
    fs::path destPath = stagingArea / filepath;

    // Create parent directories if needed
    if (destPath.has_parent_path()) {
        fs::create_directories(destPath.parent_path());
    }

    // Copy file or directory
    if (fs::is_directory(sourcePath)) {
        copyRecursive(sourcePath, destPath);
    } else {
        fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
    }
}

void Repository::addAll() {
    if (!isInitialized()) {
        cerr << RED << "fatal: not a Minivcs repository" << END << endl;
        return;
    }

    vector<string> allFiles;

    // Collect all files in current directory (non-recursively at top level)
    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        string filename = entry.path().filename().string();

        // Skip VCS directories and hidden files
        if (filename == ".Minivcs" || filename == ".git" || filename[0] == '.') {
            continue;
        }

        allFiles.push_back(filename);
    }

    if (allFiles.empty()) {
        cout << YEL << "No files to add" << END << endl;
        return;
    }

    cout << BLU << "Adding all files..." << END << endl;
    add(allFiles);
}

void Repository::copyRecursive(const fs::path& src, const fs::path& dest) {

    for (auto &part : src) {
        if (part == ".Minivcs" || part == ".git") {
            return;  // do not enter this directory
        }
    }

    if (fs::is_directory(src)) {
        fs::create_directories(dest);

        for (const auto& entry : fs::directory_iterator(src)) {
            fs::path srcPath = entry.path();
            fs::path destPath = dest / srcPath.filename();

            copyRecursive(srcPath, destPath);
        }
    } else {
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
    }
}

bool Repository::isVcsDirectory(const fs::path& path) const {
    string pathStr = path.string();
    return pathStr.find(".Minivcs") != string::npos ||
           pathStr.find(".git") != string::npos;
}

void Repository::clearStaging() {
    if (!isInitialized()) {
        return;
    }

    try {
        if (fs::exists(stagingArea)) {
            for (const auto& entry : fs::directory_iterator(stagingArea)) {
                fs::remove_all(entry);
            }
            cout << GRN << "Staging area cleared" << END << endl;
        }
    } catch (const fs::filesystem_error& e) {
        cerr << RED << "Error clearing staging area: " << e.what() << END << endl;
    }
}

vector<string> Repository::getStagedFiles() const {
    vector<string> stagedFiles;

    if (!isInitialized() || !fs::exists(stagingArea)) {
        return stagedFiles;
    }

    try {
        for (const auto& entry : fs::recursive_directory_iterator(stagingArea)) {
            if (fs::is_regular_file(entry)) {
                fs::path relativePath = fs::relative(entry.path(), stagingArea);
                stagedFiles.push_back(relativePath.string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        cerr << RED << "Error reading staged files: " << e.what() << END << endl;
    }

    return stagedFiles;
}

bool Repository::isStagingEmpty() const {
    if (!isInitialized() || !fs::exists(stagingArea)) {
        return true;
    }

    return fs::is_empty(stagingArea);
}

fs::path Repository::getVcsRoot() const {
    return vcsRoot;
}

fs::path Repository::getStagingArea() const {
    return stagingArea;
}

fs::path Repository::getCommitsDir() const {
    return commitsDir;
}

string Repository::getHead() const {
    if (!fs::exists(headFile)) {
        return "NA";
    }

    ifstream head(headFile);
    string commitID;
    getline(head, commitID);
    head.close();

    return commitID;
}

void Repository::setHead(const string& commitID) {
    ofstream head(headFile);
    if (!head) {
        throw runtime_error("Failed to update HEAD");
    }
    head << commitID;
    head.close();
}

void Repository::checkout(const string& commitID) {
    if (!isInitialized()) {
        cerr << RED << "fatal: not a Minivcs repository" << END << endl;
        return;
    }

    fs::path commitPath = commitsDir / commitID;
    fs::path commitDataPath = commitPath / "Data";

    if (!fs::exists(commitDataPath)) {
        cerr << RED << "fatal: commit '" << commitID << "' does not exist" << END << endl;
        return;
    }

    try {
        // Copy all files from commit's Data folder to working directory
        for (const auto& entry : fs::recursive_directory_iterator(commitDataPath)) {
            fs::path relativePath = fs::relative(entry.path(), commitDataPath);
            fs::path destPath = fs::current_path() / relativePath;

            if (fs::is_directory(entry)) {
                fs::create_directories(destPath);
            } else {
                // Create parent directories if needed
                if (destPath.has_parent_path()) {
                    fs::create_directories(destPath.parent_path());
                }

                // FIXED: Remove existing file first, then copy
                // This solves the Windows "File exists" error
                if (fs::exists(destPath)) {
                    fs::remove(destPath);
                }

                fs::copy_file(entry, destPath);
            }
        }

        // Update HEAD to point to this commit
        setHead(commitID);

        cout << GRN << "Checked out commit: " << commitID << END << endl;

    } catch (const fs::filesystem_error& e) {
        cerr << RED << "Error during checkout: " << e.what() << END << endl;
        throw;
    }
}