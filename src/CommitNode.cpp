#include "HashingHelper.h"
#include "CommitNode.h"
#include <fstream>
#include <filesystem>
#include <ctime>
#include <iostream>
#include <cstring>
using namespace std;

CommitNode::CommitNode() {

    this->nextNode = NULL;
    prevNode = NULL;
    nextCommitID = "NA";
    prevCommitID = "NA";

}

CommitNode::CommitNode(string cI, string cM) {

    commitID = cI;
    commitMsg = cM;
    nextNode = NULL;
    prevNode = NULL;


    createCommitData();

}

CommitNode::CommitNode(string cI) {

    commitID = cI;
    nextNode = NULL;
    prevNode = NULL;

    loadNodeInfo();

}

/*
commits follow the following structure

CUrrent Directory
|-> .Minivcs
|   |
|   |->commits (where all commits are stored)
|   |    |-> HEAD.txt  => holds ID of head commit (latest commit)
|   |    |-> TAIL.txt  => holds ID of tail commit (first commit)
|   |    |-> <Commit ID> (folders created for each commit)
|   |    |      |
|   |    |      |->info.txt  => holds commit ID, commit Message, timestamp
|   |    |      |->NextCommit.txt   => holds the next commit's ID
|   |    |      |->PrevCommit.txt   => holds the next commit's ID
|   |    |      |->Data    => this is where all the files will get stored from the staging area after calling ("commit")
|   |
|   |->staging area (where files get added upon "add" command)
|
|->any files in the project/directory/whatever it is we wanna put version control
*/
void CommitNode::createCommitData() {

    try {
        filesystem::create_directories(filesystem::current_path()/".Minivcs"/"commits"/commitID);
        filesystem::create_directories(filesystem::current_path()/".Minivcs"/"commits"/commitID/"Data");

        filesystem::path currNodePath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"info.txt";

        ofstream infoFile(currNodePath.string());

        if (!infoFile) {
            throw runtime_error("Could not save info to info.txt");
        }

        time_t timestamp; //this is just to get the current time
        time(&timestamp);
        string ts = ctime(&timestamp);



        infoFile<<"1. COMMIT ID: " + commitID + "\n2. COMMIT MESSAGE: " + commitMsg + "\n3. DATE & TIME OF COMMIT: " + ts + "\n";

        infoFile.close();

        filesystem::path dataPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"Data";
        filesystem::path staging = filesystem::current_path()/".Minivcs"/"staging_area";


        for (auto &entry : filesystem::recursive_directory_iterator(staging)) {
            auto relative = filesystem::relative(entry.path(), staging);
            auto dest = dataPath / relative;

            if (filesystem::is_directory(entry)) {
                filesystem::create_directories(dest);
            } else {
                filesystem::copy_file(entry, dest, filesystem::copy_options::overwrite_existing);
            }
        }

        // create NextCommit.txt and PrevCommit.txt with "NA"
        filesystem::path nextPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"NextCommit.txt";
        filesystem::path prevPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"PrevCommit.txt";

            ofstream n(nextPath.string());
            n << "NA";


            ofstream p(prevPath.string());
            p << "NA";
        n.close();
        p.close();
        nextCommitID = "NA";
        prevCommitID = "NA";

    }catch (filesystem::filesystem_error& e) {
        cerr<<"Error occured while creating directory: "<<e.what()<<endl;
        throw;
    }
}

void CommitNode::revertCommitData(string id) {
    try{
        filesystem::create_directories(filesystem::current_path()/".Minivcs"/"commits"/commitID/"Data");

        filesystem::path infoPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"info.txt";

        ofstream file(infoPath.string());

        if (!file) {
            throw runtime_error("Failed to open file");
        }
        time_t timestamp; //this is just to get the current time
        time(&timestamp);
        string ts = ctime(&timestamp);
        file<<"1. COMMIT ID: " + commitID + "\n2. COMMIT MESSAGE: " + commitMsg + "\n3. DATE & TIME OF COMMIT: " + ts + "\n";

        filesystem::path OldPath = filesystem::current_path()/".Minivcs"/"commits"/id/"Data";

        if (!filesystem::exists(OldPath)) {
            throw runtime_error("Old directory doesn't exist");
        }

        filesystem::path newDataPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"Data";

        for (auto &entry : filesystem::recursive_directory_iterator(OldPath)) {
            auto relative = filesystem::relative(entry.path(), OldPath);
            auto dest = newDataPath / relative;

            if (filesystem::is_directory(entry)) {
                filesystem::create_directories(dest);
            } else {
                filesystem::copy_file(entry, dest, filesystem::copy_options::overwrite_existing);
            }

        }
    }catch (filesystem::filesystem_error& e) {
        cerr<<"Something went wrong while creating the directory: "<<e.what()<<endl;
    }
}
void CommitNode::loadNodeInfo() {

    filesystem::path infoPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"info.txt";

    if (!filesystem::exists(infoPath)) {
        throw runtime_error("Could not find specified path!");
    }

    ifstream fileInfo(infoPath);
    if (!fileInfo) {
        throw runtime_error("Could not find specified file!");
    }
    string line;
    while (getline(fileInfo, line)) {

        if (line.find("1. COMMIT ID: ") == 0) {
            commitID = line.substr(strlen("1. COMMIT ID: "));
        }
        else if (line.find("2. COMMIT MESSAGE: ") == 0) {
            commitMsg = line.substr(strlen("2. COMMIT MESSAGE: "));
        }else {
            continue;
        }
    }

    filesystem::path nextPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"NextCommit.txt";
    if (filesystem::exists(nextPath)) {
        ifstream f(nextPath);
        getline(f, nextCommitID);
    } else {
        nextCommitID = "NA";
    }

    filesystem::path prevPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"PrevCommit.txt";
    if (filesystem::exists(prevPath)) {
        ifstream f(prevPath);
        getline(f, prevCommitID);
    } else {
        prevCommitID = "NA";
    }
}





void CommitNode::setCommitID(string i) {

    commitID = i;
}

void CommitNode::setCommitMsg(string m) {

    commitMsg = m;
}

void CommitNode::setNextID(string n) {

    nextCommitID = n;

}

void CommitNode::setNextNode(CommitNode*n) {

    nextNode = n;
}

void CommitNode::setPrevID(string n) {

    prevCommitID = n;

}

void CommitNode::setPrevNode(CommitNode*n) {

    prevNode = n;
}

string CommitNode::getCommitID() {
    return commitID;
}

string CommitNode::getCommitMsg() {
    return commitMsg;
}

string CommitNode::getNextID() {
    return nextCommitID;
}

CommitNode* CommitNode::getNextNode() {
    return nextNode;
}

string CommitNode::getPrevID() {
    return prevCommitID;
}

CommitNode* CommitNode::getPrevNode() {
    return prevNode;
}




void CommitNode::saveNextID(string id) {


    filesystem::path path = filesystem::current_path()/".Minivcs"/"commits"/commitID/"NextCommit.txt";

    ofstream file(path.string());
    if (!file) {
        throw runtime_error("Could not save id to prevcommit.txt");
    }

    file<<id;
    file.close();

}

void CommitNode::savePrevID(string id) {


    filesystem::path path = filesystem::current_path()/".Minivcs"/"commits"/commitID/"PrevCommit.txt";

    ofstream file(path.string());
    if (!file) {
        throw runtime_error("Could not save id to nextcommit.txt");
    }

    file<<id;
    file.close();

}
