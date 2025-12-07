#include "Restore.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;
//constructor for knowing what repo we are working on//
Restore::Restore(Repository* repository) : repo(repository), currentCommitID("NA") {
    if (repo && repo->isInitialized()) {
        loadStateFromDisk();

        // If no saved state exists, initialize from current HEAD
        if (currentCommitID == "NA") {
            string headCommit = repo->getHead();
            if (headCommit != "NA") {
                currentCommitID = headCommit;
                saveStateToDisk();
            }
        }
    }
}
//destructor, to save restore-able states to file
Restore::~Restore() {
    saveStateToDisk();
}

void Restore::recordCommit(const string& commitID) {
    // When a new commit is made, push current state to undo stack
    if (currentCommitID != "NA") {
        undoStack.push(currentCommitID);
    }

    // Clear redo stack since we're creating a new branch
    while (!redoStack.isEmpty()) {
        redoStack.pop();
    }
//set the current id and save everything to file
    currentCommitID = commitID;
    saveStateToDisk();
}

bool Restore::undo() {
    // Can't undo if there's nothing in the undo stack
    if (undoStack.isEmpty()) {
        cout << "Cannot undo! No previous commits available." << endl;
        return false;
    }

    string previousCurrent = currentCommitID;

    // Push current commit to redo stack
    redoStack.push(currentCommitID);

    // Pop the previous commit from undo stack
    currentCommitID = undoStack.pop();

    // Checkout to the previous commit (this updates working directory)
    repo->checkout(currentCommitID);

    // Save the new state immediately
    saveStateToDisk();

    return true;
}

bool Restore::redo() {

    // Can't redo if there's nothing in the redo stack
    if (redoStack.isEmpty()) {
        cout << "Cannot redo! No forward commits available." << endl;
        return false;
    }

    // Push current commit back to undo stack
    undoStack.push(currentCommitID);

    // Pop the next commit from redo stack
    currentCommitID = redoStack.pop();
    // Checkout to the next commit
    repo->checkout(currentCommitID);

    // Save the new state immediately
    saveStateToDisk();


    return true;
}

bool Restore::canUndo() const {
    return !undoStack.isEmpty();
}

bool Restore::canRedo() const {
    return !redoStack.isEmpty();
}

string Restore::getCurrentCommit() const {
    return currentCommitID;
}

string Restore::getUndoTarget() const {
    if (undoStack.isEmpty()) {
        return "NA";
    }
    return undoStack.peek();
}

string Restore::getRedoTarget() const {
    if (redoStack.isEmpty()) {
        return "NA";
    }
    return redoStack.peek();
}

int Restore::getUndoStackSize() const {
    return undoStack.size();
}

int Restore::getRedoStackSize() const {
    return redoStack.size();
}

void Restore::clear() {
    while (!undoStack.isEmpty()) {
        undoStack.pop();
    }
    while (!redoStack.isEmpty()) {
        redoStack.pop();
    }
    currentCommitID = "NA";
    saveStateToDisk();
}

void Restore::loadHistory(CommitNode* head) {
    if (!head) {
        return;
    }

    // Clear existing stacks
    clear();

    // Build undo stack from commit history (oldest to newest, excluding head)
    vector<string> commits;
    CommitNode* curr = head;

    // Collect all commits from head to tail
    while (curr) {
        commits.push_back(curr->getCommitID());
        curr = curr->getPrevNode();
    }

    // Push them in reverse order (oldest first) to undo stack
    for (int i = commits.size() - 1; i >= 1; i--) {
        undoStack.push(commits[i]);
    }

    // Set current to head (most recent commit)
    if (!commits.empty()) {
        currentCommitID = commits[0];
    }

    saveStateToDisk();
}

void Restore::printStatus() const {
    cout << "========================================\n"
         << "Restore Status:\n"
         << "========================================\n"
         << "Current: " << (currentCommitID == "NA" ? "None" : currentCommitID) << "\n"
         << "Undo Stack: " << getUndoStackSize() << " commits | Can Undo: " << (canUndo() ? "Yes" : "No") << "\n"
         << "Redo Stack: " << getRedoStackSize() << " commits | Can Redo: " << (canRedo() ? "Yes" : "No") << "\n";

    if (canUndo()) cout << "Next Undo → " << getUndoTarget() << "\n";
    if (canRedo()) cout << "Next Redo → " << getRedoTarget() << "\n";

    // Show actual stack contents for debugging
    if (undoStack.size() > 0) {
        cout << "\nUndo Stack (bottom → top):\n";
        for (int i = 0; i < undoStack.size(); i++) {
            cout << "  [" << i << "] " << undoStack.data[i] << "\n";
        }
    }
    if (redoStack.size() > 0) {
        cout << "\nRedo Stack (bottom → top):\n";
        for (int i = 0; i < redoStack.size(); i++) {
            cout << "  [" << i << "] " << redoStack.data[i] << "\n";
        }
    }

    cout << "========================================\n";
}

void Restore::viewHistory(CommitNode* head) const {
    if (!head) {
        cout << "No commits found.\n";
        return;
    }

    cout << "\n========== COMMIT HISTORY ==========\n";
    for (CommitNode* curr = head; curr; curr = curr->getPrevNode()) {
        cout << (curr->getCommitID() == currentCommitID ? " → [CURRENT] " : "             ")
             << "Commit: " << curr->getCommitID() << "\n";
    }
    cout << "====================================\n";
}

void Restore::saveStateToDisk() const {
    if (!repo || !repo->isInitialized()) {
        return;
    }

    fs::path restorePath = repo->getVcsRoot() / "restore_state.txt";

    try {
        ofstream file(restorePath);
        if (!file) {
            cerr << "Failed to save restore state to disk" << endl;
            return;
        }

        // Save current commit ID
        file << "CURRENT:" << currentCommitID << "\n";

        // Save undo stack (bottom to top)
        file << "UNDO_SIZE:" << undoStack.size() << "\n";
        for (int i = 0; i < undoStack.size(); i++) {
            file << "UNDO:" << undoStack.data[i] << "\n";
        }

        // Save redo stack (bottom to top)
        file << "REDO_SIZE:" << redoStack.size() << "\n";
        for (int i = 0; i < redoStack.size(); i++) {
            file << "REDO:" << redoStack.data[i] << "\n";
        }

        file.close();
    } catch (const exception& e) {
        cerr << "Error saving restore state: " << e.what() << endl;
    }
}

void Restore::loadStateFromDisk() {
    if (!repo || !repo->isInitialized()) {
        return;
    }

    fs::path restorePath = repo->getVcsRoot() / "restore_state.txt";

    if (!fs::exists(restorePath)) {
        // No saved state
        return;
    }

    try {
        ifstream file(restorePath);
        if (!file) {
            return;
        }

        string line;

        // Clear existing data
        while (!undoStack.isEmpty()) undoStack.pop();
        while (!redoStack.isEmpty()) redoStack.pop();

        while (getline(file, line)) {
            if (line.find("CURRENT:") == 0) {
                currentCommitID = line.substr(8);
            }
            else if (line.find("UNDO:") == 0) {
                undoStack.push(line.substr(5));
            }
            else if (line.find("REDO:") == 0) {
                redoStack.push(line.substr(5));
            }
        }

        file.close();
    } catch (const exception& e) {
        cerr << "Error loading restore state: " << e.what() << endl;
    }
}