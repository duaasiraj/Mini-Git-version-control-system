#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include "CommitNode.h"

using namespace std;

struct ChainNode {
    string commitID;
    CommitNode* commitNodePtr;
    ChainNode* next;

    ChainNode(const string& id, CommitNode* ptr): commitID(id), commitNodePtr(ptr), next(nullptr) {}
};

class HashTable {
private:
    ChainNode** table;
    int tableSize;
    int numElements;
    double loadFactorThreshold;

    int hashFunction(const string& commitID) const;

    void resize();

    void insertIntoTable(ChainNode** targetTable, int targetSize, const string& commitID, CommitNode* nodePtr);

public:
    HashTable(int initialSize = 50);

    ~HashTable();

    void insert(const string& commitID, CommitNode* nodePtr);

    CommitNode* search(const string& commitID) const;

    bool exists(const string& commitID) const;

    bool remove(const string& commitID);

    double getLoadFactor() const;

    int size() const;

    int capacity() const;

};



#endif
