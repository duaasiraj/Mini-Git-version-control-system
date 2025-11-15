#include <iostream>
#include "HashingHelper.h"
using namespace std;

int main() {

    cout<<generateCommitID()<<endl;
    cout<<generateCommitID("this is a test")<<endl;

return 0;

}
