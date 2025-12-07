# MiniVCS - Data Structures Documentation

## Overview

MiniVCS is a lightweight version control system implemented in C++ that demonstrates the use of fundamental data structures including **Linked Lists** and **Stacks**. The system manages file versioning through commits organized in a singly-linked list, with undo/redo functionality powered by stack-based navigation.

---

## Core Data Structures

### 1. Singly Linked List (Commit History)

**Purpose**: Maintains a chronological sequence of commits where each commit references its successor (next commit).

**Structure**:
```
[TAIL/Oldest] → [Commit] → [Commit] → [HEAD/Latest] → NULL
```

**Implementation**: `CommitNode` class representing individual nodes, managed by `CommitManager`

**Key Properties**:
- **Head**: Points to the most recent commit (latest)
- **Tail**: Points to the oldest commit (first)
- **Unidirectional traversal**: Can only navigate forward (next)
- **Persistent storage**: Each node corresponds to a directory on disk

---

### 2. Stack (Undo/Redo System)

**Purpose**: Implements undo/redo functionality using two stacks to track commit navigation history.

**Structure**:
```
Undo Stack:        Redo Stack:
[Commit 3]         [Commit 5]
[Commit 2]         [Commit 4]
[Commit 1]         (empty bottom)
(bottom)
```

**Implementation**: Custom `Stack` class using `vector<string>` as underlying storage

**Operations**:
- **Push**: Add commit ID to top of stack
- **Pop**: Remove and return top commit ID
- **Peek**: View top commit ID without removing
- **isEmpty**: Check if stack has elements

---

## Class Documentation

## CommitNode

### Overview
Represents a single commit in the version control system. Each node contains commit metadata and maintains a forward link to the next commit in the history.

### Data Members

| Member | Type | Description |
|--------|------|-------------|
| `commitID` | `string` | Unique identifier generated via FNV-1a hash |
| `commitMsg` | `string` | User-provided commit message |
| `nextCommitID` | `string` | ID of the next (newer) commit |
| `nextNode` | `CommitNode*` | Pointer to next commit node |
| `commitsBasePath` | `filesystem::path` | Path to repository's commits directory |

### Constructors

#### `CommitNode()`
**Purpose**: Default constructor initializing empty node.

**Algorithm**:
```
nextNode = NULL
Initialize all strings to empty
```

**Dependencies**: None

**Example**:
```cpp
CommitNode* node = new CommitNode();
// Creates empty node with no data
```

---

#### `CommitNode(const string& cI, const string& cM, Repository& repo)`
**Purpose**: Creates new commit with ID and message, saves data to disk.

**Parameters**:
- `cI`: Commit ID (16-character hex string)
- `cM`: Commit message (user-provided description)
- `repo`: Repository object to get paths and staging area

**Algorithm**:
```
1. commitID = cI
2. commitMsg = cM
3. nextNode = NULL
4. Get commitsBasePath from repo
5. Call createCommitData(repo.getStagingPath())
```

**Dependencies**:
- `Repository::getCommitsPath()` - Gets base path for commits
- `Repository::getStagingPath()` - Gets staging area path
- `createCommitData(filesystem::path)` - Creates commit directory structure

**Example**:
```cpp
Repository repo("myproject");
CommitNode* node = new CommitNode("a1b2c3d4e5f6g7h8", "Initial commit", repo);
// Creates: .Minivcs/myproject/commits/a1b2c3d4e5f6g7h8/
```

---

#### `CommitNode(const string& cI, Repository& repo)`
**Purpose**: Loads existing commit from disk using ID.

**Parameters**:
- `cI`: Commit ID to load
- `repo`: Repository object to get commits path

**Algorithm**:
```
1. commitID = cI
2. nextNode = NULL
3. Get commitsBasePath from repo
4. Call loadNodeInfo() to read from disk
```

**Dependencies**:
- `Repository::getCommitsPath()` - Gets base path for commits
- `loadNodeInfo()` - Reads commit data from info.txt

**Example**:
```cpp
Repository repo("myproject");
CommitNode* node = new CommitNode("a1b2c3d4e5f6g7h8", repo);
// Loads commit from: .Minivcs/myproject/commits/a1b2c3d4e5f6g7h8/
```

---

### Key Methods

#### `void createCommitData(const filesystem::path& stagingPath)`
**Purpose**: Creates commit folder structure and copies files from staging area.

**Parameters**:
- `stagingPath`: Path to staging area (e.g., `.Minivcs/myproject/staging_area`)

**Algorithm**:
```
STEP 1: Create directory structure
    Create: commitsBasePath/commitID/
    Create: commitsBasePath/commitID/Data/

STEP 2: Create info.txt
    Open: commitsBasePath/commitID/info.txt
    Write:
        "1. COMMIT ID: <commitID>"
        "2. COMMIT MESSAGE: <commitMsg>"
        "3. DATE & TIME OF COMMIT: <timestamp>"

STEP 3: Copy files from staging to Data/
    FOR EACH entry in recursive_directory_iterator(stagingPath):
        Calculate relative path from staging area
        
        Example:
            entry.path() = .Minivcs/myproject/staging_area/utils/helper.cpp
            stagingPath  = .Minivcs/myproject/staging_area
            relative     = utils/helper.cpp
        
        Set destination = dataPath / relative
            destination  = .Minivcs/myproject/commits/c1/Data/utils/helper.cpp
        
        IF entry is directory:
            Create directory at destination
        ELSE:
            Create parent directories if needed
            Copy file to destination (overwrite if exists)

STEP 4: Create NextCommit.txt
    Open: commitsBasePath/commitID/NextCommit.txt
    Write: "NA"
    Set nextCommitID = "NA"
```

**Dependencies**: None (leaf function)

**Directory Structure Created**:
```
.Minivcs/myproject/commits/a1b2c3d4e5f6g7h8/
    ├── info.txt              # Commit metadata
    ├── NextCommit.txt        # Link to next commit (initially "NA")
    └── Data/                 # Copied project files
        ├── src/
        │   └── main.cpp
        └── utils/
            └── helper.cpp
```

**Example Execution**:
```cpp
// Before: Files in staging_area/
//   - main.cpp
//   - utils/helper.cpp

node->createCommitData(repo.getStagingPath());

// After: Files copied to commits/c1/Data/
//   - main.cpp
//   - utils/helper.cpp (with directory structure preserved)
```

**Data Structure**: Creates persistent node on disk corresponding to linked list node

---

#### `void revertCommitData(const string& id)`
**Purpose**: Creates new commit containing data from a previous commit.

**Parameters**:
- `id`: Commit ID to revert to (copy data from)

**Algorithm**:
```
STEP 1: Create data folder for current commit
    Create: commitsBasePath/commitID/Data/

STEP 2: Create info.txt for the new commit
    Open: commitsBasePath/commitID/info.txt
    Write:
        "1. COMMIT ID: <commitID>"
        "2. COMMIT MESSAGE: <commitMsg>"
        "3. DATE & TIME OF COMMIT: <timestamp>"

STEP 3: Find old commit path
    oldPath = commitsBasePath / id / "Data"
    
    Example:
        oldPath = .Minivcs/myproject/commits/c2/Data

STEP 4: Copy all files from old commit
    FOR EACH entry in recursive_directory_iterator(oldPath):
        Calculate relative path
        
        Example:
            entry.path() = .Minivcs/myproject/commits/c2/Data/src/utils.cpp
            oldPath      = .Minivcs/myproject/commits/c2/Data
            relative     = src/utils.cpp
        
        Set destination = newDataPath / relative
            destination  = .Minivcs/myproject/commits/c4/Data/src/utils.cpp
        
        IF entry is directory:
            Create directory at destination
        ELSE:
            Copy file to destination (overwrite if exists)
```

**Dependencies**: None (leaf function)

**Example**:
```cpp
// Reverting to commit c2 by creating new commit c4
node->revertCommitData("c2");

// Result: c4/Data/ now contains exact copy of c2/Data/
```

**Note**: This creates a **new commit** with old data, not a modification of existing commits

---

#### `void loadNodeInfo()`
**Purpose**: Loads commit information from disk into memory.

**Algorithm**:
```
STEP 1: Build info.txt path
    infoPath = commitsBasePath / commitID / "info.txt"

STEP 2: Check if file exists
    IF NOT exists(infoPath):
        throw runtime_error("Could not find specified path!")

STEP 3: Open and read info.txt
    FOR EACH line in file:
        IF line starts with "1. COMMIT ID: ":
            commitID = substring after prefix
        ELSE IF line starts with "2. COMMIT MESSAGE: ":
            commitMsg = substring after prefix

STEP 4: Load next commit ID
    nextCommitID = loadNextCommitID()
```

**Dependencies**: 
- `loadNextCommitID()` - Reads NextCommit.txt file

**Example File Content** (info.txt):
```
1. COMMIT ID: a1b2c3d4e5f6g7h8
2. COMMIT MESSAGE: Initial commit
3. DATE & TIME OF COMMIT: Mon Dec 09 14:30:22 2024
```

**Data Structure**: Reconstructs linked list node from persistent storage

---

#### `string loadNextCommitID()`
**Purpose**: Reads the next commit ID from NextCommit.txt file.

**Algorithm**:
```
STEP 1: Build NextCommit.txt path
    nextPath = commitsBasePath / commitID / "NextCommit.txt"

STEP 2: Check if file exists
    IF exists(nextPath):
        Open file
        Read first line
        Return the line
    ELSE:
        Return "NA"
```

**Dependencies**: None (leaf function)

**Returns**:
- Next commit ID (16-character hex string) if exists
- "NA" if this is the most recent commit or file doesn't exist

**Example**:
```cpp
string nextID = node->loadNextCommitID();
// Returns: "b2c3d4e5f6g7h8i9" or "NA"
```

---

#### `void saveNextID(const string& id)`
**Purpose**: Persists next commit link information to disk.

**Parameters**:
- `id`: Commit ID of the next commit

**Algorithm**:
```
STEP 1: Build NextCommit.txt path
    path = commitsBasePath / commitID / "NextCommit.txt"

STEP 2: Open file for writing
    IF cannot open:
        throw runtime_error("Could not save id to NextCommit.txt")

STEP 3: Write ID to file
    Write id to file
    Close file
```

**Dependencies**: None (leaf function)

**Example**:
```cpp
node->saveNextID("b2c3d4e5f6g7h8i9");
// Writes "b2c3d4e5f6g7h8i9" to NextCommit.txt
```

**Data Structure**: Updates forward pointer in persistent storage

---

#### Getters and Setters

**Simple Accessors** (no dependencies):
- `string getCommitID()` - Returns commit ID
- `string getCommitMsg()` - Returns commit message
- `string getNextID()` - Returns next commit ID
- `CommitNode* getNextNode()` - Returns next node pointer
- `void setCommitID(const string& i)` - Sets commit ID
- `void setCommitMsg(const string& m)` - Sets commit message
- `void setNextID(const string& n)` - Sets next commit ID
- `void setNextNode(CommitNode* n)` - Sets next node pointer

---

## CommitManager

### Overview
Manages the singly-linked list of commits, handling creation, traversal, and persistence of commit history.

### Data Members

| Member | Type | Description |
|--------|------|-------------|
| `head` | `CommitNode*` | Points to latest (newest) commit |
| `tail` | `CommitNode*` | Points to oldest (first) commit |
| `repo` | `Repository*` | Pointer to repository for path access |

### Constructor

#### `CommitManager(Repository* repository)`
**Purpose**: Initializes manager and loads existing commit history from disk.

**Parameters**:
- `repository`: Pointer to Repository object

**Algorithm**:
```
1. head = nullptr
2. tail = nullptr
3. repo = repository

4. IF repo is NOT initialized:
    Return (no commits to load)

5. Call loadListFromDisk()
```

**Dependencies**:
- `Repository::isRepoInitialized()` - Checks if repository exists
- `loadListFromDisk()` - Reconstructs linked list from persistent storage

**Data Structure**: Rebuilds entire linked list from disk on initialization

---

### Key Methods

#### `void loadListFromDisk()`
**Purpose**: Reconstructs the singly-linked list of commits from persistent storage.

**Algorithm**:
```
STEP 1: Get HEAD and TAIL IDs
    commitsPath = repo->getCommitsPath()
    headID = readFile(commitsPath / "HEAD.txt")
    tailID = readFile(commitsPath / "TAIL.txt")

STEP 2: Check if commits exist
    IF headID == "NA" OR tailID == "NA":
        Return (no commits in repository)

STEP 3: Load tail node
    tail = loadSingleNode(tailID)

STEP 4: Traverse from tail to head, building links
    current = tail
    
    WHILE current != NULL AND current->getNextID() != "NA":
        Load next node using current->getNextID()
        Set current->nextNode = next
        Move current = next
    
    Update head = current (now pointing to latest commit)
```

**Dependencies**:
- `Repository::getCommitsPath()` - Gets commits directory path
- `loadSingleNode(string id)` - Loads individual commit nodes
- `CommitNode::getNextID()` - Retrieves next commit ID
- `CommitNode::setNextNode()` - Establishes forward pointer

**Data Structure**: Traversal pattern uses **tail-to-head iteration**

**Example Execution**:
```
Disk State:
    TAIL.txt: "c1"
    HEAD.txt: "c3"
    c1/NextCommit.txt: "c2"
    c2/NextCommit.txt: "c3"
    c3/NextCommit.txt: "NA"

Result:
    tail → [c1] → [c2] → [c3] → NULL
                           ↑
                          head
```

**Time Complexity**: O(n) where n = number of commits

---

#### `CommitNode* loadSingleNode(const string& id)`
**Purpose**: Factory method for creating CommitNode from existing disk data.

**Parameters**:
- `id`: Commit ID to load

**Algorithm**:
```
TRY:
    Create new CommitNode(id, *repo)
    Return pointer to node
CATCH any exception:
    Return nullptr
```

**Dependencies**:
- `CommitNode(string cI, Repository& repo)` constructor - Loads node via constructor

**Returns**: 
- Pointer to loaded node if successful
- `nullptr` if node doesn't exist or loading fails

**Example**:
```cpp
CommitNode* node = manager.loadSingleNode("a1b2c3d4e5f6g7h8");
if (node != nullptr) {
    // Successfully loaded
}
```

---

#### `void addCommit(const string& msg)`
**Purpose**: Creates new commit and adds it as new HEAD of linked list.

**Algorithm**:
```
STEP 1: Generate unique commit ID
    id = generateCommitID() from HashingHelper

STEP 2: Create new commit node
    newNode = new CommitNode(id, msg, *repo)

STEP 3: Handle first commit case
    IF head == nullptr:
        // First commit in repository
        head = tail = newNode
        
        Write id to commitsPath/HEAD.txt
        Write id to commitsPath/TAIL.txt
        Return

STEP 4: Link new commit to existing head
    head->setNextNode(newNode)      // Set pointer
    head->setNextID(id)              // Set ID field
    head->saveNextID(id)             // Persist to disk

STEP 5: Update head pointer
    head = newNode
    
    Write head->getCommitID() to commitsPath/HEAD.txt
```

**Dependencies**:
- `generateCommitID()` from HashingHelper - Creates unique commit ID
- `CommitNode(string, string, Repository&)` constructor - Creates new commit node
- `Repository::getCommitsPath()` - Gets commits directory path
- `CommitNode::setNextNode()` - Establishes forward link
- `CommitNode::setNextID()` - Sets next ID field
- `CommitNode::saveNextID()` - Persists link to disk
- `CommitNode::getCommitID()` - Gets new commit's ID

**Visual Example**:
```
Before addCommit("New feature"):
    tail → [c1] → [c2] → NULL
                   ↑
                  head

After addCommit("New feature"):
    tail → [c1] → [c2] → [c3] → NULL
                           ↑
                          head
    
    Files updated:
    - HEAD.txt: "c3"
    - c2/NextCommit.txt: "c3"
    - Created: commits/c3/ directory
```

**Data Structure**: **List insertion at head** - O(1) operation

---

#### `void revert(const string& commitID)`
**Purpose**: Creates new commit containing data from a previous commit and updates working directory.

**Algorithm**:
```
STEP 1: Validate commit exists
    commitPath = repo->getCommitsPath() / commitID
    
    IF NOT exists(commitPath):
        Print "Commit not found"
        Return

STEP 2: Copy source commit to staging
    srcCommit = commitPath / "Data"
    stagingPath = repo->getStagingPath()
    
    Clear all files in stagingPath
    
    FOR EACH entry in recursive_directory_iterator(srcCommit):
        Calculate relative path
        
        IF entry is directory:
            Create directory in staging
        ELSE:
            Create parent directories
            Copy file to staging

STEP 3: Create new commit from staging
    Call addCommit("Revert to " + commitID)
    
    Read new commit ID from HEAD.txt

STEP 4: Update working directory
    newDataPath = commitsPath / newID / "Data"
    workingDir = current_path()
    
    Remove all files in workingDir (except .Minivcs)
    
    Copy all files from newDataPath to workingDir
    
    Print success message with new commit ID
```

**Dependencies**:
- `Repository::getCommitsPath()` - Gets commits directory
- `Repository::getStagingPath()` - Gets staging area path
- `addCommit(string msg)` - Creates the revert commit
- `CommitNode::createCommitData()` - (called internally by addCommit)

**Visual Example**:
```
Before revert("c1"):
    [c1] → [c2] → [c3] → NULL
                   ↑
                  head

After revert("c1"):
    [c1] → [c2] → [c3] → [c4] → NULL
     ↑             ↑      ↑
  (copy from)  (old head) (new head, contains c1's data)
```

**Data Structure**: Creates new node at HEAD; does not modify existing list structure

**Note**: Revert creates a **new commit** rather than moving pointers, preserving history

---

#### `void printLog()`
**Purpose**: Displays commit history by traversing linked list from HEAD to TAIL.

**Algorithm**:
```
IF head == NULL:
    Print "No commits found"
    Return

current = head

WHILE current != NULL:
    STEP 1: Build path to info.txt
        infoPath = commitsPath / current->getCommitID() / "info.txt"
    
    STEP 2: Open file and find timestamp
        Read file line by line
        IF line contains "3. DATE & TIME OF COMMIT: ":
            Extract timestamp
    
    STEP 3: Print commit information
        Print "Commit: <commitID>"
        Print "Message: <commitMsg>"
        Print "Date: <timestamp>"
        Print separator
    
    STEP 4: Move to next commit
        current = current->getNextNode()
```

**Dependencies**:
- `Repository::getCommitsPath()` - Gets commits directory
- `CommitNode::getCommitID()` - Retrieves commit ID
- `CommitNode::getCommitMsg()` - Retrieves commit message
- `CommitNode::getNextNode()` - Traverses forward through list

**Output Example**:
```
Commit: c3a2b1d4e5f6g7h8
Message: Add new feature
Date: Mon Dec 09 14:30:22 2024
------------------------------------
Commit: b2c3d4e5f6g7h8i9
Message: Fix bug
Date: Mon Dec 09 13:15:10 2024
------------------------------------
Commit: a1b2c3d4e5f6g7h8
Message: Initial commit
Date: Mon Dec 09 12:00:00 2024
------------------------------------
```

**Data Structure**: **Forward traversal** from HEAD to TAIL using `nextNode` pointers

**Time Complexity**: O(n) where n = number of commits

---

#### `CommitNode* getHead()` / `CommitNode* getTail()`
**Purpose**: Accessor methods for list boundaries.

**Dependencies**: None (leaf functions)

---

#### `~CommitManager()`
**Purpose**: Destructor that frees all nodes in the linked list.

**Algorithm**:
```
current = head

WHILE current != nullptr:
    next = current->getNextNode()
    delete current
    current = next

head = nullptr
tail = nullptr
```

**Dependencies**:
- `CommitNode::getNextNode()` - Gets next node for traversal

**Data Structure**: **Forward traversal** for memory deallocation

---

## Repository

### Overview
Manages the file system operations for the version control system, including initialization, staging area management, and repository structure.

### Data Members

| Member | Type | Description |
|--------|------|-------------|
| `repoPath` | `filesystem::path` | Path to repository (.Minivcs/repoName) |
| `stagingPath` | `filesystem::path` | Path to staging area |
| `commitsPath` | `filesystem::path` | Path to commits directory |
| `repoName` | `string` | Name of the repository |

### Constructor

#### `Repository(const string& name)`
**Purpose**: Initializes repository object with paths but does NOT create directories.

**Parameters**:
- `name`: Name of repository (default: "repo")

**Algorithm**:
```
1. repoName = name

2. Set repoPath = current_path() / ".Minivcs" / repoName
   Example: C:/projects/myapp/.Minivcs/myproject

3. Set stagingPath = repoPath / "staging_area"
   Example: C:/projects/myapp/.Minivcs/myproject/staging_area

4. Set commitsPath = repoPath / "commits"
   Example: C:/projects/myapp/.Minivcs/myproject/commits
```

**Dependencies**: None (leaf function)

**Important**: Constructor only prepares paths; it does NOT create directories. Use `initRepo()` to create structure.

**Example**:
```cpp
Repository repo("myproject");
// Only sets paths, no directories created yet
```

---

### Key Methods

#### `void initRepo()`
**Purpose**: Creates repository directory structure on disk.

**Algorithm**:
```
STEP 1: Check if already initialized
    IF isRepoInitialized():
        Print "Repository already initialized"
        Return

STEP 2: Create directory structure
    Create directories:
        .Minivcs/repoName/
        .Minivcs/repoName/staging_area/
        .Minivcs/repoName/commits/

STEP 3: Create HEAD.txt
    Open: repoPath / "HEAD.txt"
    Write: "NA"
    Close file
    
    (HEAD.txt stores the current commit ID, "NA" means no commits yet)

STEP 4: Print success message
    Print "Initialized empty Minivcs repository in <repoPath>"
```

**Dependencies**:
- `isRepoInitialized()` - Checks if repository exists

**Directory Structure Created**:
```
.Minivcs/
    └── myproject/
        ├── HEAD.txt          # Contains "NA" initially
        ├── staging_area/     # Empty directory for staged files
        └── commits/          # Empty directory for commit storage
```

**Example**:
```cpp
Repository repo("myproject");
repo.initRepo();
// Creates: .Minivcs/myproject/ with subdirectories
```

---

#### `bool isRepoInitialized() const`
**Purpose**: Verifies repository is properly set up.

**Algorithm**:
```
Check all required components exist:
    1. repoPath exists AND is a directory
    2. stagingPath exists AND is a directory
    3. commitsPath exists AND is a directory

IF all exist:
    Return true
ELSE:
    Return false
```

**Dependencies**: None (leaf function)

**Example**:
```cpp
if (repo.isRepoInitialized()) {
    // Safe to perform repository operations
}
```

---

#### `void add(const vector<string>& files)`
**Purpose**: Adds multiple files to staging area (equivalent to `git add`).

**Parameters**:
- `files`: Vector of filenames to add

**Algorithm**:
```
STEP 1: Validate repository
    IF NOT isRepoInitialized():
        Print error message
        Return

STEP 2: Check if files provided
    IF files vector is empty:
        Print "Nothing specified, nothing added"
        Return

STEP 3: Initialize counters
    successCount = 0
    failCount = 0

STEP 4: Process each file
    FOR EACH filename in files:
        TRY:
            Call addSingleFile(filename)
            Print success message for this file
            Increment successCount
        CATCH exception:
            Print error message for this file
            Increment failCount

STEP 5: Print summary
    IF successCount > 0:
        Print "Successfully added <successCount> file(s)"
```

**Dependencies**:
- `isRepoInitialized()` - Validates repository
- `addSingleFile(string)` - Adds individual files

**Example**:
```cpp
vector<string> files = {"main.cpp", "utils/helper.cpp", "README.md"};
repo.add(files);

// Output:
// add 'main.cpp'
// add 'utils/helper.cpp'
// add 'README.md'
// Successfully added 3 file(s)
```

---

#### `void addSingleFile(const string& filename)`
**Purpose**: Adds a single file or directory to staging area (private helper).

**Parameters**:
- `filename`: Path to file relative to current directory

**Algorithm**:
```
STEP 1: Build source path
    sourcePath = current_path() / filename
    
    Example:
        current_path() = C:/projects/myapp
        filename = "src/main.cpp"
        sourcePath = C:/projects/myapp/src/main.cpp

STEP 2: Validate file exists
    IF NOT exists(sourcePath):
        throw runtime_error("pathspec did not match any files")

STEP 3: Check for VCS directory
    IF filename is ".Minivcs" or ".git":
        throw runtime_error("cannot add VCS directory")

STEP 4: Calculate destination path
    destPath = stagingPath / filename
    
    Example:
        stagingPath = .Minivcs/myproject/staging_area
        filename = "src/main.cpp"
        destPath = .Minivcs/myproject/staging_area/src/main.cpp

STEP 5: Create parent directories
    IF destPath has parent directories:
        Create all parent directories
        
    Example:
        Creates: staging_area/src/ directory

STEP 6: Copy file or directory
    IF sourcePath is a directory:
        Call copyRecursive(sourcePath, destPath)
    ELSE:
        Copy file with overwrite option
```

**Dependencies**:
- `copyRecursive()` - Recursively copies directories

**Example Execution**:
```cpp
// Working directory structure:
// myapp/
//   ├── src/
//   │   └── main.cpp
//   └── utils/
//       └── helper.cpp

repo.addSingleFile("src/main.cpp");

// Result in staging:
// staging_area/
//   └── src/
//       └── main.cpp
```

---

#### `void addAll()`
**Purpose**: Adds all files in current directory to staging (equivalent to `git add .`).

**Algorithm**:
```
STEP 1: Validate repository
    IF NOT isRepoInitialized():
        Print error
        Return

STEP 2: Collect all files
    allFiles = empty vector
    
    FOR EACH entry in directory_iterator(current_path()):
        filename = entry.filename()
        
        Skip if:
            - filename is ".Minivcs"
            - filename is ".git"
            - filename starts with '.' (hidden files)
        
        Add filename to allFiles vector

STEP 3: Check if any files found
    IF allFiles is empty:
        Print "No files to add"
        Return

STEP 4: Add all collected files
    Print "Adding all files..."
    Call add(allFiles)
```

**Dependencies**:
- `isRepoInitialized()` - Validates repository
- `add(vector<string>)` - Adds collected files

**Example**:
```cpp
// Working directory:
// myapp/
//   ├── main.cpp
//   ├── helper.cpp
//   └── .gitignore (skipped)

repo.addAll();

// Adds: main.cpp, helper.cpp
// Skips: .gitignore, .Minivcs
```

---

#### `void copyRecursive(const filesystem::path& src, const filesystem::path& dest)`
**Purpose**: Recursively copies directory and all its contents (private helper).

**Parameters**:
- `src`: Source directory path
- `dest`: Destination directory path

**Algorithm**:
```
STEP 1: Check if source path contains VCS directories
    FOR EACH part in src path:
        IF part is ".Minivcs" OR ".git":
            Return (do not enter VCS directories)

STEP 2: Handle directory
    IF src is a directory:
        Create dest directory
        
        FOR EACH entry in directory_iterator(src):
            srcPath = entry.path()
            destPath = dest / srcPath.filename()
            
            Recursively call copyRecursive(srcPath, destPath)

STEP 3: Handle file
    ELSE:
        Copy file from src to dest with overwrite option
```

**Dependencies**: None (leaf function - uses recursion)

**Example**:
```cpp
// Source structure:
// utils/
//   ├── parser.cpp
//   └── logger.cpp

copyRecursive("utils", "staging_area/utils");

// Result:
// staging_area/utils/
//   ├── parser.cpp
//   └── logger.cpp
```

**Recursive Depth**: Handles arbitrarily nested directories

---

#### `void clearStagingArea()`
**Purpose**: Removes all files from staging area.

**Algorithm**:
```
STEP 1: Validate repository
    IF NOT isRepoInitialized():
        Return

STEP 2: Clear staging directory
    IF stagingPath exists:
        FOR EACH entry in directory_iterator(stagingPath):
            Remove entry (files and directories recursively)
        
        Print "Staging area cleared"
```

**Dependencies**:
- `isRepoInitialized()` - Validates repository

**Example**:
```cpp
// Before:
// staging_area/
//   ├── main.cpp
//   └── utils/helper.cpp

repo.clearStagingArea();

// After:
// staging_area/
//   (empty)
```

---

#### `vector<string> getTrackedFiles() const`
**Purpose**: Returns list of all files currently in staging area.

**Algorithm**:
```
STEP 1: Initialize empty result vector
    stagedFiles = empty vector

STEP 2: Validate repository and staging area
    IF NOT isRepoInitialized() OR NOT exists(stagingPath):
        Return empty vector

STEP 3: Collect all staged files
    FOR EACH entry in recursive_directory_iterator(stagingPath):
        IF entry is a regular file (not directory):
            Calculate relative path from stagingPath
            Add relative path string to stagedFiles

STEP 4: Return list
    Return stagedFiles
```

**Dependencies**:
- `isRepoInitialized()` - Validates repository

**Returns**: Vector of relative file paths

**Example**:
```cpp
// Staging area contains:
// staging_area/
//   ├── main.cpp
//   └── src/utils.cpp

vector<string> files = repo.getTrackedFiles();
// files = {"main.cpp", "src/utils.cpp"}
```

---

#### `bool isStagingEmpty() const`
**Purpose**: Checks if staging area has any files.

**Algorithm**:
```
IF NOT isRepoInitialized() OR NOT exists(stagingPath):
    Return true

Return is_empty(stagingPath)
```

**Dependencies**:
- `isRepoInitialized()` - Validates repository

**Returns**: 
- `true` if staging area is empty or doesn't exist
- `false` if staging area contains files

---

#### Getter Methods

**Simple Accessors** (no dependencies):
- `filesystem::path getRepoPath() const` - Returns repository root path
- `filesystem::path getStagingPath() const` - Returns staging area path
- `filesystem::path getCommitsPath() const` - Returns commits directory path
- `string getRepoName() const` - Returns repository name

---

## Restore Class

### Overview
Implements undo/redo functionality using two stacks. Manages navigation through commit history independently of the linked list structure.

### Data Members

| Member | Type | Description |
|--------|------|-------------|
| `undoStack` | `Stack` | Contains commits that can be undone (previous states) |
| `redoStack` | `Stack` | Contains commits that can be redone (forward states) |
| `repo` | `Repository*` | Pointer to repository for checkout operations |
| `currentCommitID` | `string` | ID of currently checked-out commit |

### Constructor/Destructor

#### `Restore(Repository* repository)`
**Purpose**: Initializes restore system and loads saved state from disk.

**Parameters**:
- `repository`: Pointer to Repository object

**Algorithm**:
```
STEP 1: Initialize members
    repo = repository
    currentCommitID = "NA"

STEP 2: Check if repository is valid
    IF repo is NOT NULL AND repo is initialized:
        Call loadStateFromDisk()
        
        IF currentCommitID is still "NA":
            // No saved state exists, initialize from HEAD
            headCommit = repo->getHead()
            
            IF headCommit != "NA":
                currentCommitID = headCommit
                Call saveStateToDisk()
```

**Dependencies**:
- `loadStateFromDisk()` - Loads undo/redo stacks from persistent storage
- `Repository::isRepoInitialized()` - Verifies repository exists
- `Repository::getHead()` - Gets current HEAD commit
- `saveStateToDisk()` - Saves initial state

**Data Structure**: Reconstructs both stacks from disk on initialization

**Example**:
```cpp
Repository repo("myproject");
Restore restore(&repo);
// Loads previous undo/redo state if exists
// Or initializes to current HEAD
```

---

#### `~Restore()`
**Purpose**: Destructor that persists current state to disk before destruction.

**Algorithm**:
```
Call saveStateToDisk()
```

**Dependencies**:
- `saveStateToDisk()` - Saves undo/redo stacks before destruction

**Example**:
```cpp
{
    Restore restore(&repo);
    // ... perform operations ...
} // Destructor called here, state saved automatically
```

---

### Core Methods

#### `void recordCommit(const string& commitID)`
**Purpose**: Records a new commit in the undo/redo system when user creates a commit.

**Parameters**:
- `commitID`: ID of the newly created commit

**Algorithm**:
```
STEP 1: Save current state to undo stack
    IF currentCommitID != "NA":
        undoStack.push(currentCommitID)

STEP 2: Clear redo stack (new commit creates new timeline)
    WHILE NOT redoStack.isEmpty():
        redoStack.pop()

STEP 3: Update current commit
    currentCommitID = commitID

STEP 4: Persist state to disk
    Call saveStateToDisk()
```

**Dependencies**:
- `Stack::push()` - Adds to undo stack
- `Stack::isEmpty()` - Checks redo stack
- `Stack::pop()` - Clears redo stack
- `saveStateToDisk()` - Persists state

**Visual Example**:
```
Before recordCommit("c3"):
    Undo: [c1, c2]
    Current: c2
    Redo: [c4, c5]

After recordCommit("c3"):
    Undo: [c1, c2]    ← c2 added
    Current: c3       ← updated
    Redo: []          ← CLEARED (c4, c5 lost)
```

**Data Structure**: **Stack push** operation - O(1)

**Important**: Creating new commit **clears redo stack** (can't redo after new commit, creates new branch)

---

#### `bool undo()`
**Purpose**: Moves back one commit in history using undo stack.

**Algorithm**:
```
STEP 1: Check if undo is possible
    IF undoStack.isEmpty():
        Print "Cannot undo! No previous commits available."
        Return false

STEP 2: Save current commit to redo stack
    redoStack.push(currentCommitID)

STEP 3: Get previous commit from undo stack
    currentCommitID = undoStack.pop()

STEP 4: Update working directory to previous commit
    Call repo->checkout(currentCommitID)

STEP 5: Persist new state
    Call saveStateToDisk()

STEP 6: Return success
    Return true
```

**Dependencies**:
- `Stack::isEmpty()` - Checks if undo possible
- `Stack::push()` - Saves current to redo stack
- `Stack::pop()` - Gets previous commit from undo stack
- `Repository::checkout()` - Updates working directory to previous commit
- `saveStateToDisk()` - Persists state

**Visual Example**:
```
Before undo():
    Undo: [c1, c2]
    Current: c3
    Redo: []

After undo():
    Undo: [c1]        ← c2 popped
    Current: c2       ← moved back
    Redo: [c3]        ← c3 pushed

Working directory now contains c2's files
```

**Data Structure**: 
- **Stack pop** from undo stack - O(1)
- **Stack push** to redo stack - O(1)

**Effect**: Moves backward in commit history

**Calls Chain**:
```
main::undo
  └─> Restore::undo()
        ├─> Stack::isEmpty()
        ├─> Stack::push()
        ├─> Stack::pop()
        ├─> Repository::checkout()
        │     └─> Repository::setHead()
        └─> Restore::saveStateToDisk()
```

---

#### `bool redo()`
**Purpose**: Moves forward one commit in history using redo stack.

**Algorithm**:
```
STEP 1: Check if redo is possible
    IF redoStack.isEmpty():
        Print "Cannot redo! No forward commits available."
        Return false

STEP 2: Save current commit to undo stack
    undoStack.push(currentCommitID)

STEP 3: Get next commit from redo stack
    currentCommitID = redoStack.pop()

STEP 4: Update working directory to next commit
    Call repo->checkout(currentCommitID)

STEP 5: Persist new state
    Call saveStateToDisk()

STEP 6: Return success
    Return true
```

**Dependencies**:
- `Stack::isEmpty()` - Checks if redo possible
- `Stack::push()` - Saves current to undo stack
- `Stack::pop()` - Gets next commit from redo stack
- `Repository::checkout()` - Updates working directory to next commit
- `saveStateToDisk()` - Persists state

**Visual Example**:
```
Before redo():
    Undo: [c1]
    Current: c2
    Redo: [c3, c4]

After redo():
    Undo: [c1, c2]    ← c2 pushed
    Current: c3       ← moved forward
    Redo: [c4]        ← c3 popped

Working directory now contains c3's files
```

**Data Structure**: 
- **Stack pop** from redo stack - O(1)
- **Stack push** to undo stack - O(1)

**Effect**: Moves forward in commit history

**Calls Chain**:
```
main::redo
  └─> Restore::redo()
        ├─> Stack::isEmpty()
        ├─> Stack::push()
        ├─> Stack::pop()
        ├─> Repository::checkout()
        │     └─> Repository::setHead()
        └─> Restore::saveStateToDisk()
```

---

#### `void loadHistory(CommitNode* head)`
**Purpose**: Rebuilds undo/redo stacks from commit linked list.

**Parameters**:
- `head`: Pointer to HEAD (latest commit) of linked list

**Algorithm**:
```
STEP 1: Check if head is valid
    IF head is NULL:
        Return

STEP 2: Clear existing stacks
    Call clear()

STEP 3: Collect all commit IDs from linked list
    commits = empty vector
    current = head
    
    WHILE current != NULL:
        Add current->getCommitID() to commits vector
        current = current->getNextNode()

STEP 4: Push commits to undo stack (reverse order)
    FOR i from (commits.size() - 1) down to 1:
        undoStack.push(commits[i])
    
    (Note: Skip index 0 which is head/current)

STEP 5: Set current to head
    IF commits is not empty:
        currentCommitID = commits[0]

STEP 6: Persist state
    Call saveStateToDisk()
```

**Dependencies**:
- `clear()` - Empties both stacks
- `Stack::push()` - Populates undo stack
- `CommitNode::getCommitID()` - Gets commit IDs
- `CommitNode::getNextNode()` - Traverses linked list forward
- `saveStateToDisk()` - Persists state

**Visual Example**:
```
Linked List:
    [c1] → [c2] → [c3] → NULL
                   ↑
                  head

After loadHistory(head):
    Undo: [c1, c2]     ← oldest to newest (excluding head)
    Current: c3        ← set to head
    Redo: []           ← empty
```

**Data Structure**: Converts **linked list to stack** representation

**Time Complexity**: O(n) where n = number of commits

**Use Case**: Called when system starts or after loading commits from disk

---

#### `void saveStateToDisk() const`
**Purpose**: Persists undo/redo stacks and current state to disk file.

**Algorithm**:
```
STEP 1: Validate repository
    IF repo is NULL OR NOT initialized:
        Return

STEP 2: Build file path
    restorePath = repo->getRepoPath() / "restore_state.txt"

STEP 3: Open file for writing
    Open restorePath
    IF cannot open:
        Print error
        Return

STEP 4: Write current commit
    Write: "CURRENT:<currentCommitID>\n"

STEP 5: Write undo stack
    Write: "UNDO_SIZE:<undoStack.size()>\n"
    
    FOR i from 0 to undoStack.size() - 1:
        Write: "UNDO:<undoStack.data[i]>\n"

STEP 6: Write redo stack
    Write: "REDO_SIZE:<redoStack.size()>\n"
    
    FOR i from 0 to redoStack.size() - 1:
        Write: "REDO:<redoStack.data[i]>\n"

STEP 7: Close file
```

**Dependencies**:
- `Repository::getRepoPath()` - Gets repository root path
- `Repository::isRepoInitialized()` - Verifies repository
- `Stack::size()` - Gets stack sizes

**File Format Example** (restore_state.txt):
```
CURRENT:c3a2b1d4e5f6g7h8
UNDO_SIZE:2
UNDO:a1b2c3d4e5f6g7h8
UNDO:b2c3d4e5f6g7h8i9
REDO_SIZE:1
REDO:d4e5f6g7h8i9j0k1
```

**Data Structure**: Serializes stack contents to disk for persistence across sessions

---

#### `void loadStateFromDisk()`
**Purpose**: Restores undo/redo stacks and current state from disk file.

**Algorithm**:
```
STEP 1: Validate repository
    IF repo is NULL OR NOT initialized:
        Return

STEP 2: Build file path
    restorePath = repo->getRepoPath() / "restore_state.txt"

STEP 3: Check if file exists
    IF NOT exists(restorePath):
        Return (no saved state)

STEP 4: Open file for reading
    Open restorePath
    IF cannot open:
        Return

STEP 5: Clear existing data
    Clear undoStack completely
    Clear redoStack completely

STEP 6: Read file line by line
    FOR EACH line in file:
        IF line starts with "CURRENT:":
            currentCommitID = substring after "CURRENT:"
        
        ELSE IF line starts with "UNDO:":
            undoStack.push(substring after "UNDO:")
        
        ELSE IF line starts with "REDO:":
            redoStack.push(substring after "REDO:")
        
        (UNDO_SIZE and REDO_SIZE are informational, not used)

STEP 7: Close file
```

**Dependencies**:
- `Repository::getRepoPath()` - Gets repository root path
- `Repository::isRepoInitialized()` - Verifies repository
- `Stack::isEmpty()` - Clears old data
- `Stack::pop()` - Clears old data
- `Stack::push()` - Rebuilds stacks

**Example Execution**:
```cpp
// restore_state.txt contains:
// CURRENT:c3
// UNDO_SIZE:2
// UNDO:c1
// UNDO:c2
// REDO_SIZE:0

loadStateFromDisk();

// Result:
// currentCommitID = "c3"
// undoStack = [c1, c2]
// redoStack = []
```

**Data Structure**: Deserializes stack contents from disk

**Use Case**: Called during Restore constructor to restore previous session state

---

#### Helper Methods

#### `bool canUndo() const`
**Purpose**: Checks if undo operation is possible.

**Algorithm**:
```
Return NOT undoStack.isEmpty()
```

**Dependencies**:
- `Stack::isEmpty()` - Checks stack

---

#### `bool canRedo() const`
**Purpose**: Checks if redo operation is possible.

**Algorithm**:
```
Return NOT redoStack.isEmpty()
```

**Dependencies**:
- `Stack::isEmpty()` - Checks stack

---

#### `string getCurrentCommit() const`
**Purpose**: Returns ID of currently checked-out commit.

**Dependencies**: None (leaf function)

---

#### `string getUndoTarget() const`
**Purpose**: Returns ID of commit that would be checked out on undo.

**Algorithm**:
```
IF undoStack.isEmpty():
    Return "NA"
ELSE:
    Return undoStack.peek()
```

**Dependencies**:
- `Stack::isEmpty()` - Boundary check
- `Stack::peek()` - Views top without removing

---

#### `string getRedoTarget() const`
**Purpose**: Returns ID of commit that would be checked out on redo.

**Algorithm**:
```
IF redoStack.isEmpty():
    Return "NA"
ELSE:
    Return redoStack.peek()
```

**Dependencies**:
- `Stack::isEmpty()` - Boundary check
- `Stack::peek()` - Views top without removing

---

#### `int getUndoStackSize() const`
**Purpose**: Returns number of commits in undo stack.

**Dependencies**:
- `Stack::size()` - Gets stack size

---

#### `int getRedoStackSize() const`
**Purpose**: Returns number of commits in redo stack.

**Dependencies**:
- `Stack::size()` - Gets stack size

---

#### `void clear()`
**Purpose**: Clears both undo and redo stacks, resets current commit.

**Algorithm**:
```
WHILE NOT undoStack.isEmpty():
    undoStack.pop()

WHILE NOT redoStack.isEmpty():
    redoStack.pop()

currentCommitID = "NA"

Call saveStateToDisk()
```

**Dependencies**:
- `Stack::isEmpty()` - Checks stacks
- `Stack::pop()` - Empties stacks
- `saveStateToDisk()` - Persists cleared state

---

#### `void printStatus() const`
**Purpose**: Displays current undo/redo system status for debugging.

**Algorithm**:
```
STEP 1: Print header
    Print separator line
    Print "Restore Status:"

STEP 2: Print current commit
    Print "Current: <currentCommitID or 'None'>"

STEP 3: Print stack sizes and capabilities
    Print "Undo Stack: <size> commits | Can Undo: <Yes/No>"
    Print "Redo Stack: <size> commits | Can Redo: <Yes/No>"

STEP 4: Print next operations (if available)
    IF canUndo():
        Print "Next Undo → <getUndoTarget()>"
    IF canRedo():
        Print "Next Redo → <getRedoTarget()>"

STEP 5: Print detailed stack contents
    IF undoStack has elements:
        Print "Undo Stack (bottom → top):"
        FOR i from 0 to undoStack.size() - 1:
            Print "  [<i>] <undoStack.data[i]>"
    
    IF redoStack has elements:
        Print "Redo Stack (bottom → top):"
        FOR i from 0 to redoStack.size() - 1:
            Print "  [<i>] <redoStack.data[i]>"

STEP 6: Print footer
    Print separator line
```

**Dependencies**:
- `getCurrentCommit()` - Gets current commit
- `getUndoStackSize()` - Gets undo stack size
- `getRedoStackSize()` - Gets redo stack size
- `canUndo()` - Checks if undo possible
- `canRedo()` - Checks if redo possible
- `getUndoTarget()` - Gets next undo commit
- `getRedoTarget()` - Gets next redo commit

**Output Example**:
```
========================================
Restore Status:
========================================
Current: c3a2b1d4e5f6g7h8
Undo Stack: 2 commits | Can Undo: Yes
Redo Stack: 1 commits | Can Redo: Yes
Next Undo → b2c3d4e5f6g7h8i9
Next Redo → d4e5f6g7h8i9j0k1

Undo Stack (bottom → top):
  [0] a1b2c3d4e5f6g7h8
  [1] b2c3d4e5f6g7h8i9

Redo Stack (bottom → top):
  [0] d4e5f6g7h8i9j0k1
========================================
```

---

#### `void viewHistory(CommitNode* head) const`
**Purpose**: Displays commit history with current position marked.

**Parameters**:
- `head`: Pointer to HEAD of commit linked list

**Algorithm**:
```
STEP 1: Check if commits exist
    IF head is NULL:
        Print "No commits found"
        Return

STEP 2: Print header
    Print "========== COMMIT HISTORY =========="

STEP 3: Traverse and print each commit
    FOR current from head to NULL (using getNextNode()):
        IF current->getCommitID() == currentCommitID:
            Print " → [CURRENT] Commit: <commitID>"
        ELSE:
            Print "             Commit: <commitID>"

STEP 4: Print footer
    Print separator line
```

**Dependencies**:
- `CommitNode::getCommitID()` - Gets commit ID
- `CommitNode::getNextNode()` - Traverses list

**Output Example**:
```
========== COMMIT HISTORY ==========
             Commit: d4e5f6g7h8i9j0k1
 → [CURRENT] Commit: c3a2b1d4e5f6g7h8
             Commit: b2c3d4e5f6g7h8i9
             Commit: a1b2c3d4e5f6g7h8
====================================
```

---

## Stack Class

### Overview
Generic stack implementation using vector as underlying storage. Provides LIFO (Last-In-First-Out) operations for storing commit IDs.

### Data Members

| Member | Type | Description |
|--------|------|-------------|
| `data` | `vector<string>` | Stores stack elements (commit IDs) |
| `top` | `int` | Index of top element (-1 when empty) |

### Constructor

#### `Stack()`
**Purpose**: Initializes empty stack.

**Algorithm**:
```
top = -1
data = empty vector
```

**Dependencies**: None (leaf function)

---

### Methods

#### `void push(const string& value)`
**Purpose**: Adds element to top of stack.

**Parameters**:
- `value`: Commit ID to push onto stack

**Algorithm**:
```
1. data.push_back(value)
2. top++
```

**Time Complexity**: O(1) amortized (vector may reallocate)

**Dependencies**: None (leaf function)

**Example**:
```cpp
Stack s;
s.push("c1");  // top = 0
s.push("c2");  // top = 1
s.push("c3");  // top = 2

// Stack: [c1, c2, c3]
//                  ↑
//                 top
```

---

#### `string pop()`
**Purpose**: Removes and returns top element.

**Algorithm**:
```
STEP 1: Check if stack is empty
    IF isEmpty():
        throw out_of_range("Stack underflow")

STEP 2: Get top element
    value = data.back()

STEP 3: Remove top element
    data.pop_back()
    top--

STEP 4: Return value
    Return value
```

**Time Complexity**: O(1)

**Dependencies**: 
- `isEmpty()` - Boundary check

**Example**:
```cpp
Stack s;
s.push("c1");
s.push("c2");
s.push("c3");

string val = s.pop();  // Returns "c3", top = 1
// Stack: [c1, c2]
//             ↑
//            top
```

---

#### `const string& peek() const`
**Purpose**: Returns top element without removing it.

**Algorithm**:
```
STEP 1: Check if stack is empty
    IF isEmpty():
        throw out_of_range("Stack is empty")

STEP 2: Return top element
    Return data[top]
```

**Time Complexity**: O(1)

**Dependencies**: 
- `isEmpty()` - Boundary check

**Example**:
```cpp
Stack s;
s.push("c1");
s.push("c2");

const string& val = s.peek();  // Returns "c2", doesn't remove
// Stack still: [c1, c2]
```

---

#### `bool isEmpty() const`
**Purpose**: Checks if stack is empty.

**Algorithm**:
```
Return (top < 0)
```

**Time Complexity**: O(1)

**Dependencies**: None (leaf function)

**Example**:
```cpp
Stack s;
cout << s.isEmpty();  // true

s.push("c1");
cout << s.isEmpty();  // false
```

---

#### `int size() const`
**Purpose**: Returns number of elements in stack.

**Algorithm**:
```
Return (top + 1)
```

**Time Complexity**: O(1)

**Dependencies**: None (leaf function)

**Example**:
```cpp
Stack s;
s.push("c1");
s.push("c2");
s.push("c3");

cout << s.size();  // Outputs: 3
```

---

## HashingHelper

### Overview
Generates unique commit IDs using FNV-1a hashing algorithm combined with hardware randomness and high-precision timestamps.

### Key Function

#### `string generateCommitID(const string& data)`
**Purpose**: Creates unique 16-character hexadecimal commit ID.

**Parameters**:
- `data`: Optional additional data to include in hash (default: "")

**Algorithm**:
```
STEP 1: Generate hardware random number
    Create random_device object (uses hardware entropy)
    
    Generate 64-bit random:
        upper32 = rand() casted to uint64_t
        lower32 = rand()
        randomNumber = (upper32 << 32) XOR lower32
    
    Example: 0xA1B2C3D4E5F6G7H8

STEP 2: Get high-precision timestamp
    Use high_resolution_clock
    Get time since epoch (Jan 1, 1970)
    Extract count in nanoseconds
    
    Example: 1702134622123456789

STEP 3: Combine all data
    combinedData = data + to_string(randomNumber) + to_string(time)
    
    Example: ""  + "11657...8912" + "1702...6789"

STEP 4: Apply FNV-1a hash
    hash = FNV1A(combinedData)
    
    Returns 64-bit unsigned integer

STEP 5: Format as hexadecimal
    Create stringstream
    Set format to hexadecimal
    Set width to 16 characters
    Set fill character to '0'
    Write hash to stream
    
    Example: 016a3f2b4c5d6e7f

STEP 6: Return formatted ID
    Return stream as string
```

**Dependencies**:
- `FNV1A(string)` - Internal hash function

**FNV-1a Hash Algorithm** (internal function):
```
static uint64_t FNV1A(const string& data):
    STEP 1: Initialize constants
        offset = 14695981039346656037ULL (FNV offset basis for 64-bit)
        prime  = 1099511628211ULL       (FNV prime for 64-bit)
    
    STEP 2: Initialize hash
        hash = offset
    
    STEP 3: Process each character
        FOR i from 0 to data.length() - 1:
            hash = hash XOR data[i]      // XOR with byte
            hash = hash * prime           // Multiply by prime
    
    STEP 4: Return hash
        Return hash
```

**Output Format**: `016a3f2b4c5d6e7f` (16 hexadecimal digits = 8 bytes = 64 bits)

**Collision Resistance**: Extremely low probability due to:
- 64-bit hardware randomness (2^64 possible values)
- Nanosecond precision timestamp (unique at each moment)
- Cryptographic-quality FNV-1a hash function
- Combined probability ≈ 1 in 18 quintillion

**Example**:
```cpp
string id1 = generateCommitID();
// Returns: "a1b2c3d4e5f6g7h8"

string id2 = generateCommitID("extra data");
// Returns: "b2c3d4e5f6g7h8i9" (different due to extra data)
```

---

## Data Structure Interaction Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      CommitManager                          │
│  ┌────────────────────────────────────────────────────┐    │
│  │        Singly Linked List (Commit History)         │    │
│  │                                                      │    │
│  │    [TAIL] → [C1] → [C2] → [C3] → [HEAD] → NULL     │    │
│  │       ↑                             ↑               │    │
│  │       └─────────────────────────────┘               │    │
│  │          (unidirectional traversal)                 │    │
│  └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           ↕
              (checkout operations)
                           ↕
┌─────────────────────────────────────────────────────────────┐
│                      Restore System                         │
│  ┌──────────────────────┐      ┌──────────────────────┐    │
│  │    Undo Stack        │      │    Redo Stack        │    │
│  │    ┌────────┐        │      │    ┌────────┐        │    │
│  │    │  C2    │ ← top  │      │    │  C4    │ ← top  │    │
│  │    ├────────┤        │      │    ├────────┤        │    │
│  │    │  C1    │        │      │    └────────┘        │    │
│  │    ├────────┤        │      │                      │    │
│  │    │  C0    │        │      │  Current: C3         │    │
│  │    └────────┘        │      │                      │    │
│  └──────────────────────┘      └──────────────────────┘    │
│                                                             │
│  Operations:                                                │
│  • recordCommit(): push to undo, clear redo                │
│  • undo(): pop undo, push redo, checkout                   │
│  • redo(): pop redo, push undo, checkout                   │
└─────────────────────────────────────────────────────────────┘
```

---

## Complete Function Dependency Trees

### Creating a Commit (Full Call Chain)

```
main::commit
  │
  ├─> CommitManager::addCommit(msg)
  │     │
  │     ├─> generateCommitID() [HashingHelper]
  │     │     └─> FNV1A(data) [internal]
  │     │
  │     ├─> CommitNode::CommitNode(id, msg, repo)
  │     │     ├─> Repository::getCommitsPath()
  │     │     ├─> Repository::getStagingPath()
  │     │     └─> CommitNode::createCommitData(stagingPath)
  │     │
  │     ├─> (if not first commit)
  │     │   ├─> CommitNode::setNextNode(newNode)
  │     │   ├─> CommitNode::setNextID(id)
  │     │   ├─> CommitNode::saveNextID(id)
  │     │   └─> CommitNode::getCommitID()
  │     │
  │     └─> Repository::getCommitsPath()
  │
  ├─> Repository::getHead()
  │
  ├─> Restore::recordCommit(newCommitID)
  │     ├─> Stack::push() [undo stack]
  │     ├─> Stack::isEmpty() [redo stack]
  │     ├─> Stack::pop() [clear redo]
  │     └─> Restore::saveStateToDisk()
  │           ├─> Repository::getRepoPath()
  │           ├─> Repository::isRepoInitialized()
  │           └─> Stack::size() [both stacks]
  │
  └─> Repository::clearStagingArea()
        └─> Repository::isRepoInitialized()
```

### Undo Operation (Full Call Chain)

```
main::undo
  │
  └─> Restore::undo()
        │
        ├─> Stack::isEmpty() [check undo stack]
        │
        ├─> Stack::push(currentCommitID) [to redo stack]
        │
        ├─> Stack::pop() [from undo stack]
        │     └─> Stack::isEmpty() [boundary check]
        │
        ├─> Repository::checkout(currentCommitID)
        │     │
        │     ├─> Repository::isRepoInitialized()
        │     │
        │     ├─> (Remove working directory files)
        │     │
        │     ├─> (Copy commit files to working directory)
        │     │
        │     └─> Repository::setHead(commitID)
        │
        └─> Restore::saveStateToDisk()
              ├─> Repository::getRepoPath()
              ├─> Repository::isRepoInitialized()
              └─> Stack::size() [both stacks]
```

### Redo Operation (Full Call Chain)

```
main::redo
  │
  └─> Restore::redo()
        │
        ├─> Stack::isEmpty() [check redo stack]
        │
        ├─> Stack::push(currentCommitID) [to undo stack]
        │
        ├─> Stack::pop() [from redo stack]
        │     └─> Stack::isEmpty() [boundary check]
        │
        ├─> Repository::checkout(currentCommitID)
        │     │
        │     ├─> Repository::isRepoInitialized()
        │     │
        │     ├─> (Remove working directory files)
        │     │
        │     ├─> (Copy commit files to working directory)
        │     │
        │     └─> Repository::setHead(commitID)
        │
        └─> Restore::saveStateToDisk()
              ├─> Repository::getRepoPath()
              ├─> Repository::isRepoInitialized()
              └─> Stack::size() [both stacks]
```

### Revert Operation (Full Call Chain)

```
main::revert
  │
  ├─> CommitManager::revert(commitID)
  │     │
  │     ├─> Repository::getCommitsPath()
  │     │
  │     ├─> Repository::getStagingPath()
  │     │
  │     ├─> (Clear staging area)
  │     │
  │     ├─> (Copy source commit to staging)
  │     │
  │     └─> CommitManager::addCommit("Revert to " + commitID)
  │           │
  │           ├─> generateCommitID() [HashingHelper]
  │           │
  │           ├─> CommitNode::CommitNode(id, msg, repo)
  │           │     └─> CommitNode::createCommitData()
  │           │
  │           └─> (Update HEAD and link nodes)
  │
  ├─> Repository::getHead()
  │
  └─> Restore::recordCommit(newCommitID)
        ├─> Stack::push() [undo stack]
        ├─> Stack::isEmpty() [redo stack]
        ├─> Stack::pop() [clear redo]
        └─> Restore::saveStateToDisk()
```

### Loading System (Full Call Chain)

```
CommitManager::CommitManager(repo)
  │
  ├─> Repository::isRepoInitialized()
  │
  └─> CommitManager::loadListFromDisk()
        │
        ├─> Repository::getCommitsPath()
        │
        ├─> (Read HEAD.txt and TAIL.txt)
        │
        ├─> CommitManager::loadSingleNode(tailID)
        │     │
        │     └─> CommitNode::CommitNode(id, repo)
        │           │
        │           ├─> Repository::getCommitsPath()
        │           │
        │           └─> CommitNode::loadNodeInfo()
        │                 └─> CommitNode::loadNextCommitID()
        │
        └─> (Loop: load each node and link)
              ├─> CommitNode::getNextID()
              ├─> CommitManager::loadSingleNode(nextID)
              └─> CommitNode::setNextNode()

Restore::Restore(repo)
  │
  ├─> Repository::isRepoInitialized()
  │
  ├─> Restore::loadStateFromDisk()
  │     │
  │     ├─> Repository::getRepoPath()
  │     │
  │     ├─> Repository::isRepoInitialized()
  │     │
  │     ├─> Stack::isEmpty() [clear old data]
  │     │
  │     ├─> Stack::pop() [clear old data]
  │     │
  │     └─> Stack::push() [rebuild stacks]
  │
  ├─> Repository::getHead()
  │
  └─> Restore::saveStateToDisk()
```

### Adding Files (Full Call Chain)

```
main::add
  │
  └─> Repository::add(files)
        │
        ├─> Repository::isRepoInitialized()
        │
        └─> (For each file)
              └─> Repository::addSingleFile(filename)
                    │
                    ├─> Repository::copyRecursive(src, dest)
                    │     └─> (Recursive call to copyRecursive)
                    │
                    └─> (File system copy operations)

main::addall
  │
  └─> Repository::addAll()
        │
        ├─> Repository::isRepoInitialized()
        │
        └─> Repository::add(allFiles)
              └─> (Same as above)
```

### Print Log (Full Call Chain)

```
main::log
  │
  └─> CommitManager::printLog()
        │
        ├─> Repository::getCommitsPath()
        │
        └─> (Loop through commits)
              ├─> CommitNode::getCommitID()
              ├─> CommitNode::getCommitMsg()
              ├─> (Read timestamp from info.txt)
              └─> CommitNode::getNextNode()
```

---

## Algorithm Complexity Analysis

| Operation | Time Complexity | Space Complexity | Data Structure | Notes |
|-----------|----------------|------------------|----------------|-------|
| **Commit Operations** |
| Add Commit | O(1) + O(f) | O(f) | Linked List insertion at head | f = number of files |
| Load Commits | O(n) | O(n) | Linked List reconstruction | n = number of commits |
| Print Log | O(n) | O(1) | Linked List traversal | n = number of commits |
| Revert | O(f) | O(f) | Creates new commit | f = number of files |
| **Stack Operations** |
| Undo | O(1) + O(f) | O(1) | Stack pop + file operations | f = number of files |
| Redo | O(1) + O(f) | O(1) | Stack pop + file operations | f = number of files |
| Record Commit | O(m) | O(1) | Stack operations | m = redo stack size (cleared) |
| **Repository Operations** |
| Init | O(1) | O(1) | Create directories | - |
| Add File | O(f) | O(f) | Copy files | f = number of files |
| Add All | O(f) | O(f) | Copy all files | f = total files in directory |
| Checkout | O(f) | O(f) | File operations | f = number of files |
| Clear Staging | O(f) | O(1) | Delete files | f = number of staged files |
| **Hash Operations** |
| Generate ID | O(k) | O(1) | String hashing | k = string length |
| FNV-1a Hash | O(k) | O(1) | Character iteration | k = string length |
| **Stack ADT** |
| Push | O(1) | O(1) | Vector append | Amortized |
| Pop | O(1) | O(1) | Vector pop_back | - |
| Peek | O(1) | O(1) | Array access | - |
| isEmpty | O(1) | O(1) | Comparison | - |
| Size | O(1) | O(1) | Arithmetic | - |

**Legend**:
- n = number of commits
- f = number of files
- k = string length
- m = redo stack size

**Note**: File operation costs (copying files during checkout, commit) dominate the complexity for most operations. The data structure operations themselves (list/stack manipulations) are all O(1).

---

## Design Patterns & Principles

### 1. **Separation of Concerns**
Each class has a single, well-defined responsibility:

```
CommitManager    → Manages linked list structure
Restore          → Manages navigation using stacks
Repository       → Handles file system operations
CommitNode       → Represents individual commits
Stack            → Provides LIFO data structure
HashingHelper    → Generates unique IDs
```

### 2. **Persistence Strategy**
Two-layer architecture for data persistence:

```
Memory Layer:         Disk Layer:
─────────────         ───────────
Linked List    ←→     commits/<id>/info.txt
                      commits/<id>/NextCommit.txt
                      commits/<id>/Data/

Stacks         ←→     restore_state.txt

Current State  ←→     HEAD.txt
```

**Benefits**:
- State survives program restarts
- Can reconstruct data structures from disk
- Clear separation between runtime and persistent storage

### 3. **Factory Pattern**
`CommitManager::loadSingleNode()` acts as a factory method:
- Encapsulates node creation logic
- Handles errors gracefully (returns nullptr)
- Centralizes node instantiation

### 4. **Two-Level Navigation Architecture**

```
Application Layer:  Undo/Redo (Stack-based navigation)
                           ↕
                    (Uses checkout)
                           ↕
Storage Layer:      Commit History (Linked List structure)
```

**Why this separation?**
- **Flexibility**: Navigate independently of commit structure
- **State preservation**: Undo/redo state persists across sessions
- **Modularity**: Can change navigation without affecting storage
- **Different semantics**: List = storage, Stacks = navigation history

### 5. **Snapshot Model**
Each commit stores **complete project state**, not deltas:

**Advantages**:
- Simple implementation
- Fast checkout (just copy files)
- No dependency chain (each commit is independent)

**Disadvantages**:
- High disk usage (duplicate files)
- Slower for large projects

### 6. **Forward-Only Linked List**
Uses singly-linked list instead of doubly-linked:

**Why forward-only?**
- Simpler implementation (one pointer per node)
- Less disk storage (one file: NextCommit.txt)
- Sufficient for forward traversal
- Undo/redo handled by separate stack system

### 7. **Immutable History**
Once created, commits never change:
- Revert creates **new commit** with old data
- Original commits preserved
- True version control semantics

---

## Key Design Insights

### Why Linked List for Commits?

**Advantages**:
1. **Dynamic growth**: Unknown number of commits at compile time
2. **Efficient insertion**: O(1) to add new commit at HEAD
3. **Natural ordering**: Sequential structure matches chronological history
4. **Simple traversal**: Forward iteration matches log display
5. **Persistent mapping**: Each node = one directory on disk

**Why not array/vector?**
- Would need reallocation as commits grow
- Harder to map to disk structure
- No advantage since we rarely index by position

### Why Two Stacks for Undo/Redo?

**Advantages**:
1. **LIFO semantics**: Undo reverses most recent actions first (natural)
2. **Branch handling**: New commit clears redo (prevents confusion)
3. **O(1) operations**: Push/pop are constant time
4. **Memory efficient**: Only store commit IDs, not full data
5. **Simple logic**: Clear mental model for users

**Visual Example**:
```
Create c1, c2, c3:
    Undo: [c1, c2]  Current: c3  Redo: []

Undo twice:
    Undo: []        Current: c1  Redo: [c3, c2]

Create c4 (new branch):
    Undo: [c1]      Current: c4  Redo: []  ← c2, c3 lost!
```

### Why Separate Navigation from Storage?

**Reasons**:
1. **Different lifetimes**: Commits are permanent, navigation is temporary
2. **Independent concerns**: Storage = history, Navigation = current position
3. **Flexibility**: Can implement different navigation strategies
4. **State management**: Undo/redo state needs separate persistence

**Example**:
```
Linked List:  [c1] → [c2] → [c3] → [c4]  (permanent structure)

Navigation:   User at c2, can undo to c1 or redo to c3
              (temporary position)
```

### Why FNV-1a Hash?

**Advantages**:
1. **Fast**: Simple operations (XOR, multiply)
2. **Good distribution**: Low collision probability
3. **Fixed size**: Always 64-bit output
4. **Simple implementation**: Easy to understand and verify

**Enhanced with**:
- Hardware randomness (true entropy)
- Nanosecond timestamp (temporal uniqueness)
- Combined probability: ~1 in 18 quintillion collisions

---

## Usage Examples with Detailed Explanation

### Example 1: Creating Commits

```cpp
Repository repo("myproject");
repo.initRepo();

// Add files to staging
vector<string> files = {"main.cpp", "utils/helper.cpp"};
repo.add(files);

CommitManager manager(&repo);
manager.addCommit("Initial commit");  // Creates c1

// Internal state:
// Linked List: [c1] → NULL
//               ↑      ↑
//             HEAD   TAIL
// 
// Disk:
//   commits/c1/info.txt
//   commits/c1/NextCommit.txt → "NA"
//   commits/c1/Data/main.cpp
//   commits/c1/Data/utils/helper.cpp
//   HEAD.txt → "c1"
//   TAIL.txt → "c1"

repo.add({"config.txt"});
manager.addCommit("Add config");  // Creates c2

// Internal state:
// Linked List: [c1] → [c2] → NULL
//               ↑      ↑
//             TAIL   HEAD
//
// Disk:
//   commits/c1/NextCommit.txt → "c2"  (updated)
//   commits/c2/info.txt
//   commits/c2/NextCommit.txt → "NA"
//   commits/c2/Data/ (all files)
//   HEAD.txt → "c2"  (updated)
```

### Example 2: Undo/Redo Navigation

```cpp
Restore restore(&repo);

// After 3 commits: c1 → c2 → c3
// Undo Stack: [c1, c2]
// Current: c3
// Redo Stack: []
// Working directory: files from c3

restore.undo();  // Move to c2

// Undo Stack: [c1]        ← c2 popped
// Current: c2             ← moved back
// Redo Stack: [c3]        ← c3 pushed
// Working directory: files from c2 (checkout executed)

restore.undo();  // Move to c1

// Undo Stack: []          ← c1 popped
// Current: c1             ← moved back
// Redo Stack: [c3, c2]    ← c2 pushed
// Working directory: files from c1

restore.redo();  // Move to c2

// Undo Stack: [c1]        ← c1 pushed
// Current: c2             ← moved forward
// Redo Stack: [c3]        ← c2 popped
// Working directory: files from c2

restore.redo();  // Move to c3

// Undo Stack: [c1, c2]    ← c2 pushed
// Current: c3             ← moved forward
// Redo Stack: []          ← c3 popped
// Working directory: files from c3
```

### Example 3: New Commit Clears Redo

```cpp
// Current state: at c2 with c3 in redo stack
// Linked List: [c1] → [c2] → [c3]
// Undo Stack: [c1]
// Current: c2
// Redo Stack: [c3]

repo.add({"newfile.cpp"});
manager.addCommit("New feature");  // Creates c4

restore.recordCommit("c4");

// Result:
// Linked List: [c1] → [c2] → [c3]
//                      ↓
//                     [c4] → NULL
//                      ↑
//                    HEAD
//
// Undo Stack: [c1, c2]    ← c2 added
// Current: c4             ← new commit
// Redo Stack: []          ← CLEARED! (c3 lost)
//
// Note: c3 still exists in linked list but is unreachable
// from current navigation path
```

### Example 4: Revert Operation

```cpp
// Current state:
// [c1] → [c2] → [c3]
//                ↑
//              HEAD

manager.revert("c1");  // Revert to c1

// Process:
// 1. Copy c1/Data/ to staging_area/
// 2. Create new commit c4 from staging
// 3. Copy c4/Data/ to working directory
//
// Result:
// [c1] → [c2] → [c3] → [c4]
//  ↑                     ↑
// (copied from)        HEAD
//
// c4 contains exact same files as c1
// but is a NEW commit (preserves history)

restore.recordCommit("c4");

// Undo Stack: [c1, c2, c3]
// Current: c4
// Redo Stack: []
```

### Example 5: Loading Existing Repository

```cpp
// Scenario: Program starts, repository already exists
// Disk state:
//   TAIL.txt: "c1"
//   HEAD.txt: "c3"
//   commits/c1/NextCommit.txt: "c2"
//   commits/c2/NextCommit.txt: "c3"
//   commits/c3/NextCommit.txt: "NA"
//   restore_state.txt:
//     CURRENT:c3
//     UNDO_SIZE:2
//     UNDO:c1
//     UNDO:c2

Repository repo("myproject");
CommitManager manager(&repo);
// Calls loadListFromDisk():
//   1. Reads TAIL.txt → "c1"
//   2. Loads c1 node
//   3. Reads c1/NextCommit.txt → "c2"
//   4. Loads c2 node, links c1 → c2
//   5. Reads c2/NextCommit.txt → "c3"
//   6. Loads c3 node, links c2 → c3
//   7. Sets head = c3, tail = c1

Restore restore(&repo);
// Calls loadStateFromDisk():
//   1. Reads restore_state.txt
//   2. Sets currentCommitID = "c3"
//   3. Rebuilds undoStack = [c1, c2]
//   4. Rebuilds redoStack = []

// Result: Full state restored!
// Linked List: [c1] → [c2] → [c3]
// Undo Stack: [c1, c2]
// Current: c3
// Redo Stack: []
```

---

## Common Pitfalls and Solutions

### Pitfall 1: Memory Leaks in Linked List

**Problem**: Forgetting to delete nodes in destructor

**Solution**: CommitManager destructor traverses and deletes all nodes
```cpp
~CommitManager() {
    CommitNode* curr = head;
    while (curr != nullptr) {
        CommitNode* next = curr->getNextNode();
        delete curr;  // Free memory
        curr = next;
    }
}
```

### Pitfall 2: Stack Underflow

**Problem**: Calling pop() on empty stack

**Solution**: Always check isEmpty() before pop()
```cpp
string pop() {
    if (isEmpty()) {
        throw out_of_range("Stack underflow");
    }
    // ... safe to pop
}
```

### Pitfall 3: Forgetting to Save State

**Problem**: Undo/redo state lost after program exit

**Solution**: 
- Save in destructor: `~Restore() { saveStateToDisk(); }`
- Save after each operation: `undo() { ... saveStateToDisk(); }`

### Pitfall 4: Not Clearing Redo on New Commit

**Problem**: Confusing state if redo stack not cleared

**Bad behavior**:
```
At c2, redo has [c3, c4]
Create c5
Redo → goes to c3??? (Wrong!)
```

**Solution**: `recordCommit()` clears entire redo stack
```cpp
void recordCommit(const string& commitID) {
    // ... push to undo
    
    while (!redoStack.isEmpty()) {  // Clear redo!
        redoStack.pop();
    }
    
    currentCommitID = commitID;
}
```

### Pitfall 5: File System Errors

**Problem**: Copying files might fail

**Solution**: Wrap in try-catch blocks
```cpp
void createCommitData(...) {
    try {
        filesystem::create_directories(...);
        // ... file operations
    } catch (filesystem::filesystem_error& e) {
        cerr << "Error: " << e.what() << endl;
        throw;
    }
}
```

---

## Testing Scenarios

### Test 1: Basic Commit Flow
```
1. Init repository
2. Add files
3. Create commit c1
4. Verify: HEAD = c1, TAIL = c1
5. Add more files
6. Create commit c2
7. Verify: HEAD = c2, c1 → c2
```

### Test 2: Undo/Redo Sequence
```
1. Create c1, c2, c3
2. Undo → should be at c2
3. Undo → should be at c1
4. Undo → should fail (no more undos)
5. Redo → should be at c2
6. Redo → should be at c3
7. Redo → should fail (no more redos)
```

### Test 3: Branch Creation
```
1. Create c1, c2, c3
2. Undo to c2
3. Create c4
4. Verify: redo stack empty
5. Verify: can undo to c2, c1
6. Verify: c3 still exists but unreachable
```

### Test 4: Revert Operation
```
1. Create c1 with fileA.txt
2. Create c2 with fileB.txt
3. Revert to c1
4. Verify: new commit c3 created
5. Verify: working dir has only fileA.txt
6. Verify: c3/Data/ identical to c1/Data/
```

### Test 5: Persistence
```
1. Create c1, c2, c3
2. Undo to c2
3. Exit program
4. Restart program
5. Verify: at c2
6. Verify: can undo to c1
7. Verify: can redo to c3
```

---

## Potential Enhancements

### Data Structure Improvements

**1. Hash Table for Commit Lookup**
```cpp
unordered_map<string, CommitNode*> commitMap;
// O(1) access by commit ID instead of O(n) traversal
```

**2. Skip List for Commit History**
```
[c1] ─────────→ [c5] ────→ [c9]   (level 2)
 ↓               ↓           ↓
[c1] ──→ [c3] ─→ [c5] ──→ [c7] ─→ [c9]  (level 1)
 ↓       ↓       ↓        ↓       ↓
[c1][c2][c3][c4][c5][c6][c7][c8][c9]    (level 0)

// Faster search: O(log n) instead of O(n)
```

**3. DAG for Branching Support**
```
       [c3]
      /
[c1]─[c2]
      \
       [c4]

// Support multiple branches like git
```

**4. B-Tree for Timestamp Indexing**
```
// Fast search by date range
commits between 2024-01-01 and 2024-01-31
```

### Algorithm Optimizations

**1. Delta Storage (Diff-based)**
```
c1: [full snapshot]
c2: [changes from c1]  ← saves space
c3: [changes from c2]
```

**2. Compression**
```cpp
void createCommitData() {
    // Compress files before saving
    compress(files);
    save_to_disk(compressed_files);
}
```

**3. Lazy Loading**
```cpp
class CommitNode {
    bool dataLoaded = false;
    
    void loadData() {
        if (!dataLoaded) {
            // Load files only when needed
            dataLoaded = true;
        }
    }
};
```

**4. Parallel File Operations**
```cpp
void copyFiles() {
    vector<thread> threads;
    for (auto& file : files) {
        threads.push_back(thread(copyFile, file));
    }
    // Copy multiple files simultaneously
}
```

### Feature Enhancements

**1. Branch Support**
```cpp
class Branch {
    string name;
    CommitNode* head;
};

map<string, Branch> branches;
```

**2. Merge Capability**
```cpp
void merge(Branch* source, Branch* target) {
    // Combine changes from two branches
}
```

**3. Diff Viewing**
```cpp
void showDiff(string commitID1, string commitID2) {
    // Show changes between two commits
}
```

**4. Selective Staging**
```cpp
void addPartial(string filename, vector<int> lineNumbers) {
    // Stage only specific lines of a file
}
```

---

## Conclusion

MiniVCS demonstrates effective use of fundamental data structures to solve real-world version control problems:

### Core Data Structures

**Singly Linked List (Commit History)**:
- Natural representation of linear commit timeline
- O(1) insertion for new commits at HEAD
- Simple forward traversal for log display
- Direct mapping to persistent disk structure
- Each node = one directory = one commit snapshot

**Dual Stack System (Undo/Redo)**:
- Intuitive LIFO semantics for navigation
- O(1) push/pop operations
- Automatic branch handling (clear redo on new commit)
- Independent navigation layer above storage
- Persistent state across program sessions

### Key Design Principles

1. **Separation of Concerns**: Each class has single responsibility
2. **Persistence Strategy**: Memory structures mirror disk organization
3. **Snapshot Model**: Complete state at each commit (simple, not space-efficient)
4. **Immutable History**: Commits never change once created
5. **Two-Level Architecture**: Storage (linked list) + Navigation (stacks)

