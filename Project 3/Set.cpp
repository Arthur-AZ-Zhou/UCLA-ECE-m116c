#include <bits/stdc++.h>

using namespace std;

enum CoherencyState { //use enums because it is gonna get way too complicated if we just keep track of the chars
    MODIFIED = 'M',
    OWNED = 'O',
    EXCLUSIVE = 'E',
    SHARED = 'S',
    INVALID = 'I',
    FORWARD = 'F'
};

class Set {
    public:
        int dirtyBit;
        string tagBits;
        int LRUState; //0 is the OLDEST
        CoherencyState coherencyState; 

        Set() : dirtyBit(0), tagBits("0"), LRUState(0), coherencyState(INVALID) {} //DEFAULT CONSTRUCTOR
};