#ifndef COMMITNODE_H
#define COMMITNODE_H

class commitNode {

private:

    string commitID;
    string commitMsg;
    string nextCommitID;

    commitNode* nextNode;

public:

    commitNode();
    commitNode(string cI, string cM = "(No message)");

    void createCommitNode();
    void revertCommitNode(string id);
    void getNodeInfo();
    string checkNextCommit();

    void setCommitID(string i);
    void setCommitMsg(string m);
    void setNextID(string n);
    void setNextNode(commitNode* n);

    string getCommitID();
    string getCommitMsg();
    string getNextID();
    commitNode* getNextNode();

    void writeNextID(string id);
};

#endif