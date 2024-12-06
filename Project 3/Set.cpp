#include <bits/stdc++.h>

using namespace std;

enum CoherencyState {
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
        int LRUState; //can be from 0 to 3
        CoherencyState coherencyState; 
};