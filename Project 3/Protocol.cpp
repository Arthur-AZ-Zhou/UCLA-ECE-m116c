#include "Cache.cpp"

using namespace std;

class Protocol {
    public:
        vector<Cache> cores; //AT MOST 4
        int cacheHits;
        int cacheMisses;
        int writebacks;
        int broadcasts;
        int cacheToCacheTransfers;

        Protocol(vector<string> IDs) {
            cacheHits = 0;
            cacheMisses = 0;
            writebacks = 0;
            broadcasts = 0;
            cacheToCacheTransfers = 0;

            for (int i = 0; i < IDs.size(); i++) {
                cores.push_back(Cache(IDs[i]));
            }
        }
        
        int findCoreIndex(string ID) {
            for (int i = 0; i < cores.size(); i++) {
                if (cores[i].ID == ID) {
                    return i;
                }
            }

            return -1;
        } 

        void invalidateOtherCores(int coreIndex, string tag) {
            for (int i = 0; i < cores.size(); i++) {
                if (i == coreIndex) {
                    continue;
                } else {
                    Cache& curCore = cores[i];
                    int rowIndex = curCore.findTagRow(tag);

                    if (rowIndex != -1) {
                        curCore.resetRow(rowIndex);
                    }
                }
            }
        }

        void transitionState(int coreIndex, string tag, CoherencyState newState) {
            Cache& core = cores[coreIndex];
            int rowIndex = core.findTagRow(tag);
            if (rowIndex != -1) {
                core.rows[rowIndex].coherencyState = newState;
            }
        }

        void processRead(string ID, string tag) {
            // Step 1: Find the specific cache making the request
            int coreIndex = findCoreIndex(ID);
            Cache& requestingCache = cores[coreIndex];

            // Step 2: Check if the tag/Row exists in the requesting cache
            int rowIndex = requestingCache.findTagRow(tag);
            if (rowIndex != -1) {
                // Tag/Row found in cache
                cacheHits++;
                char state = requestingCache.rows[rowIndex].coherencyState;

                if (state != EXCLUSIVE) {
                    broadcasts++; // Broadcast occurs unless the state is Exclusive (E) per Nader
                }
                requestingCache.adjustLRU(rowIndex);
                
                return;
            }

            // By this point, read miss
            cacheMisses++;
            broadcasts++; // Broadcast occurs on miss (since Row is not in E state in requesting cache)

            // Step 3: Check other caches for the Row
            bool foundInOtherCache = false;
            for (int i = 0; i < cores.size(); ++i) {
                if (i == coreIndex) { 
                    continue;
                } else {
                    Cache& otherCache = cores[i];
                    int otherRowIndex = otherCache.findTagRow(tag);
                    if (otherRowIndex != -1) {
                        char otherState = otherCache.rows[otherRowIndex].coherencyState;
                        if (otherState == MODIFIED || otherState == OWNED) {
                            // Other cache has modified data; needs to write back
                            writebacks++;
                            otherCache.rows[otherRowIndex].coherencyState = SHARED;
                            otherCache.rows[otherRowIndex].dirtyBit = 0;
                        }
                        if (otherState == EXCLUSIVE) {
                            otherCache.rows[otherRowIndex].coherencyState = FORWARD; // Upgrade other cache to Exclusive state
                        }
                        foundInOtherCache = true;
                        break;
                    }
                }
            }

            // // Step 4: Install the Row in the requesting cache
            // int targetRow = requestingCache.findFirstInvalidRow();
            // if (targetRow == -1) {
            //     // Need to evict a Row
            //     targetRow = requestingCache.findOldestRow();
            //     if (requestingCache.rowNeedsWriteBack(targetRow)) {
            //         writebacks++;
            //     }
            // }

            int targetRow = requestingCache.findBootRow();

            if (requestingCache.writeBackRow(targetRow)) {
                writebacks++;
            }

            // Set state based on whether other caches have the Row
            if (foundInOtherCache) {
                // Row is shared among caches
                requestingCache.setRow(0, tag, SHARED, targetRow);
                cacheToCacheTransfers++;
            } else
                requestingCache.setRow(0, tag, EXCLUSIVE, targetRow); // Row is exclusive to requesting cache
        }

        void processWrite(string ID, string tag) {
            // Step 1: Find the specific cache making the request
            int coreIndex = findCoreIndex(ID);
            Cache& requestingCache = cores[coreIndex];

            // Step 2: Check if the tag/Row exists in the requesting cache
            int rowIndex = requestingCache.findTagRow(tag);
            if (rowIndex != -1) {
                // Tag/Row found in cache
                char state = requestingCache.rows[rowIndex].coherencyState;
                cacheHits++;
                if (state != EXCLUSIVE) {
                    broadcasts++; // Broadcast occurs unless the state is Exclusive (E)
                }
                // Update state
                invalidateOtherCores(rowIndex, tag);
                requestingCache.rows[rowIndex].coherencyState = MODIFIED;
                requestingCache.adjustLRU(rowIndex);
                return;
            }

            // By this point, read miss
            cacheMisses++;
            broadcasts++; // Broadcast occurs on miss (since Row is not in E state in requesting cache)

            // Step 3: Invalidate other caches and check for writebacks
            bool foundInOtherCache = false;
            for (int i = 0; i < cores.size(); ++i) {
                if (i == coreIndex) { 
                    continue;
                } else {

                    Cache& otherCache = cores[i];
                    int otherRowIndex = otherCache.findTagRow(tag);
                    if (otherRowIndex != -1) {
                        char otherState = otherCache.rows[otherRowIndex].coherencyState;
                        if (otherState == MODIFIED || otherState == OWNED) {
                            writebacks++; // Other cache has modified data; needs to write back
                        }
                        // Invalidate the Row in other caches
                        otherCache.resetRow(otherRowIndex);
                        foundInOtherCache = true;
                    }
                }
            }

            // Step 4: Install the Row in the requesting cache
            // int targetRow = requestingCache.findFirstInvalidRow();
            // if (targetRow == -1) {
            //     // Need to evict a Row
            //     targetRow = requestingCache.findOldestRow();
            //     if (requestingCache.rowNeedsWriteBack(targetRow)) {
            //         writebacks++;
            //     }
            // }

            int targetRow = requestingCache.findBootRow();

            if (requestingCache.writeBackRow(targetRow)) {
                writebacks++;
            }

            requestingCache.setRow(1, tag, MODIFIED, targetRow); // Set state to Modified (M) since we're writing
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
