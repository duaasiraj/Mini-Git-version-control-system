#ifndef COMMITNODE_H
#define COMMITNODE_H

class CommitNode {

private:

    string commitID;
    string commitMsg;
    string nextCommitID;

    CommitNode* nextNode;

public:

    CommitNode();
    CommitNode(string cI, string cM = "(No message)");

    void createCommitData();
    void revertCommitData(string id);
    void loadNodeInfo();
    string loadNextCommitID();

    void setCommitID(string i);
    void setCommitMsg(string m);
    void setNextID(string n);
    void setNextNode(CommitNode* n);

    string getCommitID();
    string getCommitMsg();
    string getNextID();
    CommitNode* getNextNode();

    void saveNextID(string id);
};

#endif