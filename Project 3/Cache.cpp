#include "Set.cpp"

using namespace std;

static const int LINES_PER_CORE = 4;
static const int TRACE_OUTPUT_CACHE = false;

class Cache {
    public:
        string coreID;
        vector<Set> lines; 

        Cache(string id){
            coreID = id;

            lines.resize(4);
            installLine(0, INVALID, "0", false);
            installLine(1, INVALID, "0", false);
            installLine(2, INVALID, "0", false);
            installLine(3, INVALID, "0", false);
        }

        //getters===============================
        string getCoreID() { 
            return coreID;
        }

        // Returns coherency state of selected line
        char getLineState(int lineIndex) { 
            return lines[lineIndex].coherencyState; 
        }

        bool isLineOwned(int lineIndex) { 
            return getLineState(lineIndex) == OWNED; 
        }

        bool isLineExclusive(int lineIndex) { 
            return getLineState(lineIndex) == EXCLUSIVE; 
        }

        bool isLineForward(int lineIndex) { 
            return getLineState(lineIndex) == FORWARD; 
        }
        //getters===============================

        //setters===============================
        void setLineState(int lineIndex, CoherencyState newState) { 
            lines[lineIndex].coherencyState = newState; 
        }

        void setDirtyBit(int lineIndex, int dirtyBit) { 
            lines[lineIndex].dirtyBit = dirtyBit; 
        }
        //setters===============================

        // Returns true if cache is full
        bool isCacheFull() {
            for (const auto& line : lines)
                if (line.coherencyState == 'I')
                    return false;
            return true;
        }

        // Find valid line with given tagBitsz
        int findLine(string tagBits) {
            for (int i = 0; i < LINES_PER_CORE; ++i)
                if (lines[i].tagBits == tagBits && lines[i].coherencyState != 'I')
                    return i;
            return -1;
        }

        // Find first invalid line for replacement
        int findFirstInvalidLine() {
            for (int i = 0; i < LINES_PER_CORE; ++i)
                if (lines[i].coherencyState == 'I')
                    return i;
            return -1;
        }

        // Find LRU line when no invalid line exists
        int findLRULine() {
            int LRUindex = 0;
            for (int i = 0; i < LINES_PER_CORE; ++i)
                if (lines[i].LRUState == 0) {
                    LRUindex = i;
                    break;
                }
            return LRUindex;
        }

        // Returns true if line needs write back
        bool lineNeedsWriteBack(int lineIndex) {
            return getLineState(lineIndex) == MODIFIED || isLineOwned(lineIndex) || lines[lineIndex].dirtyBit;
        }

        // Update LRU when a line is accessed
        void updateLRU(int accessedLineIndex) {
            // Decrement LRU counters of non-accessed lines
            int curAccessedLRUState = lines[accessedLineIndex].LRUState;
            for (int i = 0; i < LINES_PER_CORE; ++i)
                if (i != accessedLineIndex && lines[i].LRUState > curAccessedLRUState)
                    lines[i].LRUState--;
            // Set accessed line to MRU
            lines[accessedLineIndex].LRUState = LINES_PER_CORE - 1;
        }

        // Install new line at given index
        void installLine(int lineIndex, CoherencyState coherencyState, string tagBits, int dirtyBit) {
            lines[lineIndex].coherencyState = coherencyState;
            lines[lineIndex].tagBits = tagBits;
            lines[lineIndex].dirtyBit = dirtyBit;
            updateLRU(lineIndex);
        }

        // Reset line at given index
        void resetLine(int lineIndex) {
            lines[lineIndex].coherencyState = INVALID;
            lines[lineIndex].tagBits = "0";
            lines[lineIndex].dirtyBit = 0;
            updateLRU(lineIndex);
        }

        // Does the same as resetLine, more of an API thing
        void invalidateLine(string tagBits) {
            int lineIndex = findLine(tagBits);
            if (lineIndex != -1)
                resetLine(lineIndex);
        }

        void printCore() {
            if (TRACE_OUTPUT_CACHE == true) {
                for (int i = 0; i < lines.size(); i++) {
                    cout << "LINE: " << i << "====================" << endl;
                    cout << "dirtyBit: " << lines[i].dirtyBit << endl;
                    cout << "tagBits: " << lines[i].tagBits << endl;
                    cout << "LRUState: " << lines[i].LRUState << endl;
                    cout << "coherencyState: " << lines[i].coherencyState << endl;
                }
            }
        }
};
