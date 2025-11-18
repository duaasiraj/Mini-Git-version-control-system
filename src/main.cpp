#include <iostream>
#include <vector>
#include <string>
#include "Repository.h"
#include "CommitManager.h"

using namespace std;

int main(int argc, char* argv[])
{
    Repository repo;
    CommitManager manager;

    if (argc < 2) {
        cout << "Usage: minigit <command> [args]\n";
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
        repo.clearStaging();
        cout << "Commit created.\n";
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
    // REVERT
    // =====================================
    if (cmd == "revert") {

        if (argc < 3) {
            cout << "Usage: minigit revert <commitID>\n";
            return 0;
        }

        string id = argv[2];
        manager.revert(id);
        return 0;
    }

    // =====================================
    // DEFAULT (unknown)
    // =====================================
    cout << "Unknown command: " << cmd << "\n";
    return 0;
}
