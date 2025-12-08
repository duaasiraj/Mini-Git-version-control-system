# MiniGit - Version Control System (C++)

A mini Git-like version control system built from scratch in **C++** to demonstrate the practical use of **Data Structures and Algorithms (DSA)** in managing project files, commits, and restore operations. minigit provides core version control functionality with an intuitive undo/redo system.

##  Features

- **Repository Management** – Initialize, manage, and update repositories with `.minigit` structure
- **Staging Area** – Selectively stage files or entire directories before committing
- **Commit System** – Create snapshots with unique IDs, timestamps, and commit messages
- **Commit History Log** – View complete commit history with metadata (ID, message, timestamp)
- **Undo/Redo Navigation** – Move backward and forward through commit history using dual stacks
- **Revert Operation** – Create new commit containing data from any previous commit
- **Fast Commit Lookup** – O(1) hash table-based commit search by ID
- **Persistent Storage** – All commit data and navigation state saved to disk
- **Automatic State Recovery** – Restore undo/redo state across program sessions
- **Custom CLI Interface** – Easy-to-use text-based interface for all operations
- **Modular Codebase** – Clean separation of concerns with header/implementation files

---

##  DSA Concepts Used

This project emphasizes the **application of DSA concepts** rather than library-based solutions.

| Concept | Implementation | Usage in minigit |
|---------|----------------|------------------|
| **Linked Lists** | Singly-linked list structure | Stores commit history and maintains sequential references to previous commits |
| **Stacks** | Dual stack implementation | Undo/redo mechanism — navigating commits in reverse and forward order |
| **Hash Tables** | Separate chaining with dynamic resizing | Fast O(1) commit lookup by ID with collision resolution |
| **Hash Functions** | FNV-1a algorithm | Generates unique commit IDs combining hardware randomness and timestamps |
| **Recursion** | Recursive directory copying | `copyRecursive()` method traverses and copies nested directory structures |
| **File Handling & Buffers** | Filesystem operations | Reading/writing repository states and commit data efficiently |
| **Dynamic Memory Allocation** | Pointer-based structures | Managing linked list nodes and hash table chains |
| **Algorithms** | Search, traversal, and comparison | Searching commits, comparing file states, optimizing restore operations |
| **Iterators** | STL filesystem iterators | `recursive_directory_iterator` for traversing directory trees |

---

### Core Data Structures

```
┌─────────────────────────────────────────────────────────┐
│              Commit History (Linked List)               │
│    [TAIL/Oldest] → [Commit] → [Commit] → [HEAD/Latest] │
└─────────────────────────────────────────────────────────┘
                           ↕
              (Navigation using Stacks)
                           ↕
┌─────────────────────────────────────────────────────────┐
│              Undo/Redo System (Dual Stacks)             │
│   Undo: [c1, c2, c3]    Current: c4    Redo: [c5, c6]  │
└─────────────────────────────────────────────────────────┘
```

##  Architecture

### Core Data Structures

```
┌─────────────────────────────────────────────────────────┐
│              Commit History (Linked List)               │
│    [TAIL/Oldest] → [Commit] → [Commit] → [HEAD/Latest] │
└─────────────────────────────────────────────────────────┘
                           ↕
              (Navigation using Stacks)
                           ↕
┌─────────────────────────────────────────────────────────┐
│              Undo/Redo System (Dual Stacks)             │
│   Undo: [c1, c2, c3]    Current: c4    Redo: [c5, c6]  │
└─────────────────────────────────────────────────────────┘
```

### Components

- **CommitNode** – Individual commit node with metadata and forward pointer to next commit
- **CommitManager** – Manages singly-linked list of commits (HEAD/TAIL pointers, insertion, traversal)
- **Stack** – Generic LIFO data structure using vector, supports push/pop/peek operations
- **Restore** – Dual stack system for undo/redo with persistent state (restore_state.txt)
- **Repository** – File system operations (init, staging, file copying, directory traversal)
- **HashTable** – Separate chaining hash table with dynamic resizing (load factor 0.75)
- **HashingHelper** – FNV-1a hash + hardware randomness + nanosecond timestamps for unique IDs

### Key Implementation Details

**Singly Linked List (Commit History)**
- HEAD points to latest commit, TAIL points to oldest
- Each node stores: commitID, commitMsg, nextCommitID, nextNode pointer
- Each node persisted as directory on disk with info.txt and NextCommit.txt

**Dual Stack System (Undo/Redo)**
- Undo stack: stores previous commits (can go back)
- Redo stack: stores forward commits (can go forward)
- Creating new commit clears redo stack (prevents timeline conflicts)
- State saved to restore_state.txt for persistence

**Hash Table (Fast Lookup)**
- Separate chaining for collision resolution
- Dynamic resizing at load factor 0.75
- Polynomial rolling hash with prime = 31
- Average O(1) search/insert/delete

**Commit ID Generation**
- FNV-1a hash (64-bit): XOR-then-multiply pattern
- Hardware random_device for entropy
- High-resolution clock (nanosecond precision)
- Combined: ~1 in 18 quintillion collision probability
- Output: 16-character hexadecimal string

---

##  Project Structure

```
minigit/
├── include/
│   ├── CommitNode.h          # Individual commit representation
│   ├── CommitManager.h       # Linked list management
│   ├── Repository.h          # File system operations
│   ├── Restore.h             # Undo/redo system
│   ├── HashTable.h           # Fast commit lookup
│   └── HashingHelper.h       # Unique ID generation
│
├── src/
│   ├── main.cpp              # CLI interface
│   ├── CommitNode.cpp
│   ├── CommitManager.cpp
│   ├── Repository.cpp
│   ├── Restore.cpp
│   ├── HashTable.cpp
│   └── HashingHelper.cpp
│
├── build/
│   └── (compiled files + executable)
│
├── .minigit/                 # Repository data (created at runtime)
│   └── <repo-name>/
│       ├── HEAD.txt          # Current commit pointer
│       ├── restore_state.txt # Undo/redo stack state
│       ├── staging_area/     # Staged files
│       └── commits/          # Commit snapshots
│           └── <commit-id>/
│               ├── info.txt      # Commit metadata
│               ├── NextCommit.txt # Link to next commit
│               └── Data/         # Project files
│
├── Makefile                  # Build automation
└── README.md
```

---

##  Build & Run

### Prerequisites

- **C++17 or higher**
- **MinGW/GCC** compiler
- Standard C++ library with filesystem support

---

### Method 1: Manual Compilation

**Run the following command inside the program's directory:**

```bash
g++ -std=gnu++17 src/main.cpp src/CommitManager.cpp src/HashingHelper.cpp \
    src/LogViewer.cpp src/Repository.cpp src/Restore.cpp src/CommitNode.cpp \
    src/HashTable.cpp src/Stack.cpp -I include -o minigit.exe
```

**Then run the executable from any location:**

```bash
# Windows
C:\Users\User\Desktop>"LocationOfExe\minigit.exe"

# Linux/Mac
./minigit
```

---

### Method 2: Using Makefile 
**To build the project:**

```bash
mingw32-make        # Windows
make                # Linux/Mac
```

**To run:**

```bash
./build/vcs         # Linux/Mac
build\vcs.exe       # Windows
```

**To clean build files:**

```bash
mingw32-make clean  # Windows
make clean          # Linux/Mac
```

---

##  Quick Start

### Initialize a Repository

```bash
# Initialize minigit in current directory
minigit init 
```

Creates `.minigit/myproject/` directory structure:
```
.minigit/
└── myproject/
    ├── HEAD.txt          # Points to current commit (initially "NA")
    ├── TAIL.txt          # Points to oldest commit
    ├── restore_state.txt # Undo/redo stack state
    ├── staging_area/     # Files staged for commit
    └── commits/          # All commit snapshots
        └── <commit-id>/
            ├── info.txt       # Commit metadata (ID, message, timestamp)
            ├── NextCommit.txt # Link to next commit in chain
            └── Data/          # Complete project snapshot
```

### Basic Workflow

```bash
# Add specific files to staging area
minigit add main.cpp utils/helper.cpp

# Add all files in current directory
minigit add .

# View files in staging area
minigit status

# Create a commit with message
minigit commit "Initial commit"

# View commit history (newest to oldest)
minigit log

# Undo to previous commit
minigit undo

# Redo to next commit
minigit redo

# Revert to specific commit (creates new commit)
minigit revert <commit-id>

# Clear staging area
minigit clear
```

## Usage Examples

### Example 1: Creating Your First Commit

```bash
# Initialize repository
minigitinit 

# Create some files
echo "int main() { return 0; }" > main.cpp
echo "void helper() {}" > utils.cpp

# Stage files
minigit add main.cpp utils.cpp

# Create commit
minigit commit "Initial project setup"
```

**Result**:
```
Linked List: [c1] → NULL
             ↑
           HEAD/TAIL

Working Directory: main.cpp, utils.cpp
```

### Example 2: Undo/Redo Navigation

```bash
# Create three commits
minigit commit "First commit"    # c1
minigit commit "Second commit"   # c2
minigit commit "Third commit"    # c3

# Current state:
# Undo: [c1, c2]  Current: c3  Redo: []

# Undo to c2
minigit undo
# Undo: [c1]  Current: c2  Redo: [c3]

# Undo to c1
minigit undo
# Undo: []  Current: c1  Redo: [c3, c2]

# Redo to c2
minigit redo
# Undo: [c1]  Current: c2  Redo: [c3]

# Redo to c3
minigit redo
# Undo: [c1, c2]  Current: c3  Redo: []
```

### Example 3: Creating a New Branch (Clearing Redo)

```bash
# At c2 with c3 in redo stack
minigit undo  # From c3 to c2

# Make changes and commit
echo "new feature" > feature.cpp
minigit add feature.cpp
minigit commit "New feature"  # Creates c4

# Result: c3 is no longer accessible via redo
# Linked List: [c1] → [c2] → [c4]
#                      ↓
#                     [c3] (orphaned)
```

### Example 4: Reverting to Previous State

```bash
# Create commits with different files
minigit commit "Add file A"  # c1: fileA.txt
minigit commit "Add file B"  # c2: fileA.txt, fileB.txt
minigit commit "Add file C"  # c3: fileA.txt, fileB.txt, fileC.txt

# Revert to c1
minigit revert <c1-commit-id>

# Result: Creates c4 with only fileA.txt
# Linked List: [c1] → [c2] → [c3] → [c4]
#              ↑                      ↑
#        (copied from)              HEAD
```

##  Command Reference

| Command | Description | Example |
|---------|-------------|---------|
| `init <name>` | Initialize new repository | `minigit init myproject` |
| `add <files>` | Stage files for commit | `minigit add main.cpp utils.cpp` |
| `add .` | Stage all files | `minigit add .` |
| `commit <msg>` | Create new commit | `minigit commit "Fix bug"` |
| `log` | Display commit history | `minigit log` |
| `undo` | Move to previous commit | `minigit undo` |
| `redo` | Move to next commit | `minigit redo` |
| `revert <id>` | Restore specific commit | `minigit revert a1b2c3d4` |
| `status` | Show staging area status | `minigit status` |
| `clear` | Clear staging area | `minigit clear` |

##  Algorithm Complexity

| Operation | Time Complexity | Space Complexity | Data Structure Used | Notes |
|-----------|----------------|------------------|---------------------|-------|
| **Commit Operations** |
| Add Commit | O(1) + O(f) | O(f) | Linked List | f = number of files |
| Load Commits | O(n) | O(n) | Linked List | n = number of commits |
| Print Log | O(n) | O(1) | Linked List traversal | n = number of commits |
| Revert | O(f) | O(f) | Creates new commit | f = number of files |
| **Stack Operations** |
| Undo | O(1) + O(f) | O(1) | Stack pop | f = file copy overhead |
| Redo | O(1) + O(f) | O(1) | Stack pop | f = file copy overhead |
| Record Commit | O(m) | O(1) | Stack operations | m = redo stack size |
| **Hash Table Operations** |
| Insert Commit | O(1) avg | O(1) | Hash Table | With collision handling |
| Search Commit | O(1) avg | O(1) | Hash Table | Fast lookup by ID |
| Resize Table | O(n) | O(n) | Dynamic resizing | n = number of elements |
| **Repository Operations** |
| Init | O(1) | O(1) | Directory creation | - |
| Add File | O(f) | O(f) | File copy | f = number of files |
| Add All | O(f) | O(f) | Recursive traversal | f = total files |
| Checkout | O(f) | O(f) | File operations | f = number of files |
| Clear Staging | O(f) | O(1) | Delete files | f = staged files |

---

##  Learning Outcomes

By building and studying this project, we gained:

### **1. Understanding Version Control Internals**
- How version control systems work behind the scenes
- The architecture of commit-based systems
- File versioning and snapshot management
- State persistence and recovery mechanisms

### **2. Applying DSA in Real-World Scenarios**
- **Linked Lists** for maintaining sequential commit history
- **Stacks** for implementing undo/redo functionality
- **Hash Tables** for efficient commit lookup and retrieval
- **Hash Functions** for generating unique identifiers
- **Recursion** for directory traversal and tree operations

### **3. Software Engineering Practices**
- Modular programming with clear separation of concerns
- Build automation using Makefiles
- Header/implementation file organization
- Memory management and avoiding leaks
- Error handling and edge case management
- Appropriate code documentation

### **4. System-Level Programming**
- File I/O operations and buffering
- Filesystem navigation and manipulation
- Directory structures and path handling
- Persistent storage strategies
- Cross-platform compatibility considerations

### **5. Algorithm Design**
- Efficient search and retrieval algorithms
- State comparison and diff operations
- Dynamic memory allocation patterns
- Performance optimization techniques
- Time-space tradeoff analysis

---

## 📊 Project Structure

```
├── include/
│ ├── CommitManager.h
│ ├── Repository.h
│ ├── Restore.h
│
├── src/
│ ├── main.cpp
│ ├── CommitManager.cpp
│ ├── Repository.cpp
│ ├── Restore.cpp
│
├── build/
│ ├── (compiled object files + executable)
│
├── Makefile
└── README.md
```
---
##  Build & Run
### TO COMPILE MANUALLY
**run the following command inside the program's directory**
```bash
g++ -std=gnu++17 src/main.cpp src/CommitManager.cpp src/HashingHelper.cpp  src/Repository.cpp src/Restore.cpp src/CommitNode.cpp src/HashTable.cpp -I include -o minigit.exe
```
**then run the exe wherever you want by referring to the exe's location**
```
C:\Users\User\Desktop>"LocationOfexe\minigit.exe"
```
### **To build the project**
```bash
mingw32-make
```
### **To run**
```bash
./build/vcs
````
### **To clean build files**
```bash
mingw32-make clean
````
### **To run from cmd**
- Create .exe , store in folder and copy path
- Set this path as one of the environment variables
- Now it can be run directly from cmd using ```minigit init``` and other related commands
---
## **Learning Outcomes**
Understanding how version control systems work internally.
Applying DSA concepts in real-world software scenarios.
Learning modular programming and build automation using Makefiles.
Gaining insight into file I/O, system design, and memory management in C++.

##  How It Works

### Commit Creation Flow

```
1. User stages files → Repository::add() → staging_area/
2. User creates commit → CommitManager::addCommit()
3. Generate unique ID → HashingHelper::generateCommitID()
   - Hardware randomness (random_device)
   - Nanosecond timestamp (high_resolution_clock)
   - FNV-1a hash algorithm
   - Format as 16-char hex string
4. Create CommitNode → CommitNode::CommitNode(id, msg, repo)
5. Create commit directory → commits/<id>/
6. Save metadata → info.txt (ID, message, timestamp)
7. Copy staged files → commits/<id>/Data/
8. Link to previous commit → update previous NextCommit.txt
9. Update HEAD pointer → HEAD.txt
10. Record in undo/redo → Restore::recordCommit()
    - Push current to undo stack
    - Clear entire redo stack
11. Clear staging area → Repository::clearStagingArea()
12. Save state to disk → Restore::saveStateToDisk()
```

### Undo Operation Flow

```
1. Check if undo possible → Stack::isEmpty()
2. Save current commit → redoStack.push(currentCommitID)
3. Get previous commit → currentCommitID = undoStack.pop()
4. Checkout previous commit → Repository::checkout()
   - Remove all files from working directory (except .minigit)
   - Copy files from commits/<id>/Data/ to working directory
5. Update HEAD → Repository::setHead(commitID)
6. Save state → Restore::saveStateToDisk()
   - Write CURRENT:<id>
   - Write undo stack contents
   - Write redo stack contents
```

### Redo Operation Flow

```
1. Check if redo possible → Stack::isEmpty()
2. Save current commit → undoStack.push(currentCommitID)
3. Get next commit → currentCommitID = redoStack.pop()
4. Checkout next commit → Repository::checkout()
5. Update HEAD → Repository::setHead(commitID)
6. Save state → Restore::saveStateToDisk()
```

### Revert Operation Flow

```
1. Verify commit exists → filesystem::exists(commits/<id>/)
2. Copy source commit to staging:
   - Clear staging_area/
   - Copy all files from commits/<id>/Data/ to staging_area/
3. Create new commit → CommitManager::addCommit("Revert to " + id)
4. Update working directory:
   - Remove current files
   - Copy new commit's Data/ to working directory
5. Result: New commit at HEAD with old commit's data
   - Original commit unchanged (immutable history)
```

### System Initialization/Loading Flow

```
1. Program starts → Create Repository object
2. Load commit history → CommitManager::loadListFromDisk()
   - Read HEAD.txt and TAIL.txt
   - Load TAIL node → CommitNode(id, repo)
   - Traverse forward using NextCommit.txt
   - Build linked list: TAIL → ... → HEAD
3. Load undo/redo state → Restore::loadStateFromDisk()
   - Read restore_state.txt
   - Parse CURRENT:<id>
   - Rebuild undo stack from UNDO:<id> lines
   - Rebuild redo stack from REDO:<id> lines
4. System ready with full state restored
```

### Data Persistence Strategy

**Memory Layer** ↔ **Disk Layer**

```
Linked List Structure:
  CommitNode          ↔  commits/<id>/info.txt
  nextCommitID field  ↔  commits/<id>/NextCommit.txt
  nextNode pointer    ↔  (reconstructed on load)
  HEAD/TAIL pointers  ↔  HEAD.txt, TAIL.txt

Stack State:
  undoStack          ↔  restore_state.txt (UNDO: lines)
  redoStack          ↔  restore_state.txt (REDO: lines)
  currentCommitID    ↔  restore_state.txt (CURRENT: line)

Commit Data:
  Staged files       ↔  staging_area/<files>
  Committed files    ↔  commits/<id>/Data/<files>
```

### Hash Table Operations

**Insert with Collision Handling:**
```
1. Check load factor: numElements / tableSize
2. If load factor ≥ 0.75:
   - Create new table (size * 2)
   - Rehash all elements
   - Delete old table
3. Calculate hash: polynomial rolling hash % tableSize
4. Insert at head of chain (O(1))
5. Increment numElements
```

**Search Operation:**
```
1. Calculate hash: hashFunction(commitID)
2. Get chain at table[hash]
3. Traverse chain:
   - Compare commitID with each node
   - Return CommitNode* if found
   - Return nullptr if not found
4. Average O(1), worst O(n) if all in one bucket
```

## ⚠️ Limitations

### Current Implementation Constraints

- **No Branching Support**: Single linear commit history only (like early VCS systems)
- **Full Snapshots**: Each commit stores complete project state, not deltas/diffs
  - High disk usage for large projects
  - Slower commit creation for many files
- **No Merge Capability**: Cannot combine different commit histories
- **No Remote Support**: Local repository only, no push/pull/clone operations
- **Limited File Handling**: No file compression or delta encoding
- **Single Repository**: One repository per directory structure
- **No Conflict Resolution**: No detection or resolution of conflicting changes
- **Not Suitable for Large Projects**: Full snapshot model inefficient for 1000+ files

### Technical Limitations

- **Memory Usage**: Entire commit list loaded into memory on startup
- **File Size Limits**: Values in hash table limited to 5MB per commit data
- **No Symbolic Links**: Symbolic links not supported
- **Platform-Specific**: File paths may need adjustment for cross-platform use
- **No Permissions**: File permissions not preserved across commits


##  Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request
- **Issues**: Please open an issue on GitHub for bugs or feature requests



##  Acknowledgments

- Inspired by **Git** and other version control systems
- **FNV-1a hash algorithm** by Glenn Fowler, Landon Curt Noll, and Kiem-Phong Vo
- Data structures concepts from classic computer science textbooks
- Community feedback and contributions


---

<div align="center">
Made to learn <3


⭐ **Star this repository if you find it helpful!** ⭐ :)

</div>
