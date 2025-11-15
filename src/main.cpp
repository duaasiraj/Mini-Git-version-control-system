#include <iostream>
#include <filesystem>
#include "CommitNode.h"
#include "HashingHelper.h"
#include <fstream>

using namespace std;

int main() {

    filesystem::create_directories(".Minivcs/commits");
    filesystem::create_directories(".Minivcs/staging_area");


    cout << "=== Testing CommitNode ===\n";

    // --- Create an initial commit ---

    CommitNode c1(HASHINGHELPER_H::generateCommitID(), "Initial commit");
    cout << "Created commit: " << c1.getCommitID()
         << " message: " << c1.getCommitMsg() << endl;

    // --- Simulate adding a file into Data of commit 1 ---
    filesystem::create_directories(".Minivcs/commits/"+c1.getCommitID()+"/Data");
    ofstream f(".Minivcs/commits/"+c1.getCommitID()+"/Data/test.txt");
    f << "Hello from commit c1";
    f.close();


    CommitNode c2(HASHINGHELPER_H::generateCommitID(), "Initial commit");
    cout << "Created commit: " << c2.getCommitID()
         << " message: " << c2.getCommitMsg() << endl;

    // --- Simulate adding a file into Data of commit 2 ---
    filesystem::create_directories(".Minivcs/commits/"+c2.getCommitID()+"/Data");
    ofstream f1(".Minivcs/commits/"+c2.getCommitID()+"/Data/test.txt");
    f1 << "Hello from commit c2";
    f1.close();

    CommitNode c3(HASHINGHELPER_H::generateCommitID());
    c3.revertCommitData(c2.getCommitID());


    return 0;
}
