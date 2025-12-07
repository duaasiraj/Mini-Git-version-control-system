#include "HashTable.h"
#include <iostream>
#include <iomanip>

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
// Uses polynomial rolling hash for better distribution
//----------------------------------------------------------------------------------------------------------------------------

int HashTable::hashFunction(const string& commitID) const {
    if (commitID.empty()) {
        return 0;
    }

    unsigned long hash = 0;
    const unsigned long prime = 31; // Prime number for better distribution

    // Process each character in the commit ID
    for (char c : commitID) {
        hash = hash * prime + static_cast<unsigned long>(c);
    }

    // Ensure the hash is within table bounds (positive and within range)
    return static_cast<int>(hash % tableSize);
}

//----------------------------------------------------------------------------------------------------------------------------
// INSERT
// Inserts a commit node into the hash table
// Automatically resizes if load factor exceeds threshold
// Time Complexity: O(1) average case
//----------------------------------------------------------------------------------------------------------------------------

void HashTable::insert(const string& commitID, CommitNode* nodePtr) {
    if (commitID.empty() || nodePtr == nullptr) {
        return;
    }

    // Check if we need to resize
    if (getLoadFactor() >= loadFactorThreshold) {
        resize();
    }

    // Check if commit already exists (avoid duplicates)
    if (exists(commitID)) {
        return;
    }

    // Insert into the table
    insertIntoTable(table, tableSize, commitID, nodePtr);
    numElements++;
}

//----------------------------------------------------------------------------------------------------------------------------
// INSERT INTO TABLE (HELPER)
// Helper function used by both insert() and resize()
//----------------------------------------------------------------------------------------------------------------------------

void HashTable::insertIntoTable(ChainNode** targetTable, int targetSize,
                                 const string& commitID, CommitNode* nodePtr) {
    // Calculate hash for target table
    unsigned long hash = 0;
    const unsigned long prime = 31;
    for (char c : commitID) {
        hash = hash * prime + static_cast<unsigned long>(c);
    }
    int index = static_cast<int>(hash % targetSize);

    // Create new chain node
    ChainNode* newNode = new ChainNode(commitID, nodePtr);

    // Insert at the beginning of the chain (O(1))
    newNode->next = targetTable[index];
    targetTable[index] = newNode;
}

//----------------------------------------------------------------------------------------------------------------------------
// SEARCH
// Searches for a commit by ID and returns pointer to CommitNode
// Returns nullptr if not found
// Time Complexity: O(1) average case, O(n) worst case
//----------------------------------------------------------------------------------------------------------------------------

CommitNode* HashTable::search(const string& commitID) const {
    if (commitID.empty()) {
        return nullptr;
    }

    int index = hashFunction(commitID);
    ChainNode* current = table[index];

    // Traverse the chain at this index
    while (current != nullptr) {
        if (current->commitID == commitID) {
            return current->commitNodePtr;
        }
        current = current->next;
    }

    return nullptr; // Not found
}

//----------------------------------------------------------------------------------------------------------------------------
// EXISTS
// Checks if a commit exists in the hash table
// Time Complexity: O(1) average case
//----------------------------------------------------------------------------------------------------------------------------

bool HashTable::exists(const string& commitID) const {
    return search(commitID) != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------
// REMOVE
// Removes a commit from the hash table
// Returns true if successfully removed, false if not found
// Time Complexity: O(1) average case
//----------------------------------------------------------------------------------------------------------------------------

bool HashTable::remove(const string& commitID) {
    if (commitID.empty()) {
        return false;
    }

    int index = hashFunction(commitID);
    ChainNode* current = table[index];
    ChainNode* previous = nullptr;

    // Traverse the chain to find the node
    while (current != nullptr) {
        if (current->commitID == commitID) {
            // Found the node, remove it
            if (previous == nullptr) {
                // Node is at the head of the chain
                table[index] = current->next;
            } else {
                // Node is in the middle or end
                previous->next = current->next;
            }

            delete current;
            numElements--;
            return true;
        }
        previous = current;
        current = current->next;
    }

    return false; // Not found
}

//----------------------------------------------------------------------------------------------------------------------------
// RESIZE
// Doubles the table size and rehashes all elements
// Called automatically when load factor exceeds threshold
//----------------------------------------------------------------------------------------------------------------------------

void HashTable::resize() {
    int newSize = tableSize * 2;
    ChainNode** newTable = new ChainNode*[newSize];

    // Initialize new table
    for (int i = 0; i < newSize; i++) {
        newTable[i] = nullptr;
    }

    // Rehash all elements from old table to new table
    for (int i = 0; i < tableSize; i++) {
        ChainNode* current = table[i];
        while (current != nullptr) {
            ChainNode* next = current->next;

            // Calculate new hash for the new table size
            unsigned long hash = 0;
            const unsigned long prime = 31;
            for (char c : current->commitID) {
                hash = hash * prime + static_cast<unsigned long>(c);
            }
            int newIndex = static_cast<int>(hash % newSize);

            // Insert into new table
            current->next = newTable[newIndex];
            newTable[newIndex] = current;

            current = next;
        }
    }

    // Delete old table array (but not the chain nodes, they're in new table)
    delete[] table;

    // Update table pointer and size
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

//----------------------------------------------------------------------------------------------------------------------------
// PRINT STATISTICS
// Prints hash table statistics for debugging purposes
//----------------------------------------------------------------------------------------------------------------------------

void HashTable::printStatistics() const {
    cout << "========================================" << endl;
    cout << "Hash Table Statistics:" << endl;
    cout << "========================================" << endl;
    cout << "Table Size: " << tableSize << endl;
    cout << "Number of Elements: " << numElements << endl;
    cout << "Load Factor: " << fixed << setprecision(2) << getLoadFactor() << endl;

    // Count chains and find longest chain
    int emptySlots = 0;
    int maxChainLength = 0;
    int totalChainLength = 0;

    for (int i = 0; i < tableSize; i++) {
        int chainLength = 0;
        ChainNode* current = table[i];

        if (current == nullptr) {
            emptySlots++;
        } else {
            while (current != nullptr) {
                chainLength++;
                current = current->next;
            }
            totalChainLength += chainLength;
            if (chainLength > maxChainLength) {
                maxChainLength = chainLength;
            }
        }
    }

    cout << "Empty Slots: " << emptySlots << endl;
    cout << "Longest Chain: " << maxChainLength << endl;
    if (numElements > 0) {
        cout << "Average Chain Length: " << fixed << setprecision(2)
             << (static_cast<double>(totalChainLength) / (tableSize - emptySlots)) << endl;
    }
    cout << "========================================" << endl;
}