#ifndef COMMITNODE_H
#define COMMITNODE_H
#include <string>
using namespace std;

class CommitNode {

private:

    string commitID;
    string commitMsg;
    string nextCommitID;
    string prevCommitID;

    CommitNode* nextNode;
    CommitNode* prevNode;

public:

    CommitNode();
    CommitNode(string cI, string cM);
    CommitNode(string cI);

    void createCommitData();
    void revertCommitData(string id);
    void loadNodeInfo();

    void setCommitID(string i);
    void setCommitMsg(string m);
    void setNextID(string n);
    void setNextNode(CommitNode* n);

    string getCommitID();
    string getCommitMsg();
    string getNextID();
    CommitNode* getNextNode();




    void setPrevNode(CommitNode* p);
    CommitNode* getPrevNode();

    void setPrevID(string p);
    string getPrevID();

    void savePrevID(string id);


    void saveNextID(string id);
};

#endif