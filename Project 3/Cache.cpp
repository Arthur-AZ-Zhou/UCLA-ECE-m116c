#include "Set.cpp"

using namespace std;

static const int32_t ROWS_PER_CORE = 4;
static const bool TRACE_OUTPUT_CACHE = false;

class Cache {
    public:
        string ID;
        vector<Set> rows; //FOUR ROWS

        Cache(string id){
            ID = id;
            Set defaultSet; //calls default constructor
            rows = vector<Set>(ROWS_PER_CORE, defaultSet);

            rows.clear(); // Ensure rows vector is empty
            rows.push_back(Set()); // Default constructor for row 0
            rows.back().dirtyBit = 0;
            rows.back().tagBits = "0";
            rows.back().LRUState = 0;
            rows.back().coherencyState = INVALID;

            rows.push_back(Set()); // Default constructor for row 1
            rows.back().dirtyBit = 0;
            rows.back().tagBits = "0";
            rows.back().LRUState = 0;
            rows.back().coherencyState = INVALID;

            rows.push_back(Set()); // Default constructor for row 2
            rows.back().dirtyBit = 0;
            rows.back().tagBits = "0";
            rows.back().LRUState = 0;
            rows.back().coherencyState = INVALID;

            rows.push_back(Set()); // Default constructor for row 3
            rows.back().dirtyBit = 0;
            rows.back().tagBits = "0";
            rows.back().LRUState = 0;
            rows.back().coherencyState = INVALID;

            // initializeRow(0, "0", INVALID, 0);
            // initializeRow(0, "0", INVALID, 1);
            // initializeRow(0, "0", INVALID, 2);
            // initializeRow(0, "0", INVALID, 3);
        }

        // int32_t findRow(const string tagBits) { //Finds the index of a row with the given tag bits and a valid coherency state
        
        // void setRow(int32_t dirtyBit, string tagBits, int32_t LRUState, CoherencyState coherencyState, int32_t it) {
        void setRow(int32_t dirtyBit, string tagBits, CoherencyState coherencyState, int32_t indexOfRow) {
            rows[indexOfRow].dirtyBit = dirtyBit;
            rows[indexOfRow].tagBits = tagBits;
            // rows[indexOfRow].LRUState = LRUState;
            rows[indexOfRow].coherencyState = coherencyState;

            adjustLRU(indexOfRow);

            if (TRACE_OUTPUT_CACHE) {
                cout << "ROW: " << indexOfRow << "====================" << endl;
                cout << "dirtyBit: " << rows[indexOfRow].dirtyBit << endl;
                cout << "tagBits: " << rows[indexOfRow].tagBits << endl;
                cout << "LRUState: " << rows[indexOfRow].LRUState << endl;
                cout << "coherencyState: " << rows[indexOfRow].coherencyState << endl;
            }
        }

        int32_t findTagRow(string tagBits) { //Finds the index of a row with the given tag bits and a valid coherency state
            // for (auto it : rows) { 
            //     if (it.coherencyState != INVALID) {
            //         if (it.tagBits == tagBits) {
            //             cout << "TRIGGER TAGBIT FOUND" << endl;
            //             return &it - &rows[0];

            int32_t i = 0;
            for (auto& r : rows) {
                if (r.coherencyState != INVALID) {
                    if (r.tagBits == tagBits) {
                        if (TRACE_OUTPUT_CACHE == true) {
                            cout << "ith tagbit found: " << i << endl;
                        }

                        return i;
                    }
                }

                i++;
            }

            // printCache();

            return -1; //NO MATCHING ROW FOUND
        }

        // int32_t findFirstInvalidRow() { //get first invalid row to boot
        //     printCore();

        //     for (int32_t i = 0; i < rows.size(); i++) {
        //         if (rows[i].coherencyState == INVALID) { 

        int32_t findBootRow() { //find first row to boot
            printCache();

            for (int32_t i = 0; i < rows.size(); i++) { //find invalid
                if (rows[i].coherencyState == INVALID) {
                    // if (TRACE_OUTPUT_CACHE) {
                    //     cout << "INVALID TRIGGER" << endl;
                    // }
                    return i; 
                }
            }

            for (int32_t i = 0; i < rows.size(); ++i) { //find oldest/LRU
                if (rows[i].LRUState == 0) {
                    // if (TRACE_OUTPUT_CACHE) {
                    //     cout << "LRU TRIGGER" << endl;
                    // }
                    return i; 
                }
            }

            if (TRACE_OUTPUT_CACHE) {
                cout << "WE SHOULD NEVER GET HERE" << endl;
            } 
            return -1; 
        }

        void resetRow(int32_t indexOfRow) {
            int32_t oldLRU = rows[indexOfRow].LRUState;

            rows[indexOfRow] = Set();
            rows[indexOfRow].LRUState = oldLRU;

            adjustLRU(indexOfRow);
        }

        // void initializeRow(int32_t dirtyBit, string tagBits, int32_t LRUState, CoherencyState coherencyState, int32_t indexOfRow) {
        //     rows[indexOfRow].dirtyBit = dirtyBit;
        // }

        bool writeBackRow(int32_t rowIndex) {
            if (rows[rowIndex].coherencyState == MODIFIED) {
                return true;
            }

            if (rows[rowIndex].coherencyState == OWNED) {
                if (TRACE_OUTPUT_CACHE) {
                    cout << "OWNED TRIGGER" << endl;
                }

                return true;
            }

            if (rows[rowIndex].dirtyBit == true) {
                return true;
            }

            return false;
        }

        void adjustLRU(int32_t accessedRowIndex) { //KEEP TRACK OF LRU
            for (int32_t i = 0; i < rows.size(); i++) {
                if (i == accessedRowIndex) { //CANNOT BE OWN ROW
                    continue;
                } else {
                    if (rows[accessedRowIndex].LRUState < rows[i].LRUState) { //check if LRU is greater than the accessed row's LRU
                        if (TRACE_OUTPUT_CACHE) {
                            cout << "adjusted row: " << i << endl;
                        }

                        rows[i].LRUState--;
                    }
                }
            }

            printCache();
            rows[accessedRowIndex].LRUState = rows.size() - 1;
        }

        void printCache() {
            if (TRACE_OUTPUT_CACHE == true) {
                for (int32_t i = 0; i < rows.size(); i++) {
                    cout << "ROW: " << i << "====================" << endl;
                    cout << "dirtyBit: " << rows[i].dirtyBit << endl;
                    cout << "tagBits: " << rows[i].tagBits << endl;
                    cout << "LRUState: " << rows[i].LRUState << endl;
                    cout << "coherencyState: " << rows[i].coherencyState << endl;
                }
                
                cout << endl;
            }
        }
};
