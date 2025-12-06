#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "CommitNode.h"
#include <string>
#include <vector>

using namespace std;

struct HashEntry {
    string commitID;
    CommitNode* node;
    HashEntry* next;

    HashEntry(const string& id, CommitNode* n){
        commitID = id;
        node = n;
        next = nullptr;
    }
};

class HashTable {

public:

    vector<HashEntry*> table;
    int capacity;
    int numElements;

    const double LOAD_FACTOR_THRESHOLD = 1.0;



    int hashFunction(const string& commitID) const;

    void rehash();

    void deleteChain(HashEntry* head);

    int getChainLength(HashEntry* head) const;

    HashTable(int initialCapacity = 100);

    ~HashTable();

    void insert(const string& commitID, CommitNode* node);

    CommitNode* search(const string& commitID) const;

    bool remove(const string& commitID);

    bool contains(const string& commitID) const;

    int size() const;

    int getCapacity() const;

    double getLoadFactor() const;

    int getMaxChainLength() const;

    double getAvgChainLength() const;

    void printStats() const;

    void displayAll();

    void clear();
};


#endif //HASHTABLE_H