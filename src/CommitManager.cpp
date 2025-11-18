#include "CommitManager.h"
#include "HashingHelper.h"
#include "HashingHelper.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;
namespace fs = std::filesystem;

// Helper: read file into string
static string readFile(const fs::path& p) {
    if (!fs::exists(p)) return "NA";
    ifstream file(p);
    string s;
    getline(file, s);
    return s;
}

// --------------------
// Constructor
// --------------------
CommitManager::CommitManager() {
    head = nullptr;
    tail = nullptr;

    fs::path repo = fs::current_path() / ".Minivcs" / "commits";
    if (!fs::exists(repo)) return;  // repo doesn't exist yet

    loadListFromDisk();
}


// --------------------
// Load list from disk
// --------------------
void CommitManager::loadListFromDisk() {

    fs::path commits = fs::current_path() / ".Minivcs" / "commits";

    string headID = readFile(commits / "HEAD.txt");
    string tailID = readFile(commits / "TAIL.txt");

    if (headID == "NA" || tailID == "NA") {
        return; // nothing to load
    }

    head = loadSingleNode(headID);
    tail = loadSingleNode(tailID);

    // Now link nodes from tail → head
    CommitNode* current = tail;

    while (current && current->getNextID() != "NA") {
        CommitNode* next = loadSingleNode(current->getNextID());
        current->setNextNode(next);
        next->setPrevNode(current);
        current = next;
    }

    head = current;
}


// --------------------
// Load a single commit node
// --------------------
CommitNode* CommitManager::loadSingleNode(const string& id) {
    try {
        return new CommitNode(id);   // node loads its own info files
    }
    catch (...) {
        return nullptr;
    }
}


// -----------------------------
// Add a new commit
// -----------------------------
void CommitManager::addCommit(const string& msg) {

    string id = HASHINGHELPER_H::generateCommitID();      // your hash generator
    CommitNode* newNode = new CommitNode(id, msg);  // creates dir, copies staging

    if (head == nullptr) {
        // first commit in repo
        head = tail = newNode;

        ofstream f1(fs::current_path() / ".Minivcs" / "commits" / "HEAD.txt");
        ofstream f2(fs::current_path() / ".Minivcs" / "commits" / "TAIL.txt");
        f1 << id;
        f2 << id;
        return;
    }

    // link from old tail to new commit
    tail->setNextNode(newNode);
    tail->setNextID(newNode->getCommitID());
    tail->saveNextID(newNode->getCommitID());

    // link new tail backwards
    newNode->setPrevNode(tail);
    newNode->setPrevID(tail->getCommitID());
    newNode->savePrevID(tail->getCommitID());

    // update tail pointer
    tail = newNode;

    // update HEAD.txt
    ofstream f(fs::current_path() / ".Minivcs" / "commits" / "HEAD.txt");
    f << newNode->getCommitID();
}


// -----------------------------
// REVERT
// -----------------------------
void CommitManager::revert(const string& commitID) {

    fs::path commitPath = fs::current_path() / ".Minivcs" / "commits" / commitID;

    if (!fs::exists(commitPath)) {
        cout << "Commit not found.\n";
        return;
    }

    fs::path oldData = commitPath / "Data";
    fs::path staging = fs::current_path() / ".Minivcs" / "staging_area";

    // ----------------------------------------------------
    // (1) CLEAR STAGING AREA
    // ----------------------------------------------------
    for (auto& entry : fs::directory_iterator(staging))
        fs::remove_all(entry);

    // ----------------------------------------------------
    // (2) COPY OLD COMMIT → STAGING AREA
    // ----------------------------------------------------
    for (auto& entry : fs::recursive_directory_iterator(oldData)) {
        fs::path rel = fs::relative(entry.path(), oldData);
        fs::path dest = staging / rel;

        if (fs::is_directory(entry)) {
            fs::create_directories(dest);
        } else {
            fs::create_directories(dest.parent_path());
            if (fs::exists(dest)) fs::remove(dest);
            fs::copy_file(entry.path(), dest);
        }
    }

    // ----------------------------------------------------
    // (3) CREATE A NEW COMMIT BASED ON OLD STATE
    // ----------------------------------------------------
    addCommit("Revert to " + commitID);

    string newID;
    {
        ifstream h(fs::current_path() / ".Minivcs" / "commits" / "HEAD.txt");
        getline(h, newID);
    }

    // ----------------------------------------------------
    // (4) CLEAN WORKING DIRECTORY (remove files not in commit)
    // ----------------------------------------------------
    fs::path newData = fs::current_path() / ".Minivcs" / "commits" / newID / "Data";
    fs::path working = fs::current_path();

    // remove files that don't exist in reverted commit
    for (auto& entry : fs::directory_iterator(working)) {
        string name = entry.path().filename().string();

        if (name == ".Minivcs") continue; // don't delete repo

        fs::path equivalent = newData / name;
        if (!fs::exists(equivalent)) {
            fs::remove_all(entry);
        }
    }

    // ----------------------------------------------------
    // (5) COPY NEW DATA → WORKING DIR (overwrite safe)
    // ----------------------------------------------------
    for (auto& entry : fs::recursive_directory_iterator(newData)) {
        fs::path rel = fs::relative(entry.path(), newData);
        fs::path dest = working / rel;

        if (fs::is_directory(entry)) {
            fs::create_directories(dest);
        } else {
            fs::create_directories(dest.parent_path());
            if (fs::exists(dest)) fs::remove(dest);
            fs::copy_file(entry.path(), dest);
        }
    }

    cout << "Revert complete. Created commit: " << newID << "\n";
}


// -----------------------------
// LOG FUNCTION
// -----------------------------
void CommitManager::printLog() const {

    if (!tail) {
        cout << "No commits found." << endl;
        return;
    }

    CommitNode* cur = head;

    while (cur) {
        // Load timestamp from info.txt
        fs::path infoPath = fs::current_path()/
                            ".Minivcs"/"commits"/cur->getCommitID()/"info.txt";

        ifstream f(infoPath.string());
        string line;
        string timestamp;

        while (getline(f, line)) {
            if (line.find("3. DATE & TIME OF COMMIT: ") == 0)
                timestamp = line.substr(strlen("3. DATE & TIME OF COMMIT: "));
        }

        cout << "Commit: " << cur->getCommitID() << endl;
        cout << "Message: " << cur->getCommitMsg() << endl;
        cout << "Date: " << timestamp << endl;
        cout << "------------------------------------" << endl;

        cur = cur->getPrevNode();
    }
}
