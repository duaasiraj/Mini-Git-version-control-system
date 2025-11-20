#include "CommitManager.h"
#include "HashingHelper.h"
#include "HashingHelper.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

/* HELPER FUNCTION TO READ FIRST LINE FROM FILE
Made cause baar baar koi file parhni par rahi thi to get the ID
*/
//----------------------------------------------------------------------------------------------------------------------------

static string readFile(const filesystem::path& path) {
    if (!filesystem::exists(path)){
        return "NA";
    }

    ifstream file(path);

    string s;
    getline(file, s); //IDs jo baar baar parhni par rahi thein
    return s;
}

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

/* CONSTRUCTOR

    -Sets the head (latest commit) and tail (oldest commit) to NULL.
    -Initializes a path variable that contains the path to the commits folder (current_directory/.Minivcs/commits)
    -Checks if the path exists. If it doesn't, it returns, else, it calls loadListFromDisk() function.

*/
//----------------------------------------------------------------------------------------------------------------------------

CommitManager::CommitManager() {
    head = nullptr;
    tail = nullptr;

    filesystem::path VCSRepo = filesystem::current_path() / ".Minivcs" / "commits";
    if (!filesystem::exists(VCSRepo)) {
        return;
    }

    loadListFromDisk();
}

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

/* SET UP THE LINKED LIST OF COMMITS

    Structure of List of commits

               NULL                     ↑  => Next commit       ↓  => Previous commit
                ↑
        HEAD (Latest commit)
                ↑↓
            <Commit ID>
                ↑↓
        TAIL (oldest commit)
                 ↓
                NULL

We started creating our linked list from tail to head simply due to the convenience of having the nextNode of the TAIL commit not be NULL
There is no issue in starting from the HEAD commit, but changes would be made to the direction of nodes we pick, i.e, we'll traverse using prev nodes

commitPaths refers to the path "current_directory/.Minivcs/commits" where all the commits are saved, along with HEAD.txt and TAIL.txt

As the name suggests, HEAD.txt holds the commitID of the head node and TAIL.txt holds the tail ID

If there is no head or tail yet, both files will contain "NA"

After loading up the head and tail pointer, we traverse the list and set the next and prev pointers accordingly.

*/
//----------------------------------------------------------------------------------------------------------------------------

void CommitManager::loadListFromDisk() {

    filesystem::path commitsPath = filesystem::current_path() / ".Minivcs" / "commits";

    string headID = readFile(commitsPath / "HEAD.txt");
    string tailID = readFile(commitsPath / "TAIL.txt");

    if (headID == "NA" || tailID == "NA") {
        return;
    }

    head = loadSingleNode(headID);
    tail = loadSingleNode(tailID);

    CommitNode* current = tail;

    while (current && current->getNextID() != "NA") {
        CommitNode* next = loadSingleNode(current->getNextID());
        current->setNextNode(next);
        next->setPrevNode(current);
        current = next;
    }

    head = current;
}

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

/* FUNCTION TO LOAD UP A SINGLE NODE

We could do this part within LoadListFfromDisk but for the sake of readability and cleaner code we've seperated it.
Taking an ID, it calls the commitNode's constructor that handles loading up one specific node from the info.txt file.
If it works, we send it back up to the caller function e.g, LoadListffromDisj
else, the catch block returns a NULL pointer to signify said Node does not exist

*/

//----------------------------------------------------------------------------------------------------------------------------

CommitNode* CommitManager::loadSingleNode(const string& id) {
    try {
        return new CommitNode(id);
    }
    catch (...) {
        return nullptr;
    }
}


//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

/* FUNCTION TO ADD A NEW COMMIT TO THE LIST

This function takes the files that have been added to the staging area, and moves them over to a commit.
The relevant commit folder will be made by the CommitNode class constructor that takes the ID + msg as parameter.

- The function calls the hashinghelper class to create a random unique ID for the commit.
- A new node is created by calling CommitNode(Commit ID, Commit message)
- The function checks if the head is NULL or not
    -if the head is null, that means that this is the first commit to be made.
        -in which case, our new node is both the head and the tail for our list.
        - we open up two files: HEAD.txt and TAIL.txt, to store the ID of the node
        -After writing the ID to both files, the function returns to caller

    -if the head is not null, it means that this is NOT the first commit, and as such, we will now have a new head
        -first we need to create the forward link between head and new node
            -the first thing we do is to set head->next = new commit.
            -then, we set the nextID parameter to that too.
            -then, we save the nextID inside head's NextCommit.txt file
        -then, we create the backward link betweem head and the new node.
            -new node's previous node is set to head.
            -new node's previous node id is set to head id
            -new node's previous node id is saved in previousCommit.txt
        -now that the connection is done, we'll set the new node asour head (latest commit)
        -we open up HEAD.txt, and save the now current head's id to the file.

*/

//----------------------------------------------------------------------------------------------------------------------------

void CommitManager::addCommit(const string& msg) {

    string id = HASHINGHELPER_H::generateCommitID();
    CommitNode* newNode = new CommitNode(id, msg);

    if (head == nullptr) {
        // first commit in repo
        head = tail = newNode;

        ofstream f1(filesystem::current_path() / ".Minivcs" / "commits" / "HEAD.txt");
        ofstream f2(filesystem::current_path() / ".Minivcs" / "commits" / "TAIL.txt");
        f1 << id;
        f2 << id;
        return;
    }

    head->setNextNode(newNode);
    head->setNextID(newNode->getCommitID());
    head->saveNextID(newNode->getCommitID());

    newNode->setPrevNode(head);
    newNode->setPrevID(head->getCommitID());
    newNode->savePrevID(head->getCommitID());

    head = newNode;

    ofstream Hfile(filesystem::current_path() / ".Minivcs" / "commits" / "HEAD.txt");
    Hfile << head->getCommitID();
}

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

/* FUNCTION TO REVERT TO A SPECIFIC COMMIT
This function creates a new commit that holds a previous commit's data. it also clears the current working directory and replaces the contents with the data from the previous commit
This function works in two main parts
Before that, we make sure the path to the given source commit actually exists
We check that current_directory/.Minivcs/commits/<source commit ID> exists

    PART 1 => Copying source commit's data to the staging area

        -Source COmmit path: current_directory/.Minivcs/commits/<source commit ID>
        -Staging area path: current_directory/.Minivcs/commits/staging_area

        -We remove all existing files and directories inside staging area

        -then we use filesystem's recurive directory iterator to iterate through everything in the src commmit/Data path. All these directories are labelled as "entry"
            -we first determine the path of each directory and file relative to the src commit path.
                -this is because we want to recreate the same file structure inside the staging_area
            -to get relative path
                relativePath = filesystem::relative(entry.path(), srcCommit)
                example,
                    entry.path() = My_Current_Directory/.Minivcs/commits/srcCommitID/Data/FolderA/FileA.txt
                    relativePath = FolderA/FileA.txt

                    so now inside staging area we can do

                    staging_area/FolderA/FileA.txt

            -we check if the current relative path is a directory or file
                -in case of a directory, we create a new directory inside staging area with the same name/structure

                -in case of a file, we first ensure the parent path/directories exist. we also ensure that incase the file already exists there for whatever reason, we remove it
                -then we copy the file from the source commit to the staging area


    PART 2 => Copying staging area data to a new commit + working directory

        -Copying staging area data to a new commit
            -We create a new commit with a revert message
            -the node becomes the new head
            -we get the head node's ID from HEAD.txt
            -we create two paths, one for the new commit, and the other for the current working directory
            -we ensure two things:
                1. We don't delete .Minivcs. To do this is the path's eventual file is compared to ".Minivcs"
                2. We remove all files in the working directory that don't exist in the NewDataPath
            -we then copy all files/directories from the new data path to the working directory, same way we did for the above function
*/

//----------------------------------------------------------------------------------------------------------------------------

void CommitManager::revert(const string& commitID) {

    // ----------------------------------------- PART 1 -----------------------------------------

    filesystem::path commitPath = filesystem::current_path() / ".Minivcs" / "commits" / commitID;

    if (!filesystem::exists(commitPath)) {
        cout << "Commit not found.\n";
        return;
    }

    filesystem::path srcCommit = commitPath / "Data";
    filesystem::path stagingPath = filesystem::current_path() / ".Minivcs" / "staging_area";


    for (auto& entry : filesystem::directory_iterator(stagingPath)) {
        filesystem::remove_all(entry);
    }


    for (auto& entry : filesystem::recursive_directory_iterator(srcCommit)) {

       filesystem::path relativePath = filesystem::relative(entry.path(), srcCommit);

        if (filesystem::is_directory(entry)) {
            filesystem::create_directories(stagingPath/relativePath);
        } else {
            filesystem::create_directories((stagingPath/relativePath).parent_path());
            if (filesystem::exists(stagingPath/relativePath)) {
                filesystem::remove(stagingPath/relativePath);
            }
            filesystem::copy_file(entry.path(), stagingPath/relativePath);
        }
    }
    // ----------------------------------------- PART 2 -----------------------------------------

    addCommit("Revert to " + commitID);

    string newID;

    ifstream h(filesystem::current_path() / ".Minivcs" / "commits" / "HEAD.txt");
    getline(h, newID);


    filesystem::path newDataPath = filesystem::current_path() / ".Minivcs" / "commits" / newID / "Data";
    filesystem::path workingDir = filesystem::current_path();

    for (auto& entry : filesystem::directory_iterator(workingDir)) {
        string name = entry.path().filename().string();

        if (name == ".Minivcs") {
            continue;
        }

        filesystem::path equivalent = newDataPath / name;
        if (!filesystem::exists(equivalent)) {
            filesystem::remove_all(entry);
        }
    }



    for (auto& entry : filesystem::recursive_directory_iterator(newDataPath)) {

        filesystem::path relativePath = filesystem::relative(entry.path(), newDataPath);

        if (filesystem::is_directory(entry)) {
            filesystem::create_directories(workingDir / relativePath);
        } else {
            filesystem::create_directories((workingDir / relativePath).parent_path());

            if (filesystem::exists(workingDir / relativePath)) {
                filesystem::remove(workingDir / relativePath);
            }
            filesystem::copy_file(entry.path(), workingDir / relativePath);
        }
    }

    cout << "Revert complete. Created commit: " << newID << "\n";
    h.close();
}

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

/* FUNCTION TO PRINT A LOG OF ALL COMMITS DONE SO FAR

Traversal from HEAD (latest commit) to TAIL(oldest commit).
We use previous nodes for traversal since head will not have any next nodes.
We open the info.txt file to extract the timestamp
once we're on the correct line, we take the substring after the DATE AND TIME heading
we then print the ID + MEssage + timestamp
we then move to the previous node
*/

//----------------------------------------------------------------------------------------------------------------------------

void CommitManager::printLog(){

    if (!head) {
        cout << "No commits found." << endl;
        return;
    }

    CommitNode* curr = head;

    while (curr) {
        filesystem::path infoPath = filesystem::current_path()/
                            ".Minivcs"/"commits"/curr->getCommitID()/"info.txt";

        ifstream f(infoPath.string());
        string line;
        string timestamp;

        while (getline(f, line)) {
            if (line.find("3. DATE & TIME OF COMMIT: ") == 0)
                timestamp = line.substr(strlen("3. DATE & TIME OF COMMIT: "));
        }

        cout << "Commit: " << curr->getCommitID() << endl;
        cout << "Message: " << curr->getCommitMsg() << endl;
        cout << "Date: " << timestamp << endl;
        cout << "------------------------------------" << endl;

        curr = curr->getPrevNode();
    }
}

//----------------------------------------------------------------------------------------------------------------------------

CommitNode* CommitManager::getHead() {
    return head;
}

CommitNode* CommitManager::getTail() {
    return tail;
}

CommitManager::~CommitManager() {
    CommitNode* curr = head;

    while (curr != nullptr) {
        CommitNode* prev = curr->getPrevNode();
        delete curr;
        curr = prev;
    }

    head = nullptr;
    tail = nullptr;
}



