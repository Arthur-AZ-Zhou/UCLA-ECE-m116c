#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define NUM_LINES 4

struct CacheLine {
    char coherencyState; // MOESIF
    int lruState; // 0 (LRU) - 3 (MRU)
    string tag;
    bool dirty;
};

class Cache {
  public:
    Cache(string id);
    string getCoreID() const { return coreID; }
    CacheLine& getCacheLine(int lineIndex) { return lines[lineIndex]; }
    vector<CacheLine>& getAllCacheLines() { return lines; }

    bool isCacheFull() const;
    int  findLine(string tag) const;
    int  findFirstInvalidLine() const;
    int  findLRULine() const;
    bool lineNeedsWriteBack(int lineIndex) const;
    char getLineState(int lineIndex) const;
    bool isLineOwned(int lineIndex) const;
    bool isLineExclusive(int lineIndex) const;
    bool isLineForward(int lineIndex) const;
    void updateLRU(int accessedLineIndex);
    void installLine(int lineIndex, char coherencyState, string tag, bool dirty);
    void updateLineWithoutLRU(int lineIndex, char newState, bool newDirty);
    void setLineState(int lineIndex, char coherencyState);
    void setDirtyBit(int lineIndex, bool dirty);
    void resetLine(int lineIndex);
    void invalidateLine(string tag);

    void printCacheState() const;

  private:
    vector<CacheLine> lines; // 4 lines
    string coreID;
};

#endif