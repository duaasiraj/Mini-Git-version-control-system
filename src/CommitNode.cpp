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

}

CommitNode::CommitNode(string cI, string cM) {

    commitID = cI;
    commitMsg = cM;
    nextNode = NULL;

    createCommitData();

}

/*
commits follow the following structure

CUrrent Directory
|-> .Minivcs
|   |
|   |->commits (where all commits are stored)
|   |    |
|   |    |-> <Commit ID> (folders created for each commit)
|   |    |      |
|   |    |      |->info.txt  => holds commit ID, commit Message, timestamp
|   |    |      |->NextCommit.txt   => holds the next commit's ID
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
        filesystem::path stagingAreaPath = filesystem::current_path()/".Minivcs"/"staging_area";
        filesystem::path newDataPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"Data";

        filesystem::copy(stagingAreaPath, newDataPath, filesystem::copy_options::recursive | filesystem::copy_options::update_existing);
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

        filesystem::path OldPath = filesystem::current_path()/".Minivcs"/id/"Data";
        filesystem::path newDataPath = filesystem::current_path()/".Minivcs"/"commits"/commitID/"Data";

        filesystem::copy(OldPath, newDataPath, filesystem::copy_options::recursive);
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
        }
    }
}

string CommitNode::loadNextCommitID() {

    filesystem::path path = filesystem::current_path()/".Minivcs"/"commits"/commitID/"NextCommit.txt";

    if (filesystem::exists(path)) {

        ifstream file(path);
        getline(file, nextCommitID);
        file.close();
        return nextCommitID;
    }
    nextCommitID = "NA";
    return nextCommitID;

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

void CommitNode::saveNextID(string id) {


    filesystem::path path = filesystem::current_path()/".Minivcs"/"commits"/commitID/"NextCommit.txt";

    ofstream file(path.string());
    if (!file) {
        throw runtime_error("Could not save id to nextcommit.txt");
    }

    file<<id;
    file.close();

}
