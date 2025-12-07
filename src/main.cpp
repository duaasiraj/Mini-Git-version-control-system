#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "Repository.h"
#include "CommitManager.h"
#include "Restore.h"

using namespace std;

int main(int argc, char* argv[])
{
    Repository repo;

    if (argc < 2) {
        cout << "Usage: minigit <command> [args]\n";
        cout << "Commands:\n";
        cout << "  init              - Initialize repository\n";
        cout << "  add <files>       - Add files to staging\n";
        cout << "  addall            - Add all files\n";
        cout << "  commit <message>  - Create a commit\n";
        cout << "  log               - Show commit history\n";
        cout << "  revert <commitID> - Revert to a commit (creates new commit)\n";
        cout << "  undo              - Undo to previous commit\n";
        cout << "  redo              - Redo to next commit\n";
        cout << "  status            - Show restore status\n";
        cout << "  history           - Show commit history with current position\n";
        return 0;
    }

    string cmd = argv[1];

    // =====================================
    // INIT
    // =====================================
    if (cmd == "init") {
        repo.init();
        cout << "Repository initialized.\n";
        return 0;
    }

    // Check if repository is initialized for all other commands
    if (!repo.isInitialized() && cmd != "init") {
        cerr << "fatal: not a Minivcs repository\n";
        cerr << "Hint: Use 'minigit init' to create a repository\n";
        return 1;
    }

    // Create manager and restore AFTER checking initialization
    CommitManager manager;
    Restore restore(&repo);

    // =====================================
    // ADD FILES
    // =====================================
    if (cmd == "add") {
        if (argc < 3) {
            cout << "Usage: minigit add <file1> <file2> ...\n";
            return 0;
        }

        vector<string> files;
        for (int i = 2; i < argc; i++)
            files.push_back(argv[i]);

        repo.add(files);
        return 0;
    }

    // =====================================
    // ADD ALL
    // =====================================
    if (cmd == "addall") {
        repo.addAll();
        return 0;
    }

    // =====================================
    // COMMIT
    // =====================================
    if (cmd == "commit") {
        if (argc < 3) {
            cout << "Usage: minigit commit <message>\n";
            return 0;
        }

        // Combine everything after "commit" into a message
        string msg;
        for (int i = 2; i < argc; i++) {
            msg += argv[i];
            if (i < argc - 1) msg += " ";
        }

        manager.addCommit(msg);

        // Get the new commit ID from HEAD.txt
        string newCommitID = repo.getHead();

        // Record the commit in restore system
        restore.recordCommit(newCommitID);

        repo.clearStaging();
        cout << "Commit created: " << newCommitID << "\n";
        return 0;
    }

    // =====================================
    // LOG
    // =====================================
    if (cmd == "log") {
        manager.printLog();
        return 0;
    }

    // =====================================
    // REVERT (creates a new commit with old data)
    // =====================================
    if (cmd == "revert") {
        if (argc < 3) {
            cout << "Usage: minigit revert <commitID>\n";
            return 0;
        }

        string id = argv[2];
        manager.revert(id);

        // Get the new revert commit ID
        string newCommitID = repo.getHead();

        // Record the new revert commit
        restore.recordCommit(newCommitID);

        return 0;
    }

    // =====================================
    // UNDO (checkout to previous commit)
    // =====================================
    if (cmd == "undo") {
        restore.undo();
        return 0;
    }

    // =====================================
    // REDO (checkout to next commit)
    // =====================================
    if (cmd == "redo") {
        restore.redo();
        return 0;
    }

    // =====================================
    // STATUS (Restore Status)
    // =====================================
    if (cmd == "status") {
        restore.printStatus();
        return 0;
    }

    // =====================================
    // HISTORY (Show commit history with current position)
    // =====================================
    if (cmd == "history") {
        restore.viewHistory(manager.getHead());
        return 0;
    }

    // =====================================
    // DEFAULT (unknown)
    // =====================================
    cout << "Unknown command: " << cmd << "\n";
    return 0;
}