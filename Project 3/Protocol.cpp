#include "Cache.cpp"

using namespace std;

class Protocol {
    public:
        vector<Cache> cores;
        int cacheHits;
        int cacheMisses;
        int writebacks;
        int broadcasts;
        int cacheToCacheTransfers;

        Protocol(vector<string> coreIDs) {
            cacheHits = 0;
            cacheMisses = 0;
            writebacks = 0;
            broadcasts = 0;
            cacheToCacheTransfers = 0;

            for (int i = 0; i < coreIDs.size(); i++) {
                cores.push_back(Cache(coreIDs[i]));
            }
        }

        //getters===============================
        vector<Cache> getCores() { 
            return cores; 
        }

        int getCacheHits() { 
            return cacheHits; 
        }

        int getCacheMisses() { 
            return cacheMisses; 
        }

        int getWritebacks() { 
            return writebacks; 
        }

        int getBroadcasts() { 
            return broadcasts; 
        }

        int getTransfers() { 
            return cacheToCacheTransfers; 
        }
        //getters===============================
        
        int findCoreIndex(string coreID) {
            for (int i = 0; i < cores.size(); ++i)
                if (cores[i].getCoreID() == coreID)
                    return i;

            return -1;
        } 

        void invalidateOtherCores(int coreIndex, string tag) {
            for (int i = 0; i < cores.size(); ++i) {
                if (i == coreIndex) continue;
                Cache& curCore = cores[i];
                int lineIndex = curCore.findLine(tag);
                if (lineIndex != -1)
                    curCore.resetLine(lineIndex);
            }
        }

        void transitionState(int coreIndex, string tag, CoherencyState newState) {
            Cache& core = cores[coreIndex];
            int lineIndex = core.findLine(tag);
            if (lineIndex != -1)
                core.setLineState(lineIndex, newState);
        }

        void processRead(string coreID, string tag) {
            // Step 1: Find the specific cache making the request
            int coreIndex = findCoreIndex(coreID);
            Cache& requestingCache = cores[coreIndex];

            // Step 2: Check if the tag/line exists in the requesting cache
            int lineIndex = requestingCache.findLine(tag);
            if (lineIndex != -1) {
                // Tag/line found in cache
                cacheHits++;
                char state = requestingCache.getLineState(lineIndex);
                if (state != EXCLUSIVE)
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
                    if (otherState == MODIFIED || otherState == OWNED) {
                        // Other cache has modified data; needs to write back
                        writebacks++;
                        otherCache.setLineState(otherLineIndex, SHARED);
                        otherCache.setDirtyBit(otherLineIndex, 0);
                    }
                    if (otherState == EXCLUSIVE)
                        otherCache.setLineState(otherLineIndex, FORWARD); // Upgrade other cache to Exclusive state
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
                requestingCache.installLine(targetLine, SHARED, tag, false);
                cacheToCacheTransfers++;
            } else
                requestingCache.installLine(targetLine, EXCLUSIVE, tag, false); // Line is exclusive to requesting cache
        }

        void processWrite(string coreID, string tag) {
            // Step 1: Find the specific cache making the request
            int coreIndex = findCoreIndex(coreID);
            Cache& requestingCache = cores[coreIndex];

            // Step 2: Check if the tag/line exists in the requesting cache
            int lineIndex = requestingCache.findLine(tag);
            if (lineIndex != -1) {
                // Tag/line found in cache
                char state = requestingCache.getLineState(lineIndex);
                cacheHits++;
                if (state != EXCLUSIVE)
                    broadcasts++; // Broadcast occurs unless the state is Exclusive (E)
                // Update state
                invalidateOtherCores(lineIndex, tag);
                requestingCache.setLineState(lineIndex, MODIFIED);
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
                    if (otherState == MODIFIED || otherState == OWNED)
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

            requestingCache.installLine(targetLine, MODIFIED, tag, true); // Set state to Modified (M) since we're writing
        }
        
        // Final behavior
        void printStatistics() {
            cout << cacheHits << endl;
            cout << cacheMisses << endl;
            cout << writebacks << endl;
            cout << broadcasts << endl;
            cout << cacheToCacheTransfers;
        };        
};
