#include "Set.cpp"

using namespace std;

static const int ROWS_PER_CORE = 4;
static const int TRACE_OUTPUT_CACHE = false;

class Cache {
    public:
        string ID;
        vector<Set> rows; //FOUR ROWS

        Cache(string id){
            ID = id;
            Set defaultSet; // Calls the default constructor
            rows = vector<Set>(ROWS_PER_CORE, defaultSet);

            installRow(0, INVALID, "0", false);
            installRow(1, INVALID, "0", false);
            installRow(2, INVALID, "0", false);
            installRow(3, INVALID, "0", false);
        }

        bool isCacheFull() {
            for (int i = 0; i < rows.size(); i++) {
                if (rows[i].coherencyState == INVALID) {
                    return false;
                }
            }

            return true;
        }

        // Find valid Row with given tagBitsz
        int findRow(string tagBits) {
            for (int i = 0; i < rows.size(); i++) {
                if (rows[i].tagBits == tagBits && rows[i].coherencyState != INVALID) {
                    return i;
                }
            }

            return -1;
        }

        // Find first invalid Row for replacement
        int findFirstInvalidRow() {
            for (int i = 0; i < rows.size(); i++) {
                if (rows[i].coherencyState == INVALID) {
                    return i;
                }
            }

            return -1;
        }

        // Find LRU Row when no invalid Row exists
        int findLRURow() {
            int LRUindex = 0;
            for (int i = 0; i < rows.size(); i++) {
                if (rows[i].LRUState == 0) {
                    LRUindex = i;
                    break;
                }
            }

            return LRUindex;
        }

        // Returns true if Row needs write back
        bool rowNeedsWriteBack(int rowIndex) {
            return rows[rowIndex].coherencyState == MODIFIED || rows[rowIndex].coherencyState == OWNED || rows[rowIndex].dirtyBit;
        }

        // Update LRU when a Row is accessed
        void updateLRU(int accessedRowIndex) {
            // Decrement LRU counters of non-accessed Rows
            int curAccessedLRUState = rows[accessedRowIndex].LRUState;
            for (int i = 0; i < rows.size(); i++) {
                if (i != accessedRowIndex && rows[i].LRUState > curAccessedLRUState) {
                    rows[i].LRUState--;
                }
            }
            // Set accessed Row to MRU
            rows[accessedRowIndex].LRUState = ROWS_PER_CORE - 1;
        }

        // Install new Row at given index
        void installRow(int rowIndex, CoherencyState coherencyState, string tagBits, int dirtyBit) {
            rows[rowIndex].coherencyState = coherencyState;
            rows[rowIndex].tagBits = tagBits;
            rows[rowIndex].dirtyBit = dirtyBit;
            updateLRU(rowIndex);
        }

        // Reset Row at given index
        void resetRow(int rowIndex) {
            rows[rowIndex].coherencyState = INVALID;
            rows[rowIndex].tagBits = "0";
            rows[rowIndex].dirtyBit = 0;
            updateLRU(rowIndex);
        }

        // Does the same as resetRow, more of an API thing
        void invalidateRow(string tagBits) {
            int rowIndex = findRow(tagBits);
            if (rowIndex != -1) {
                resetRow(rowIndex);
            }
        }

        void printCore() {
            if (TRACE_OUTPUT_CACHE == true) {
                for (int i = 0; i < rows.size(); i++) {
                    cout << "ROW: " << i << "====================" << endl;
                    cout << "dirtyBit: " << rows[i].dirtyBit << endl;
                    cout << "tagBits: " << rows[i].tagBits << endl;
                    cout << "LRUState: " << rows[i].LRUState << endl;
                    cout << "coherencyState: " << rows[i].coherencyState << endl;
                }
            }
        }
};
