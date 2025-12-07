#ifndef RESTORE_H
#define RESTORE_H

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "CommitManager.h"
#include "Repository.h"
#include "CommitNode.h"
#include <filesystem>

using namespace std;

class Stack {
public:
    vector<string> data;
    int  top;

    Stack() :  top(-1) {}

    bool isEmpty() const {
        return  top < 0;
    }

    int size() const {
        return  top + 1;
    }

    void push(const string &value) {
        data.push_back(value);
        ++ top;
    }

    string pop() {
        if (isEmpty()) {
            throw out_of_range("Stack underflow");
        }
        string value = data.back();
        data.pop_back();
        -- top;
        return value;
    }

    const string& peek() const {
        if (isEmpty()) {
            throw out_of_range("Stack is empty");
        }
        return data[ top];
    }
};

class Restore {
private:
    Stack undoStack;
    Stack redoStack;
    Repository* repo;
    string currentCommitID;

public:
    Restore(Repository* repository);
    ~Restore();

    void recordCommit(const string& commitID);
    bool undo();
    bool redo();
    bool canUndo() const;
    bool canRedo() const;
    string getCurrentCommit() const;
    string getUndoTarget() const;
    string getRedoTarget() const;
    void clear();
    void loadHistory(CommitNode* head);
    int getUndoStackSize() const;
    int getRedoStackSize() const;
    void printStatus() const;
    void viewHistory(CommitNode* head) const;
    void saveStateToDisk() const;
    void loadStateFromDisk();
};


#endif // RESTORE_H