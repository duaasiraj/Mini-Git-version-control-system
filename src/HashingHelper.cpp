#include "HashingHelper.h"
#include <random> //will use it to generate rnadm 64 bit numbers
#include <chrono> //used to get the current time
#include <sstream> //used to create a combined string
#include <iomanip> //used so that all IDs will have the same full length
#include <cstdint> //we use this for our unsined 64 bit integr data type

using namespace std;
/*==============================================
FNV1A
Return type: unsigned 64 bit integer
Parameters: string&
Purpose: generate a hash of the data we'll be giving
This data will be in the from of:
(optional)input + random 64 bit integer + time since epoch (the time passed since Thursday 1 January 1970)
We use the pre defined FNV offset and FNV prime to calculate a hash
this hash is formed by:
    1. XORing each char in the data string with the offset
    2. Multiplying the previous result with the prime

This forms an unsigned 64 bit integer, which we return
================================================*/
static uint64_t FNV1A(const string& data) {

    const uint64_t offset = 14695981039346656037ULL; //ULL to ensure they are 64 bit unsigned literals
    const uint64_t prime = 1099511628211ULL;

    //the above 2 values are the standard values defined by the FNV1A hash for 64 bits

    uint64_t generatedHash = offset;

    for (int i=0;i<data.length();i++) {

        generatedHash = generatedHash^data[i];
        generatedHash = generatedHash*prime;

    }

    return generatedHash;
}

/*==============================================
generateCommitID
Return type: string
Parameters: string&
Purpose: generate the commit ID. will use the FNV1A function above

1. we use random_device from <random> since it uses hardware randomness to create random numbers
    - we call said random_device object, and typecast the returning value to unsigned 64 bit integer
    -to ensure randomness, we shift these 32 bits to the left, so the lower half is empty
    -we call the ibject again, and XOR, so that the lower half is filled with another random number
    (random_device usually produces an atleast 32 bit integer)
2. we use high resolution clock in chrono since it's the most precise one
    -we use time since epoch, which defined in computer terminology is the time passed since thursday 1 january 1970
    -count just gives a numeric vlaue to that time
3. combined data consists of all this random stuff we made up and is sent to the FNV1a function to be hashed
4. for standardization's sake, we take the generated ID and ensure it's 16 nibbles long (8 bytes) and set any empty parts to 0
5. we use stringstream object to stre this hex value, then we convert it back to string and return it
================================================*/

string generateCommitID(const string &data) {

    random_device rand; //uses randomness of the system itself to create random numbers

    uint64_t randomNumber = ((uint64_t)rand() << 32)^rand();
    uint64_t time = chrono::high_resolution_clock::now().time_since_epoch().count();

    string combinedData = data + to_string(randomNumber) + to_string(time);

    uint64_t generatedIDHash = FNV1A(combinedData);

    stringstream generatedID;
    generatedID<<hex<<setw(16)<<setfill('0')<<generatedIDHash;

    return generatedID.str();
}

