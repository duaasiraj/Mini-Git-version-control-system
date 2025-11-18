#ifndef COMMITLIST_H
#define COMMITLIST_H

#include "CommitNode.h"
#include <string>

class CommitManager {
private:
    CommitNode* head;
    CommitNode* tail;

    void loadListFromDisk();
    CommitNode* loadSingleNode(const std::string& id);

public:
    CommitManager();

    void addCommit(const std::string& msg);
    void revert(const std::string& commitID);
    void printLog() const;

    CommitNode* getHead() const { return head; }
    CommitNode* getTail() const { return tail; }
};

#endif
