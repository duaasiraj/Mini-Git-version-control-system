#ifndef COMMITMANAGER_H
#define COMMITMANAGER_H

#include "CommitNode.h"
#include "HashTable.h"
#include <string>

class CommitManager {
private:
    CommitNode* head;
    CommitNode* tail;

    HashTable* hashTable;


public:
    CommitManager();

    void loadListFromDisk();
    CommitNode* loadSingleNode(const string& id);

    void addCommit(const string& msg);
    void revert(const string& commitID);
    void printLog();

    CommitNode* getHead();
    CommitNode* getTail();

    bool commitExists(const string& commitID);

    ~CommitManager();
};

#endif
