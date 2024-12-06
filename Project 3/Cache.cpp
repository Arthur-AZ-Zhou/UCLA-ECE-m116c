#include "Cache.h"

// Initialize 4 lines for this private cache
Cache::Cache(string id) : coreID(id) {
    lines.resize(NUM_LINES);
    for (int i = 0; i < NUM_LINES; ++i)
        installLine(i, 'I', "0", false);
}

// Returns true if cache is full
bool Cache::isCacheFull() const {
    for (const auto& line : lines)
        if (line.coherencyState == 'I')
            return false;
    return true;
}

// Find valid line with given tag
int Cache::findLine(string tag) const {
    for (int i = 0; i < NUM_LINES; ++i)
        if (lines[i].tag == tag && lines[i].coherencyState != 'I')
            return i;
    return -1;
}

// Find first invalid line for replacement
int Cache::findFirstInvalidLine() const {
    for (int i = 0; i < NUM_LINES; ++i)
        if (lines[i].coherencyState == 'I')
            return i;
    return -1;
}

// Find LRU line when no invalid line exists
int Cache::findLRULine() const {
    int LRUindex = 0;
    for (int i = 0; i < NUM_LINES; ++i)
        if (lines[i].lruState == 0) {
            LRUindex = i;
            break;
        }
    return LRUindex;
}

// Returns true if line needs write back
bool Cache::lineNeedsWriteBack(int lineIndex) const {
    return getLineState(lineIndex) == 'M' || isLineOwned(lineIndex) || lines[lineIndex].dirty;
}

// Returns coherency state of selected line
char Cache::getLineState(int lineIndex) const { return lines[lineIndex].coherencyState; }

// Returns true if line is owned
bool Cache::isLineOwned(int lineIndex) const { return getLineState(lineIndex) == 'O'; }

// Returns true if line is exclusive
bool Cache::isLineExclusive(int lineIndex) const { return getLineState(lineIndex) == 'E'; }

// Returns true if line is forward
bool Cache::isLineForward(int lineIndex) const { return getLineState(lineIndex) == 'F'; }

// Update LRU when a line is accessed
void Cache::updateLRU(int accessedLineIndex) {
    // Decrement LRU counters of non-accessed lines
    int curAccessedLRUState = lines[accessedLineIndex].lruState;
    for (int i = 0; i < NUM_LINES; ++i)
        if (i != accessedLineIndex && lines[i].lruState > curAccessedLRUState)
            lines[i].lruState--;
    // Set accessed line to MRU
    lines[accessedLineIndex].lruState = NUM_LINES - 1;
}

// Install new line at given index
void Cache::installLine(int lineIndex, char coherencyState, string tag, bool dirty) {
    lines[lineIndex].coherencyState = coherencyState;
    lines[lineIndex].tag = tag;
    lines[lineIndex].dirty = dirty;
    updateLRU(lineIndex);
}

// For API
void Cache::updateLineWithoutLRU(int lineIndex, char newState, bool newDirty) {
    setLineState(lineIndex, newState);
    setDirtyBit(lineIndex, newDirty);
}

// Directly changes the selected line's coherency state
void Cache::setLineState(int lineIndex, char newState) { lines[lineIndex].coherencyState = newState; }

// Directly changes the selected line's dirty bit
void Cache::setDirtyBit(int lineIndex, bool dirty) { lines[lineIndex].dirty = dirty; }

// Reset line at given index
void Cache::resetLine(int lineIndex) {
    lines[lineIndex].coherencyState = 'I';
    lines[lineIndex].tag = "0";
    lines[lineIndex].dirty = false;
    updateLRU(lineIndex);
}

// Does the same as resetLine, more of an API thing
void Cache::invalidateLine(string tag) {
    int lineIndex = findLine(tag);
    if (lineIndex != -1)
        resetLine(lineIndex);
}

// Print debugging
void Cache::printCacheState() const {
    for (int i = 0; i < NUM_LINES; ++i) {
        cout << "Line " << i 
        << ": State=" << lines[i].coherencyState 
        << ", Tag=" << lines[i].tag 
        << ", LRU=" << lines[i].lruState 
        << ", Dirty=" << lines[i].dirty << endl;
    }
}
