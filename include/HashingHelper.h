#ifndef HASHINGHELPER_H
#define HASHINGHELPER_H

#include <string>
using namespace std;

string generateCommitID(const string& data = "");
//HEADER file for our hashing helper. This just decalres the function.
//Implementation inside HashingHelpier.cpp
//Hashing helper uses FNV-1a hash

//this header file is just to keep code organized and make inclusion in main easier

#endif