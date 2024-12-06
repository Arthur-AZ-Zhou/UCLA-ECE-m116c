#include "CacheManager.h"

// Initialize Cache Manager to hold at most 4 cores
CacheManager::CacheManager(vector<string> coreIDs) : cacheHits(0), cacheMisses(0), writebacks(0), broadcasts(0), cacheToCacheTransfers(0) {
    for (string id : coreIDs) 
        cores.push_back(Cache(id));
}

// Returns index of core based on given ID
int CacheManager::findCoreIndex(string coreID) const {
    for (int i = 0; i < cores.size(); ++i)
        if (cores[i].getCoreID() == coreID)
            return i;
}

// Invalidates any non-selected core that contains the tag
void CacheManager::invalidateOtherCores(int coreIndex, string tag) {
    for (int i = 0; i < cores.size(); ++i) {
        if (i == coreIndex) continue;
        Cache& curCore = cores[i];
        int lineIndex = curCore.findLine(tag);
        if (lineIndex != -1)
            curCore.resetLine(lineIndex);
    }
}

// Changes state of selected core if it contains the tag
void CacheManager::transitionState(int coreIndex, string tag, char newState) {
    Cache& core = cores[coreIndex];
    int lineIndex = core.findLine(tag);
    if (lineIndex != -1)
        core.setLineState(lineIndex, newState);
}

void CacheManager::processRead(string coreID, string tag) {
    // Step 1: Find the specific cache making the request
    int coreIndex = findCoreIndex(coreID);
    Cache& requestingCache = cores[coreIndex];

    // Step 2: Check if the tag/line exists in the requesting cache
    int lineIndex = requestingCache.findLine(tag);
    if (lineIndex != -1) {
        // Tag/line found in cache
        cacheHits++;
        char state = requestingCache.getLineState(lineIndex);
        if (state != 'E')
            broadcasts++; // Broadcast occurs unless the state is Exclusive (E) per Nader
        requestingCache.updateLRU(lineIndex);
        return;
    }

    // By this point, read miss
    cacheMisses++;
    broadcasts++; // Broadcast occurs on miss (since line is not in E state in requesting cache)

    // Step 3: Check other caches for the line
    bool foundInOtherCache = false;
    for (int i = 0; i < cores.size(); ++i) {
        if (i == coreIndex) continue;
        Cache& otherCache = cores[i];
        int otherLineIndex = otherCache.findLine(tag);
        if (otherLineIndex != -1) {
            char otherState = otherCache.getLineState(otherLineIndex);
            if (otherState == 'M' || otherState == 'O') {
                // Other cache has modified data; needs to write back
                writebacks++;
                otherCache.setLineState(otherLineIndex, 'S');
                otherCache.setDirtyBit(otherLineIndex, false);
            }
            if (otherState == 'E')
                otherCache.setLineState(otherLineIndex, 'F'); // Upgrade other cache to Exclusive state
            foundInOtherCache = true;
            break;
        }
    }

    // Step 4: Install the line in the requesting cache
    int targetLine = requestingCache.findFirstInvalidLine();
    if (targetLine == -1) {
        // Need to evict a line
        targetLine = requestingCache.findLRULine();
        if (requestingCache.lineNeedsWriteBack(targetLine))
            writebacks++;
    }

    // Set state based on whether other caches have the line
    if (foundInOtherCache) {
        // Line is shared among caches
        requestingCache.installLine(targetLine, 'S', tag, false);
        cacheToCacheTransfers++;
    } else
        requestingCache.installLine(targetLine, 'E', tag, false); // Line is exclusive to requesting cache
}

void CacheManager::processWrite(string coreID, string tag) {
    // Step 1: Find the specific cache making the request
    int coreIndex = findCoreIndex(coreID);
    Cache& requestingCache = cores[coreIndex];

    // Step 2: Check if the tag/line exists in the requesting cache
    int lineIndex = requestingCache.findLine(tag);
    if (lineIndex != -1) {
        // Tag/line found in cache
        char state = requestingCache.getLineState(lineIndex);
        cacheHits++;
        if (state != 'E')
            broadcasts++; // Broadcast occurs unless the state is Exclusive (E)
        // Update state
        invalidateOtherCores(lineIndex, tag);
        requestingCache.setLineState(lineIndex, 'M');
        requestingCache.updateLRU(lineIndex);
        return;
    }

    // By this point, read miss
    cacheMisses++;
    broadcasts++; // Broadcast occurs on miss (since line is not in E state in requesting cache)

    // Step 3: Invalidate other caches and check for writebacks
    bool foundInOtherCache = false;
    for (int i = 0; i < cores.size(); ++i) {
        if (i == coreIndex) continue;
        Cache& otherCache = cores[i];
        int otherLineIndex = otherCache.findLine(tag);
        if (otherLineIndex != -1) {
            char otherState = otherCache.getLineState(otherLineIndex);
            if (otherState == 'M' || otherState == 'O')
                writebacks++; // Other cache has modified data; needs to write back
            // Invalidate the line in other caches
            otherCache.resetLine(otherLineIndex);
            foundInOtherCache = true;
        }
    }

    // Step 4: Install the line in the requesting cache
    int targetLine = requestingCache.findFirstInvalidLine();
    if (targetLine == -1) {
        // Need to evict a line
        targetLine = requestingCache.findLRULine();
        if (requestingCache.lineNeedsWriteBack(targetLine))
            writebacks++;
    }

    requestingCache.installLine(targetLine, 'M', tag, true); // Set state to Modified (M) since we're writing
}
