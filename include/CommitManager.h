#ifndef COMMITLIST_H
#define COMMITLIST_H

#include "CommitNode.h"
#include <string>

class CommitManager {
private:
    CommitNode* head;
    CommitNode* tail;



public:
    CommitManager();

    void loadListFromDisk();
    CommitNode* loadSingleNode(const string& id);

    void addCommit(const string& msg);
    void revert(const string& commitID);
    void printLog();

    CommitNode* getHead();
    CommitNode* getTail();

    ~CommitManager();
};

#endif
