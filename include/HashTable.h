#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include "CommitNode.h"

using namespace std;

// Chain node for handling collisions
struct ChainNode {
    string commitID;
    CommitNode* commitNodePtr;
    ChainNode* next;

    ChainNode(const string& id, CommitNode* ptr)
        : commitID(id), commitNodePtr(ptr), next(nullptr) {}
};

class HashTable {
private:
    ChainNode** table;      // Array of chain pointers
    int tableSize;          // Current size of hash table
    int numElements;        // Number of elements stored
    double loadFactorThreshold; // Threshold for resizing (default 0.75)

    // Hash function: uses all characters in commit ID
    int hashFunction(const string& commitID) const;

    // Helper function to resize and rehash when load factor exceeds threshold
    void resize();

    // Helper to insert into a specific table (used during resize)
    void insertIntoTable(ChainNode** targetTable, int targetSize,
                         const string& commitID, CommitNode* nodePtr);

public:
    // Constructor: initializes hash table with default size of 50
    HashTable(int initialSize = 50);

    // Destructor: cleans up all chains
    ~HashTable();

    // Insert a commit node into the hash table
    void insert(const string& commitID, CommitNode* nodePtr);

    // Search for a commit by ID - O(1) average case
    CommitNode* search(const string& commitID) const;

    // Check if a commit exists - O(1) average case
    bool exists(const string& commitID) const;

    // Remove a commit from hash table
    bool remove(const string& commitID);

    // Get current load factor
    double getLoadFactor() const;

    // Get number of elements
    int size() const;

    // Get table size
    int capacity() const;

};



#endif //MINIGIT_HASHTABLE_H
