#include "HashTable.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

/*
============================================================================
CONSTRUCTOR
============================================================================
Initializes the hash table with the specified capacity
All bucket pointers start as nullptr (empty chains)
*/
HashTable::HashTable(int initialCapacity) {
    capacity = initialCapacity;
    numElements = 0;
    table.resize(capacity, nullptr);  // Initialize all buckets to nullptr
}

/*
============================================================================
DESTRUCTOR
============================================================================
Deletes all HashEntry nodes in all chains
Note: Does NOT delete CommitNode objects (managed by CommitManager)
*/
HashTable::~HashTable() {
    clear();
}

/*
============================================================================
DELETE CHAIN HELPER
============================================================================
Deletes all nodes in a linked list chain
Used by destructor and clear()

Process:
1. Traverse the chain
2. Delete each HashEntry node
3. Move to next node

Example:
Chain: [A] -> [B] -> [C] -> nullptr
Step 1: Delete A, move to B
Step 2: Delete B, move to C
Step 3: Delete C, done
*/
void HashTable::deleteChain(HashEntry* head) {
    while (head != nullptr) {
        HashEntry* temp = head;
        head = head->next;
        delete temp;  // Delete the HashEntry (not the CommitNode!)
    }
}

/*
============================================================================
GET CHAIN LENGTH HELPER
============================================================================
Returns the length of a chain (number of nodes)
Used for statistics
*/
int HashTable::getChainLength(HashEntry* head) const {
    int length = 0;
    while (head != nullptr) {
        length++;
        head = head->next;
    }
    return length;
}

/*
============================================================================
HASH FUNCTION
============================================================================
Purpose: Convert a commit ID (hex string) into a bucket index

How it works:
1. Extract first 8 characters from the commit ID
   - CommitIDs are 16-character hex strings (64 bits)
   - We use first 8 chars (32 bits) for hashing
   
2. Convert hex string to unsigned long long
   - Example: "a3f2b1c4" becomes 0xa3f2b1c4
   
3. Apply modulo to get bucket index
   - Ensures: 0 <= index < capacity

Example:
commitID = "a3f2b1c4d5e6f7a8"
prefix = "a3f2b1c4"
hashValue = 2751463876 (decimal)
bucketIndex = 2751463876 % 100 = 76
*/
int HashTable::hashFunction(const string& commitID) const {
    if (commitID.empty() || commitID == "NA") {
        return 0;
    }
    
    // Take first 8 characters (32 bits) of the hex string
    string prefix = commitID.substr(0, min(8, (int)commitID.length()));
    
    // Convert hex string to unsigned long long
    unsigned long long hashValue = 0;
    
    try {
        hashValue = stoull(prefix, nullptr, 16);  // Base 16 (hexadecimal)
    } catch (...) {
        // If conversion fails, fall back to simple character sum
        for (char c : prefix) {
            hashValue += static_cast<unsigned long long>(c);
        }
    }
    
    // Return bucket index within table bounds
    return hashValue % capacity;
}

/*
============================================================================
REHASHING - CRITICAL FOR PERFORMANCE!
============================================================================
Purpose: Maintain O(1) performance by keeping chains short

WHY DO WE NEED REHASHING WITH CHAINING?

Without rehashing example:
- Start with 100 buckets
- Add 1000 commits
- Load factor = 1000/100 = 10
- Average chain length = 10
- Search requires traversing ~10 nodes on average
- Performance: O(10) instead of O(1) - BAD!

With rehashing:
- Start with 100 buckets
- Add commits until load factor hits 1.0 (100 commits)
- REHASH: Double buckets to 200
- Continue adding
- At 200 commits: load factor = 200/200 = 1.0
- REHASH: Double buckets to 400
- Result: Average chain length always stays ≈ 1
- Performance: O(1) maintained - GOOD!

Process:
1. Create new table with double capacity
2. Iterate through old table's chains
3. For each entry, recalculate its bucket in new table
4. Insert into new table (chains will be redistributed)
5. Delete old table

Example:
Old table (capacity 4):
  Bucket 0: [A] -> nullptr
  Bucket 1: [B] -> [C] -> nullptr  (both hashed to 1)
  Bucket 2: nullptr
  Bucket 3: [D] -> nullptr

After rehash (capacity 8):
  Bucket 0: [A] -> nullptr
  Bucket 1: [B] -> nullptr  (now separate!)
  Bucket 2: nullptr
  Bucket 3: [D] -> nullptr
  Bucket 4: nullptr
  Bucket 5: [C] -> nullptr  (moved to different bucket!)
  Bucket 6: nullptr
  Bucket 7: nullptr

Result: B and C are no longer in the same chain!
Searches are faster because chains are shorter.

TIME COMPLEXITY:
- Rehashing itself: O(n) to process all n elements
- But it happens rarely (only when doubling)
- Amortized cost: O(1) per insertion
*/
void HashTable::rehash() {
    int oldCapacity = capacity;
    vector<HashEntry*> oldTable = table;
    
    // Double the capacity
    capacity = capacity * 2;
    table.clear();
    table.resize(capacity, nullptr);
    
    // Reset counter (will be incremented during reinsertion)
    numElements = 0;
    
    // Reinsert all elements from old table
    for (int i = 0; i < oldCapacity; i++) {
        HashEntry* current = oldTable[i];
        
        // Traverse the chain at this bucket
        while (current != nullptr) {
            // Save next pointer before reinserting
            HashEntry* next = current->next;
            
            // Reinsert this entry (will be placed in new bucket)
            insert(current->commitID, current->node);
            
            // Delete the old HashEntry (insert creates a new one)
            delete current;
            
            // Move to next in old chain
            current = next;
        }
    }
    
    cout << "Hash table rehashed: " << oldCapacity << " -> " << capacity 
         << " (chains redistributed for better performance)" << endl;
}

/*
============================================================================
INSERT
============================================================================
Purpose: Add a new commit to the hash table using separate chaining

Process:
1. Check if rehashing is needed (load factor > 1.0)
2. Calculate bucket index using hash function
3. Check if entry already exists in that bucket's chain
   - If yes, update the node pointer
   - If no, create new HashEntry and add to front of chain
4. Increment element counter

Why add to front of chain?
- O(1) insertion (no need to traverse to end)
- Most recently added items are found first
- Simpler code

Time Complexity: O(1) average case
- Hash calculation: O(1)
- Check for duplicates: O(k) where k = average chain length ≈ 1
- Insertion at front: O(1)

Example:
insert("a3f2b1c4...", nodePtr)

Before:
Bucket 76: [CommitX] -> [CommitY] -> nullptr

After:
Bucket 76: [a3f2b1c4] -> [CommitX] -> [CommitY] -> nullptr
           ^
           New entry added to front!
*/
void HashTable::insert(const string& commitID, CommitNode* node) {
    if (commitID.empty() || commitID == "NA" || node == nullptr) {
        return;
    }
    
    // Check if rehashing is needed
    if (getLoadFactor() > LOAD_FACTOR_THRESHOLD) {
        rehash();
    }
    
    // Calculate bucket index
    int index = hashFunction(commitID);
    
    // Check if this commitID already exists in the chain
    HashEntry* current = table[index];
    while (current != nullptr) {
        if (current->commitID == commitID) {
            // Update existing entry
            current->node = node;
            return;
        }
        current = current->next;
    }
    
    // Create new entry and add to front of chain
    HashEntry* newEntry = new HashEntry(commitID, node);
    newEntry->next = table[index];  // Point to current head
    table[index] = newEntry;         // New entry becomes head
    numElements++;
}

/*
============================================================================
SEARCH
============================================================================
Purpose: Find a commit by its ID

Process:
1. Calculate bucket index using hash function
2. Traverse the chain at that bucket
3. Compare each entry's commitID
4. Return pointer to CommitNode if found
5. Return nullptr if not found

Time Complexity: O(1) average case
- Hash calculation: O(1)
- Chain traversal: O(k) where k = average chain length
- With rehashing, k stays ≈ 1, so overall O(1)

Example:
search("a3f2b1c4...")

Bucket 76: [CommitA] -> [CommitB] -> [a3f2b1c4] -> [CommitC] -> nullptr
           Check A (no) -> Check B (no) -> Check a3f2b1c4 (YES!)
           Return node pointer
*/
CommitNode* HashTable::search(const string& commitID) const {
    if (commitID.empty() || commitID == "NA") {
        return nullptr;
    }
    
    // Calculate bucket index
    int index = hashFunction(commitID);
    
    // Traverse the chain at this bucket
    HashEntry* current = table[index];
    while (current != nullptr) {
        if (current->commitID == commitID) {
            return current->node;
        }
        current = current->next;
    }
    
    return nullptr;  // Not found
}

/*
============================================================================
REMOVE - SIMPLE DELETION WITH CHAINING!
============================================================================
Purpose: Delete a commit from the hash table

With separate chaining, deletion is SIMPLE:
1. Calculate bucket index
2. Traverse chain to find entry
3. Update pointers to remove entry from chain
4. Delete the HashEntry node
5. Done!

No rehashing of other entries needed!
No tombstones!
No complex logic!

Time Complexity: O(1) average case
- Hash calculation: O(1)
- Find in chain: O(k) where k ≈ 1
- Removal: O(1)

Example:
remove("CommitB")

Before:
Bucket 76: [CommitA] -> [CommitB] -> [CommitC] -> nullptr
                         ^
                    Want to delete this

Process:
1. Start at head: CommitA
2. Check CommitA.next = CommitB (found it!)
3. Set CommitA.next = CommitB.next (skip over CommitB)
4. Delete CommitB

After:
Bucket 76: [CommitA] -> [CommitC] -> nullptr
                    ^
              Pointer updated, CommitB removed!

SPECIAL CASE: Deleting head of chain
Before:
Bucket 76: [CommitB] -> [CommitC] -> nullptr
           ^
      Want to delete head

Process:
1. Save CommitB.next (CommitC)
2. Update table[76] = CommitC
3. Delete CommitB

After:
Bucket 76: [CommitC] -> nullptr
           ^
      CommitC is new head!
*/
bool HashTable::remove(const string& commitID) {
    if (commitID.empty() || commitID == "NA") {
        return false;
    }
    
    // Calculate bucket index
    int index = hashFunction(commitID);
    
    // Handle empty bucket
    if (table[index] == nullptr) {
        return false;  // Not found
    }
    
    // Special case: deleting the head of the chain
    if (table[index]->commitID == commitID) {
        HashEntry* temp = table[index];
        table[index] = table[index]->next;  // Move head to next
        delete temp;                         // Delete old head
        numElements--;
        return true;
    }
    
    // General case: deleting from middle or end of chain
    HashEntry* current = table[index];
    while (current->next != nullptr) {
        if (current->next->commitID == commitID) {
            // Found it! Remove from chain
            HashEntry* temp = current->next;
            current->next = current->next->next;  // Skip over the node
            delete temp;                           // Delete the node
            numElements--;
            return true;
        }
        current = current->next;
    }
    
    return false;  // Not found
}

/*
============================================================================
CONTAINS
============================================================================
Purpose: Check if a commit exists in the table
Returns: true if commit is found, false otherwise
*/
bool HashTable::contains(const string& commitID) const {
    return search(commitID) != nullptr;
}

/*
============================================================================
UTILITY FUNCTIONS
============================================================================
*/

int HashTable::size() const {
    return numElements;
}

int HashTable::getCapacity() const {
    return capacity;
}

/*
LOAD FACTOR = total elements / number of buckets

This represents the average chain length.

Examples:
- 50 elements, 100 buckets → load factor = 0.5 (short chains)
- 100 elements, 100 buckets → load factor = 1.0 (optimal)
- 200 elements, 100 buckets → load factor = 2.0 (long chains - need rehash!)

Goal: Keep load factor ≈ 1.0 for O(1) performance
*/
double HashTable::getLoadFactor() const {
    return (double)numElements / capacity;
}

/*
Get the length of the longest chain in the table
Useful for detecting poor hash distribution

Ideally: max chain length should be small (< 5)
If max chain length is large (> 10), hash function might be poor
*/
int HashTable::getMaxChainLength() const {
    int maxLength = 0;
    for (int i = 0; i < capacity; i++) {
        int length = getChainLength(table[i]);
        maxLength = max(maxLength, length);
    }
    return maxLength;
}

/*
Get average chain length for NON-EMPTY buckets
This gives a better sense of actual search cost
*/
double HashTable::getAvgChainLength() const {
    if (numElements == 0) return 0.0;
    
    int nonEmptyBuckets = 0;
    for (int i = 0; i < capacity; i++) {
        if (table[i] != nullptr) {
            nonEmptyBuckets++;
        }
    }
    
    if (nonEmptyBuckets == 0) return 0.0;
    
    return (double)numElements / nonEmptyBuckets;
}

/*
============================================================================
PRINT STATISTICS
============================================================================
Purpose: Display hash table metrics for debugging and analysis

Shows:
- Current capacity (number of buckets) and number of elements
- Load factor (average elements per bucket)
- Chain statistics:
  - Number of non-empty buckets
  - Maximum chain length
  - Average chain length (for non-empty buckets)
- Chain length distribution
- Performance assessment

KEY METRICS TO WATCH:
1. Load factor: should be ≈ 1.0 for best performance
2. Max chain length: should be < 5 for good hash function
3. Avg chain length: should be close to load factor
*/
void HashTable::printStats() const {
    cout << "\n========== Hash Table Statistics ==========" << endl;
    cout << "Implementation: SEPARATE CHAINING (Manual Linked Lists)" << endl;
    cout << "Capacity (buckets): " << capacity << endl;
    cout << "Active Elements: " << numElements << endl;
    cout << "Load Factor: " << fixed << setprecision(2) 
         << (getLoadFactor() * 100) << "%" << endl;
    
    // Count non-empty buckets
    int nonEmptyBuckets = 0;
    int emptyBuckets = 0;
    for (int i = 0; i < capacity; i++) {
        if (table[i] == nullptr) {
            emptyBuckets++;
        } else {
            nonEmptyBuckets++;
        }
    }
    
    cout << "Non-Empty Buckets: " << nonEmptyBuckets << endl;
    cout << "Empty Buckets: " << emptyBuckets << endl;
    cout << "Max Chain Length: " << getMaxChainLength() << endl;
    
    if (nonEmptyBuckets > 0) {
        cout << "Avg Chain Length (non-empty): " << fixed << setprecision(2)
             << getAvgChainLength() << endl;
    }
    
    // Show distribution of chain lengths
    int maxLen = getMaxChainLength();
    if (maxLen > 0) {
        cout << "\nChain Length Distribution:" << endl;
        vector<int> distribution(maxLen + 1, 0);
        for (int i = 0; i < capacity; i++) {
            int length = getChainLength(table[i]);
            distribution[length]++;
        }
        
        for (int i = 0; i <= maxLen; i++) {
            if (distribution[i] > 0) {
                cout << "  Length " << i << ": " << distribution[i] << " buckets";
                if (i == 0) cout << " (empty)";
                cout << endl;
            }
        }
    }
    
    // Memory and performance analysis
    cout << "\nMemory & Performance:" << endl;
    cout << "✓ No wasted slots (no tombstones)" << endl;
    cout << "✓ No probe sequences to maintain" << endl;
    cout << "✓ Simple deletion (remove from list)" << endl;
    
    // Performance assessment
    double avgChain = getAvgChainLength();
    if (avgChain <= 1.5) {
        cout << "\n✓ Excellent performance (avg chain ≤ 1.5)" << endl;
    } else if (avgChain <= 3.0) {
        cout << "\nℹ️  Good performance (avg chain ≤ 3.0)" << endl;
    } else {
        cout << "\n⚠️  Performance degrading (avg chain > 3.0)" << endl;
        cout << "   Rehashing will occur automatically at load factor > " 
             << LOAD_FACTOR_THRESHOLD << endl;
    }
    
    cout << "==========================================\n" << endl;
}

/*
============================================================================
CLEAR
============================================================================
Purpose: Remove all entries from the hash table
Resets table to initial empty state

Process:
1. For each bucket, delete its entire chain
2. Set bucket pointer to nullptr
3. Reset element counter
*/
void HashTable::clear() {
    for (int i = 0; i < capacity; i++) {
        deleteChain(table[i]);
        table[i] = nullptr;
    }
    numElements = 0;
}

void HashTable::displayAll() {
    for (int i=0;i<capacity;i++) {
        if (table[i] == nullptr) {
            continue;
        }
        else {
            cout<<"TABLE["<<i<<"]:"<<endl;
            HashEntry* temp = table[i];
            while (temp!=nullptr) {
                cout<<"(COMMIT ID: "<<temp->node->getCommitID()<<" , COMMIT MSG: "<<temp->node->getCommitMsg()<<" , NEXT COMMIT: "<<temp->node->getNextID()<<")->";
                temp = temp->next;
            }
        }
    }
}
