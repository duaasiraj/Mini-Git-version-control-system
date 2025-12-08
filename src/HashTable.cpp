#include "HashTable.h"
#include <iostream>

using namespace std;

//----------------------------------------------------------------------------------------------------------------------------
// CONSTRUCTOR
// Initializes hash table with specified size (default 50)
// All chain pointers are set to nullptr
//----------------------------------------------------------------------------------------------------------------------------

HashTable::HashTable(int initialSize) {
    tableSize = initialSize;
    numElements = 0;
    loadFactorThreshold = 0.75;

    table = new ChainNode*[tableSize];
    for (int i = 0; i < tableSize; i++) {
        table[i] = nullptr;
    }
}

//----------------------------------------------------------------------------------------------------------------------------
// DESTRUCTOR
// Cleans up all chains and the table array
//----------------------------------------------------------------------------------------------------------------------------

HashTable::~HashTable() {
    for (int i = 0; i < tableSize; i++) {
        ChainNode* current = table[i];
        while (current != nullptr) {
            ChainNode* temp = current;
            current = current->next;
            delete temp;
        }
    }
    delete[] table;
}

//----------------------------------------------------------------------------------------------------------------------------
// HASH FUNCTION
// Uses all characters in the commit ID to generate a hash
// We use a prime number to allow for greater distribution and variety
//we take the mod of the hash with the size of the table to ensure the index is always in bounds
//----------------------------------------------------------------------------------------------------------------------------

int HashTable::hashFunction(const string& commitID) const {
    if (commitID.empty()) {
        return 0;
    }

    unsigned long hash = 0;
    const unsigned long prime = 31;


    for (char c : commitID) {
        hash = hash * prime + static_cast<unsigned long>(c);
    }


    return static_cast<int>(hash % tableSize);
}

//----------------------------------------------------------------------------------------------------------------------------
// INSERT
//first we validate the commit node we're inserting, then we check if we need to rehash. then we check if the id already exists in table
//once all clear, we insert into the table
//----------------------------------------------------------------------------------------------------------------------------

void HashTable::insert(const string& commitID, CommitNode* nodePtr) {
    if (commitID.empty() || nodePtr == nullptr) {
        return;
    }

    if (getLoadFactor() >= loadFactorThreshold) {
        resize();
    }

    if (exists(commitID)) {
        return;
    }

    insertIntoTable(table, tableSize, commitID, nodePtr);
    numElements++;
}

//----------------------------------------------------------------------------------------------------------------------------
// INSERT INTO TABLE (HELPER)
// We made this helper function as we need to insert various times (during regular insertion and rehashing)
//we tried doing direct insertion but there seemed to be some sort of ambiguity while inserting, so we decided to create a seperate function
//----------------------------------------------------------------------------------------------------------------------------

void HashTable::insertIntoTable(ChainNode** targetTable, int targetSize, const string& commitID, CommitNode* nodePtr) {

    unsigned long hash = 0;
    const unsigned long prime = 31;
    for (char c : commitID) {
        hash = hash * prime + static_cast<unsigned long>(c);
    }

    int index = static_cast<int>(hash % targetSize);

    ChainNode* newNode = new ChainNode(commitID, nodePtr);

    newNode->next = targetTable[index];
    targetTable[index] = newNode;
}

//----------------------------------------------------------------------------------------------------------------------------
// SEARCH
// Searches for a commit by ID and returns pointer to CommitNode
// Returns nullptr if not found
// (O(n) would occur if somehow we got a really long chain )
//----------------------------------------------------------------------------------------------------------------------------

CommitNode* HashTable::search(const string& commitID) const {
    if (commitID.empty()) {
        return nullptr;
    }

    int index = hashFunction(commitID);
    ChainNode* current = table[index];

    while (current != nullptr) {
        if (current->commitID == commitID) {
            return current->commitNodePtr;
        }
        current = current->next;
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------
// EXISTS
// Checks if a commit exists in the hash table
//----------------------------------------------------------------------------------------------------------------------------

bool HashTable::exists(const string& commitID) const {
    return search(commitID) != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------
// REMOVE
// Removes a commit from the hash table
// we use the two pointer approach for deleting nodes in a given chain
//----------------------------------------------------------------------------------------------------------------------------

bool HashTable::remove(const string& commitID) {
    if (commitID.empty()) {
        return false;
    }

    int index = hashFunction(commitID);
    ChainNode* current = table[index];
    ChainNode* prev = nullptr;

    while (current != nullptr) {
        if (current->commitID == commitID) {
            if (prev == nullptr) {
                table[index] = current->next;
            } else {
                prev->next = current->next;
            }

            delete current;
            numElements--;
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------------
// RESIZE
// Doubles the table size and rehashes all elements
// This function gets called whenever load factor is exceeded
//----------------------------------------------------------------------------------------------------------------------------

void HashTable::resize() {

    int newSize = tableSize * 2;
    ChainNode** newTable = new ChainNode*[newSize];

    for (int i = 0; i < newSize; i++) {
        newTable[i] = nullptr;
    }

    for (int i = 0; i < tableSize; i++) {
        ChainNode* current = table[i];
        while (current != nullptr) {
            ChainNode* next = current->next;

            unsigned long hash = 0;
            const unsigned long prime = 31;

            for (char c : current->commitID) {
                hash = hash * prime + static_cast<unsigned long>(c);
            }
            int newIndex = static_cast<int>(hash % newSize);

            current->next = newTable[newIndex];
            newTable[newIndex] = current;

            current = next;
        }
    }

    delete[] table;

    table = newTable;
    tableSize = newSize;
}

//----------------------------------------------------------------------------------------------------------------------------
// GET LOAD FACTOR
// Returns the current load factor (numElements / tableSize)
//----------------------------------------------------------------------------------------------------------------------------

double HashTable::getLoadFactor() const {
    return static_cast<double>(numElements) / tableSize;
}

//----------------------------------------------------------------------------------------------------------------------------
// SIZE
// Returns the number of elements in the hash table
//----------------------------------------------------------------------------------------------------------------------------

int HashTable::size() const {
    return numElements;
}

//----------------------------------------------------------------------------------------------------------------------------
// CAPACITY
// Returns the current capacity (table size) of the hash table
//----------------------------------------------------------------------------------------------------------------------------

int HashTable::capacity() const {
    return tableSize;
}

