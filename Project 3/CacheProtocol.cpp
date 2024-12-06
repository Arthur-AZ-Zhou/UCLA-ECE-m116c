#include "Cache.cpp"

using namespace std;
static const bool TRACE_OUTPUT_CACHE_PROTOCOL = false; 

class CacheProtocol {
    public:
        int hits;
        int misses;
        int writeBacks;
        int broadcasts;
        int transfers;
        vector<Cache> allCaches; 

        CacheProtocol(vector<string> cacheIDs) {
            hits = 0;
            misses = 0;
            writeBacks = 0;
            broadcasts = 0;
            transfers = 0;

            for (int i = 0; i < cacheIDs.size(); i++) {
                allCaches.push_back(Cache(cacheIDs[i]));
            }

            printCaches();
        }
        
        int findCache(string cacheID) { //finds specific cache given the ID
            // cout << "FINDCACHE TRIGGER" << endl;

            // auto it = allCaches.begin();
            // while (it != allCaches.end()) {
            //     if (cacheID != it->ID) { 
            //         if (TRACE_OUTPUT_CACHE_PROTOCOL) {
            //             cout << "FOUND ID AT iterator index: " 
            //                 << distance(allCaches.begin(), it) << endl;
            //         }
            //         return distance(allCaches.begin(), it); 
            //     }
            //     it++; 
            // }

            for (int i = 0; i < allCaches.size(); i++) {
                if (cacheID == allCaches[i].ID) {
                    if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                        cout << "FOUND ID AT i: " << i << endl;
                    }

                    return i;
                }
            }

            if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                cout << "SOMEHOW DID NOT FIND ID, SHOULD NEVER GET HERE" << endl;
            }

            return -1;

            //  if (TRACE_OUTPUT_CACHE_PROTOCOL) {
            //     cout << "SOMEHOW DID NOT FIND ID" << endl;
            // }
            // return allCaches.size();
        } 

        void invalidateOtherallCaches(int coreIndex, string tagBits) {
            for (int i = 0; i < allCaches.size(); i++) {
                if (i == coreIndex) {
                    continue;
                } else {
                    Cache& curCore = allCaches[i];
                    int rowIndex = curCore.findTagRow(tagBits);

                    if (rowIndex != -1) {
                        curCore.resetRow(rowIndex);
                    }
                }
            }
        }

        void processRead(string cacheID, string tagBits) {
            // Step 1: Find the specific cache making the request
            int coreIndex = findCache(cacheID);
            Cache& requestingCache = allCaches[coreIndex];

            // Step 2: Check if the tag/Row exists in the requesting cache
            int rowIndex = requestingCache.findTagRow(tagBits);
            if (rowIndex != -1) {
                // Tag/Row found in cache
                hits++;
                char state = requestingCache.rows[rowIndex].coherencyState;

                if (state != EXCLUSIVE) {
                    broadcasts++; // Broadcast occurs unless the state is Exclusive (E) per Nader
                }
                requestingCache.adjustLRU(rowIndex);
                
                return;
            }

            // By this point, read miss
            misses++;
            broadcasts++; // Broadcast occurs on miss (since Row is not in E state in requesting cache)

            // Step 3: Check other caches for the Row
            bool foundInOtherCache = false;
            for (int i = 0; i < allCaches.size(); ++i) {
                if (i == coreIndex) { 
                    continue;
                } else {
                    Cache& otherCache = allCaches[i];
                    int otherRowIndex = otherCache.findTagRow(tagBits);
                    if (otherRowIndex != -1) {
                        char otherState = otherCache.rows[otherRowIndex].coherencyState;
                        if (otherState == MODIFIED || otherState == OWNED) {
                            // Other cache has modified data; needs to write back
                            writeBacks++;
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

            int targetRow = requestingCache.findBootRow();

            if (requestingCache.writeBackRow(targetRow)) {
                writeBacks++;
            }

            // Set state based on whether other caches have the Row
            if (foundInOtherCache) {
                // Row is shared among caches
                requestingCache.setRow(0, tagBits, SHARED, targetRow);
                transfers++;
            } else
                requestingCache.setRow(0, tagBits, EXCLUSIVE, targetRow); // Row is exclusive to requesting cache
        }

        void processWrite(string cacheID, string tagBits) {
            // Step 1: Find the specific cache making the request
            int coreIndex = findCache(cacheID);
            Cache& requestingCache = allCaches[coreIndex];

            // Step 2: Check if the tag/Row exists in the requesting cache
            int rowIndex = requestingCache.findTagRow(tagBits);
            if (rowIndex != -1) {
                // Tag/Row found in cache
                char state = requestingCache.rows[rowIndex].coherencyState;
                hits++;
                if (state != EXCLUSIVE) {
                    broadcasts++; // Broadcast occurs unless the state is Exclusive (E)
                }
                // Update state
                // invalidateOtherallCaches(rowIndex, tagBits);

                for (int i = 0; i < allCaches.size(); i++) { //fuck over all other caches
                    if (i == rowIndex) {
                        continue;
                    } else {
                        Cache& curCore = allCaches[i];
                        int found = curCore.findTagRow(tagBits);

                        if (found != -1) {
                            curCore.resetRow(found);
                        }
                    }
                }

                requestingCache.rows[rowIndex].coherencyState = MODIFIED;
                requestingCache.adjustLRU(rowIndex);
                return;
            }

            // By this point, read miss
            misses++;
            broadcasts++; // Broadcast occurs on miss (since Row is not in E state in requesting cache)

            // Step 3: Invalidate other caches and check for writeBacks
            bool foundInOtherCache = false;
            for (int i = 0; i < allCaches.size(); ++i) {
                if (i == coreIndex) { 
                    continue;
                } else {

                    Cache& otherCache = allCaches[i];
                    int otherRowIndex = otherCache.findTagRow(tagBits);
                    if (otherRowIndex != -1) {
                        char otherState = otherCache.rows[otherRowIndex].coherencyState;
                        if (otherState == MODIFIED || otherState == OWNED) {
                            writeBacks++; // Other cache has modified data; needs to write back
                        }
                        // Invalidate the Row in other caches
                        otherCache.resetRow(otherRowIndex);
                        foundInOtherCache = true;
                    }
                }
            }

            int targetRow = requestingCache.findBootRow();

            if (requestingCache.writeBackRow(targetRow)) {
                writeBacks++;
            }

            requestingCache.setRow(1, tagBits, MODIFIED, targetRow); // Set state to Modified (M) since we're writing
        }   

        void printCaches() {
            if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                for (int i = 0; i < allCaches.size(); i++) {
                    allCaches[i].printCache();
                }
            }
        }
};
