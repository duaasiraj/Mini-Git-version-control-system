

# CODE DOCUMENTATION

# COMMITNODE

<br>

## Functions

<br>

```
#ifndef COMMITNODE_H
#define COMMITNODE_H

#include <string>
#include <filesystem>
using namespace std;

class Repository; // Forward declaration

class CommitNode {
private:
string commitID;
string commitMsg;
string nextCommitID;
CommitNode* nextNode;
filesystem::path commitsBasePath; // Store the repo's commits path

public:
CommitNode();
CommitNode(const  string& cI, const  string& cM, Repository& repo);
CommitNode(const  string& cI, Repository& repo);

    void createCommitData(const  filesystem::path& stagingPath);
    void revertCommitData(const  string& id);
    void loadNodeInfo();
     string loadNextCommitID();

    // Setters
    void setCommitID(const  string& i);
    void setCommitMsg(const  string& m);
    void setNextID(const  string& n);
    void setNextNode(CommitNode* n);

    // Getters
     string getCommitID();
     string getCommitMsg();
     string getNextID();
    CommitNode* getNextNode();

    void saveNextID(const  string& id);
};

#endif
```

---

### CommitNode()

-> Initialises commit node setting next node to null

---

### CommitNode(const string& cI, const string& cM, Repository& repo)

-> Initialises CommitID, message , next node, gets the base where commits are to be stores (inside a specific repo)
-> Create commit explained later

---

### void CommitNode::createCommitData(const filesystem::path& stagingPath)

-> Parameter: it takes staging path so it knows kahan store karni hai
-> it creates a directory inside the path we set with a commit id
-> creates a subfolder data
->creates info.txt where it writes commit id, commit message and date and time of commit
-> inside commitid directory files will be copied from the staging area to data folder, which means if it is a folder it will iteratively copy and if it is just a file then it will calculate the relative path

**Example:**
entry: Staging/utils/helper.cpp
stagingPath: Staging/

so on running
`auto relative = filesystem::relative(entry.path(), stagingPath);`

we will have now:
`utils/helper.cpp`

and then it will set destination path
`auto dest = dataPath / relative;`

`C:/myrepo/Commits/c1/Data/utils/helper.cpp`

-> it also checks if it was a directory/ folder so the same files can be created in destination , otherwise files can be overwritten

---

### void CommitNode::revertCommitData(const string& id)

-> create data folder for current commit
-> create info.txt for the new commit
-> open info file for writing and write basic info in it
-> find old path u wanna copy `filesystem::path OldPath = commitsBasePath / id / "Data";`
-> copy all files :

```
for (auto &entry : filesystem::recursive_directory_iterator(OldPath)) {
auto relative = filesystem::relative(entry.path(), OldPath);
```

**Example:**
entry.path() = `Commits/c2/Data/src/utils.cpp`
OldPath = `Commits/c2/Data`
relative becomes:
`src/utils.cpp`

Destination path:

`auto dest = newDataPath / relative;`

Example:
`Commits/c4/Data/src/utils.cpp`

---

### void CommitNode::loadNodeInfo()

-> create path so we can access info.txt
-> if it does, open it and read it

---

### string CommitNode::loadNextCommitID()

-> NextCommit.txt exists Returns the next commit ID from the file
->NextCommit.txt does NOT exist Returns "NA"

---

### void CommitNode::saveNextID(const string& id)

Save the next commit ID Writes the given ID into NextCommit.txt

---

The rest are just getters and setters

---

# REPOSITORY

## Functions

```
#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <string>
#include <vector>
#include <filesystem>
using namespace std;

// Color codes for terminal output
#define RED "\033[31m"
#define GRN "\033[32m"
#define YEL "\033[33m"
#define END "\033[0m"

class Repository {
private:
filesystem::path repoPath;      // .Minivcs/repo
filesystem::path stagingPath; // .Minivcs/repo/staging_area
filesystem::path commitsPath;   // .Minivcs/repo/commits
string repoName;

public:
Repository(const   string& name = "repo");

    void initRepo();
    bool isRepoInitialized() const;

    void add(const   vector<  string>& files);
    void addAll();

    void clearStagingArea();
      vector<  string> getTrackedFiles() const;
    bool isStagingEmpty() const;

    // Getters
      filesystem::path getRepoPath() const;
      filesystem::path getStagingPath() const;
      filesystem::path getCommitsPath() const;
      string getRepoName() const;

private:
void addSingleFile(const   string& filename);
};

#endif
```

---

## REPOSITORY

<br>

### Functions

<br>

---

### Repository::Repository(const string& name)

-> Constructor
-> Initializes repository name
-> Sets repo path to:
`.Minivcs/<repoName>` inside the current working directory

-> Also sets:

* `stagingPath = repoPath / "staging_area"`
* `commitsPath = repoPath / "commits"`

No folders are created here — only paths are prepared.

---

### void Repository::initRepo()

-> Checks if the repository already exists using `isRepoInitialized()`
-> If already exists → prints message and returns

-> Otherwise:

* Creates directory structure:

  ```
  .Minivcs/<repoName>/
      staging_area/
      commits/
  ```
* Creates `HEAD.txt` in repo root
* Writes `"NA"` inside `HEAD.txt` indicating no commits yet

-> Prints success message
-> Wrapped in `try-catch` to handle filesystem errors

---

### bool Repository::isRepoInitialized() const

-> Verifies repository is properly set up
-> Checks:

* `.Minivcs/<repoName>` exists
* `staging_area/` exists
* `commits/` exists

-> If all exist → returns **true**, else **false**

---

### void Repository::add(const vector<string>& files)

-> Equivalent to git’s:

```
git add file1 file2 file3
```

-> Accepts multiple filenames in a vector

Process:

1. Check if repo is initialized
2. If vector is empty → show message and return
3. For each filename:

   * Calls `addSingleFile(filename)`
   * Counts how many succeeded / failed
4. Prints final summary

   * Number of files added
   * Number of files that failed

---

### void Repository::addSingleFile(const string& filename)

-> Helper function for `add()`
-> Adds **one file or directory** to the staging area

Steps:

1. Converts given filename into full path (`sourcePath = current_path/filename`)
2. Checks if file exists
3. Prevents adding the internal `.Minivcs` folder
4. Uses `copy_options::recursive` for directories
5. Behavior:

#### If it is a directory

* Copies entire folder into staging area
* Example:

  ```
  src/ → staging_area/src/
  ```

#### If it is a file

* Computes relative path from project root
* Ensures parent directories exist inside staging
* Copies file with overwrite allowed

---

### void Repository::addAll()

-> Equivalent to:

```
git add .
```

-> Adds **all files and folders** in current directory EXCEPT:

* `.Minivcs/`
* `.git/`

Process:

1. Iterate over everything in `current_path()`
2. Skip `.Minivcs` and `.git`
3. For folders → copy recursively
4. For files → copy individually
5. Prints how many files were added

Useful when staging many files at once.

---

### void Repository::clearStagingArea()

-> Removes everything from `staging_area/`
-> Used after committing files

Steps:

1. Check if repo is initialized
2. Iterate through staging directory
3. Call `remove_all()` on each entry
4. Wrapped in `try-catch` for safety

After this, staging becomes empty.

---

### vector<string> Repository::getTrackedFiles() const

-> Returns **a list of all files currently in staging**

Process:

1. Check repo/staging exist
2. Traverse staging area with `recursive_directory_iterator`
3. Collect only regular files (not folders)
4. Convert to relative paths for readability

Example return:

```
[ "src/main.cpp", "include/utils.h" ]
```

Useful for displaying what is staged before committing.

---

### bool Repository::isStagingEmpty() const

-> Returns true if:

* repo not initialized, OR
* staging folder does not exist, OR
* staging folder contains nothing

-> Uses `filesystem::is_empty()`

---

### filesystem::path Repository::getRepoPath() const

Returns base repo path.

### filesystem::path Repository::getStagingPath() const

Returns staging folder path.

### filesystem::path Repository::getCommitsPath() const

Returns commits folder path.

### string Repository::getRepoName() const

Returns name of the repository.

---





