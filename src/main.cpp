#include "Repository.h"
#include "CommitNode.h"
#include "HashingHelper.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

string readFile(const string& filename) {
    ifstream file(filename);
    if (!file) return "[FILE NOT FOUND]";

    string content, line;
    while (getline(file, line)) {
        content += line + "\n";
    }
    return content;
}

void printFileContent(const string& filename) {
    cout << YEL << "📄 Content of " << filename << ":" << END << "\n";
    cout << WHT << readFile(filename) << END << "\n";
}

void printSeparator(const string& title) {
    cout << "\n" << BLU << "============================================" << END << "\n";
    cout << CYN << "  " << title << END << "\n";
    cout << BLU << "============================================" << END << "\n";
}

int main() {
    Repository repo;

    printSeparator(" STEP 1: Initialize Repository");
    repo.init();

    // ========================================
    // FIRST VERSION OF test.txt
    // ========================================
    printSeparator(" STEP 2: Create test.txt (Version 1)");

    ofstream file1("test.txt");
    file1 << "This is VERSION 1 of test.txt\n";
    file1 << "Initial content\n";
    file1.close();

    printFileContent("test.txt");

    printSeparator(" STEP 3: Add and Commit Version 1");
    repo.add({"test.txt"});

    string commitMsg1 = "First commit: added test.txt v1";
    string commitID1 = generateCommitID(commitMsg1);
    CommitNode* commit1 = new CommitNode(commitID1, commitMsg1);
    repo.setHead(commitID1);
    repo.clearStaging();

    cout << GRN << " Commit 1 created: " << commitID1 << END << "\n";
    cout << WHT << "   Message: " << commitMsg1 << END << "\n";
    cout << WHT << "   Working directory has: VERSION 1" << END << "\n";

    // ========================================
    // SECOND VERSION OF test.txt
    // ========================================
    printSeparator(" STEP 4: Modify test.txt (Version 2)");

    ofstream file2("test.txt");
    file2 << "This is VERSION 2 of test.txt\n";
    file2 << "Content has been UPDATED!\n";
    file2 << "New line added\n";
    file2.close();

    printFileContent("test.txt");

    printSeparator(" STEP 5: Add and Commit Version 2");
    repo.add({"test.txt"});

    string commitMsg2 = "Second commit: updated test.txt to v2";
    string commitID2 = generateCommitID(commitMsg2);
    CommitNode* commit2 = new CommitNode(commitID2, commitMsg2);

    commit1->saveNextID(commitID2);
    commit2->setNextNode(commit1);
    repo.setHead(commitID2);
    repo.clearStaging();

    cout << GRN << " Commit 2 created: " << commitID2 << END << "\n";
    cout << WHT << "   Message: " << commitMsg2 << END << "\n";
    cout << WHT << "   Working directory now has: VERSION 2" << END << "\n";

    // ========================================
    // THIRD VERSION OF test.txt
    // ========================================
    printSeparator(" STEP 6: Modify test.txt AGAIN (Version 3)");

    ofstream file3("test.txt");
    file3 << "This is VERSION 3 of test.txt\n";
    file3 << "Third iteration!\n";
    file3 << "Even more changes\n";
    file3 << "Latest version!\n";
    file3.close();

    printFileContent("test.txt");

    printSeparator(" STEP 7: Add and Commit Version 3");
    repo.add({"test.txt"});

    string commitMsg3 = "Third commit: updated test.txt to v3";
    string commitID3 = generateCommitID(commitMsg3);
    CommitNode* commit3 = new CommitNode(commitID3, commitMsg3);

    commit2->saveNextID(commitID3);
    commit3->setNextNode(commit2);
    repo.setHead(commitID3);
    repo.clearStaging();

    cout << GRN << " Commit 3 created: " << commitID3 << END << "\n";
    cout << WHT << "   Message: " << commitMsg3 << END << "\n";
    cout << WHT << "   Working directory now has: VERSION 3 (LATEST)" << END << "\n";

    // ========================================
    // VERIFY COMMITS ARE SEPARATE
    // ========================================
    printSeparator(" STEP 8: Verify All Commits Are Stored Separately");

    cout << MAG << " Commit 1 (" << commitID1 << "):" << END << "\n";
    cout << WHT << "   Location: .Minivcs/commits/" << commitID1 << "/Data/test.txt" << END << "\n";
    string commit1Content = readFile(".Minivcs/commits/" + commitID1 + "/Data/test.txt");
    cout << "   Content: " << commit1Content;

    cout << MAG << "\n Commit 2 (" << commitID2 << "):" << END << "\n";
    cout << WHT << "   Location: .Minivcs/commits/" << commitID2 << "/Data/test.txt" << END << "\n";
    string commit2Content = readFile(".Minivcs/commits/" + commitID2 + "/Data/test.txt");
    cout << "   Content: " << commit2Content;

    cout << MAG << "\n Commit 3 (" << commitID3 << "):" << END << "\n";
    cout << WHT << "   Location: .Minivcs/commits/" << commitID3 << "/Data/test.txt" << END << "\n";
    string commit3Content = readFile(".Minivcs/commits/" + commitID3 + "/Data/test.txt");
    cout << "   Content: " << commit3Content;

    cout << "\n" << GRN << " All three versions are stored separately in .Minivcs!" << END << "\n";

    // ========================================
    // SHOW CURRENT WORKING FILE
    // ========================================
    printSeparator(" STEP 9: Current Working Directory (Like Git)");

    cout << "Current HEAD: " << repo.getHead() << "\n\n";
    printFileContent("test.txt");

    cout << GRN << " Working directory has the LATEST committed version (v3)" << END << "\n";
    cout << WHT << "   Just like in Git!" << END << "\n";

    // ========================================
    // CHECKOUT OLD VERSION
    // ========================================
    printSeparator(" STEP 10: Checkout Old Version (Commit 1)");

    cout << YEL << "Checking out commit: " << commitID1 << END << "\n";
    repo.checkout(commitID1);

    cout << "\n";
    printFileContent("test.txt");

    cout << GRN << " Working directory now shows VERSION 1!" << END << "\n";
    cout << WHT << "   (Commits 2 and 3 still preserved in .Minivcs/)" << END << "\n";

    // ========================================
    // CHECKOUT VERSION 2
    // ========================================
    printSeparator(" STEP 11: Checkout Version 2");

    cout << YEL << "Checking out commit: " << commitID2 << END << "\n";
    repo.checkout(commitID2);

    cout << "\n";
    printFileContent("test.txt");

    cout << GRN << " Working directory now shows VERSION 2!" << END << "\n";

    // ========================================
    // BACK TO LATEST
    // ========================================
    printSeparator(" STEP 12: Back to Latest Version");

    cout << YEL << "Checking out commit: " << commitID3 << END << "\n";
    repo.checkout(commitID3);

    cout << "\n";
    printFileContent("test.txt");

    cout << GRN << " Working directory restored to VERSION 3 (latest)!" << END << "\n";

    // ========================================
    // DEMONSTRATE GIT-LIKE BEHAVIOR
    // ========================================
    printSeparator(" STEP 13: Git-Like Behavior Demo");

    cout << WHT << "Let's make a NEW change (Version 4) while HEAD is at v3:\n" << END;

    ofstream file4("test.txt");
    file4 << "This is VERSION 4 of test.txt\n";
    file4 << "Fourth iteration!\n";
    file4 << "Building on v3\n";
    file4 << "New features added!\n";
    file4.close();

    cout << "\n";
    printFileContent("test.txt");

    repo.add({"test.txt"});
    string commitMsg4 = "Fourth commit: updated test.txt to v4";
    string commitID4 = generateCommitID(commitMsg4);
    CommitNode* commit4 = new CommitNode(commitID4, commitMsg4);

    commit3->saveNextID(commitID4);
    commit4->setNextNode(commit3);
    repo.setHead(commitID4);
    repo.clearStaging();

    cout << GRN << " Commit 4 created: " << commitID4 << END << "\n";
    cout << WHT << "   Working directory automatically has VERSION 4!" << END << "\n";

    // ========================================
    // SUMMARY
    // ========================================
    printSeparator("📊 FINAL SUMMARY");

    cout << "Repository Structure (Git-Like!):\n\n";
    cout << "working_directory/\n";
    cout << "├── test.txt ← Currently: VERSION 4 (latest commit)\n";
    cout << "│   (This file changes when you checkout different commits)\n";
    cout << "│\n";
    cout << "└── .Minivcs/\n";
    cout << "    └── commits/\n";
    cout << "        ├── " << commitID1 << "/\n";
    cout << "        │   └── Data/test.txt (VERSION 1)  Preserved!\n";
    cout << "        ├── " << commitID2 << "/\n";
    cout << "        │   └── Data/test.txt (VERSION 2)  Preserved!\n";
    cout << "        ├── " << commitID3 << "/\n";
    cout << "        │   └── Data/test.txt (VERSION 3)  Preserved!\n";
    cout << "        └── " << commitID4 << "/\n";
    cout << "            └── Data/test.txt (VERSION 4)  Preserved!\n";

    cout << "\n" << GRN << " Perfect! This works just like Git:" << END << "\n";
    cout << WHT << "    All versions stored in .Minivcs/commits/" << END << "\n";
    cout << WHT << "    Working directory has latest committed version" << END << "\n";
    cout << WHT << "    Can checkout any previous version anytime" << END << "\n";
    cout << WHT << "    Checking out updates the working directory file" << END << "\n";

    // Cleanup
    delete commit1;
    delete commit2;
    delete commit3;
    delete commit4;

    return 0;
}