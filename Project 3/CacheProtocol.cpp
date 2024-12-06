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

        void readInstruction(string cacheID, string tagBits) {  
            int cacheRqIndex = findCache(cacheID);
            Cache& cacheRq = allCaches[cacheRqIndex];

            //check if exists in the requesting cache
            int indexOfRow = cacheRq.findTagRow(tagBits);
            if (indexOfRow == -1) {
                // cout << "FIRST READ TRIGGER" << endl;

                // Cache miss handling
                handleCacheMiss(cacheRq, tagBits);
                bool isInDiffCache = checkDiffCaches(tagBits, cacheRqIndex); //check other caches
                int targetCacheRow = cacheRq.findBootRow();

                handleCacheWriteBack(cacheRq, targetCacheRow); //write if needed

                handleCacheRowSetting(cacheRq, tagBits, isInDiffCache, targetCacheRow); //finish off
            } else {
                // cout << "SECOND READ TRIGGER" << endl;
                 
                handleCacheHit(cacheRq, indexOfRow);
                return;
            }

            if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                cout << "SHOULD NEVER GET HERE!!!" << endl;
                printCaches();
            }
        }

        void writeInstruction(string cacheID, string tagBits) {
            int cacheRqIndex = findCache(cacheID);
            Cache& cacheRq = allCaches[cacheRqIndex];
            int indexOfRow = cacheRq.findTagRow(tagBits);

            if (indexOfRow == -1) {
                if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                    cout << "SECOND WRITE TRIGGER" << endl;
                    cout << hits << endl;
                    cout << misses << endl;
                    cout << writeBacks << endl;
                    cout << broadcasts << endl;
                    cout << transfers << endl;
                }
                
                // Cache miss handling
                handleCacheMiss(cacheRq, tagBits);
                bool isInDiffCache = false;

                for (int i = 0; i < allCaches.size(); ++i) {
                    if (i == cacheRqIndex) { 
                        continue;
                    } else {
                        Cache& otherCache = allCaches[i];
                        int otherRowIndex = otherCache.findTagRow(tagBits);

                        if (otherRowIndex != -1) {
                            char otherState = otherCache.rows[otherRowIndex].coherencyState;
                            if (otherState == OWNED || otherState == MODIFIED) {
                                // cout << "BOTH MODIFIED AND OWNED" << endl;
                                writeBacks++; 
                                // cout << "NUM WRITEBACKS: " << writeBacks << endl;
                            }
                            otherCache.resetRow(otherRowIndex); //invalidate the row
                            isInDiffCache = true;
                        }
                    }
                }

                int targetCacheRow = cacheRq.findBootRow();
                handleCacheWriteBack(cacheRq, targetCacheRow);
                cacheRq.setRow(1, tagBits, MODIFIED, targetCacheRow); //must change to this since we actually modify
                printCaches();
            } else {
                if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                    cout << "FIRST WRITE TRIGGER" << endl;
                    cout << hits << endl;
                    cout << misses << endl;
                    cout << writeBacks << endl;
                    cout << broadcasts << endl;
                    cout << transfers << endl;
                }

                char state = cacheRq.rows[indexOfRow].coherencyState;
                hits++;
                if (state != EXCLUSIVE) {
                    broadcasts++; // Broadcast occurs unless the state is Exclusive (E)
                }

                // Invalidate other caches and update the state
                for (int i = 0; i < allCaches.size(); i++) { 
                    if (i != indexOfRow) {
                        Cache& curCore = allCaches[i];
                        int found = curCore.findTagRow(tagBits);

                        if (found != -1) {
                            curCore.resetRow(found);
                        }
                    }
                }

                cacheRq.rows[indexOfRow].coherencyState = MODIFIED;
                cacheRq.adjustLRU(indexOfRow);
                printCaches();
                return;
            }

            if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                cout << "SHOULD NEVER GET HERE!!!" << endl;
                printCaches();
            }
        }

        void handleCacheHit(Cache& cacheRq, int indexOfRow) { //Row found in cache, increment hits
            hits++;            
            if (cacheRq.rows[indexOfRow].coherencyState != EXCLUSIVE) { //BROADCAST HAPPENS UNELSS STATE IS EXCLUSIVE
                broadcasts++;
            }

            if (TRACE_OUTPUT_CACHE_PROTOCOL == true) {
                cout << "cache hit at row index: " << indexOfRow << endl;
            } 

            cacheRq.adjustLRU(indexOfRow);

            if (TRACE_OUTPUT_CACHE_PROTOCOL == true) {
                cout << hits << endl;
                cout << misses << endl;
                cout << writeBacks << endl;
                cout << broadcasts << endl;
                cout << transfers << endl;
                printCaches();
            } 

            if (TRACE_OUTPUT_CACHE_PROTOCOL == true) {
                cout << "cache hit at row index: " << indexOfRow << endl;
            } 
        }

        void handleCacheMiss(Cache& cacheRq, string tagBits) { //case when the requested tag is not found in the cache
            misses++;
            broadcasts++; //broadcast occurs on miss since Row is not in Exclusive state in requesting cache
            // cout << "cache miss for tag: " << tagBits << endl;
            // if (TRACE_OUTPUT_CACHE_PROTOCOL == true) {
            //     cout << hits << endl;
            //     cout << misses << endl;
            //     cout << writeBacks << endl;
            //     cout << broadcasts << endl;
            //     cout << transfers << endl;
            // } 
        }

        bool checkDiffCaches(string tagBits, int cacheRqIndex) {
            for (int i = 0; i < allCaches.size(); ++i) {
                Cache& diffCache = allCaches[i];
                int diffRowIndex = diffCache.findTagRow(tagBits);
                
                // if (i == cacheIndex) continue;
                if (i != cacheRqIndex) { 
                    if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                        cout << "INFERNO TRIGGER" << endl;
                    }

                    if (diffRowIndex != -1) {
                        handleDiffCacheRowState(diffCache, diffRowIndex, diffCache.rows[diffRowIndex].coherencyState);
                        printCaches();
                        return true;
                    }
                    // else {
                }

                // cout << "otherwise continue" << endl;
            }
            return false;
        }

        void handleDiffCacheRowState(Cache& diffCache, int diffRowIndex, char diffState) {
            // if (otherState == MODIFIED) {
            //     if (otherState == OWNED) {
            //     otherCache.rows[otherRowIndex].coherencyState = SHARED; 

            if (diffState == OWNED || diffState == MODIFIED) {
                // cout << "BOTH MODIFIED AND OWNED" << endl;
                writeBacks++; //other cache has modified data, must write back
                // cout << "NUM WRITEBACKS: " << writeBacks << endl;
                diffCache.rows[diffRowIndex].coherencyState = SHARED;
                diffCache.rows[diffRowIndex].dirtyBit = 0;
            }

            if (diffState == EXCLUSIVE) {
                // cout << "EXCLUSIVE TRIGGER" << endl;
                diffCache.rows[diffRowIndex].coherencyState = FORWARD; //upgrade from exclusive
            }
        }

        void handleCacheWriteBack(Cache& cacheRq, int targetCacheRow) {
            if (cacheRq.writeBackRow(targetCacheRow)) {
                // cout << "write-back at row index: " << targetRow << endl;
                writeBacks++;
                if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                    cout << "num writeBacks: " << writeBacks << endl;
                }
            }
            
        }

        void handleCacheRowSetting(Cache& cacheRq, string tagBits, bool isInDiffCache, int targetCacheRow) {
            if (isInDiffCache) {
                cacheRq.setRow(0, tagBits, SHARED, targetCacheRow); //row is shared among caches
                // cout << "SHARED=========================================" << endl;
                transfers++;
                return;
            } 

            cacheRq.setRow(0, tagBits, EXCLUSIVE, targetCacheRow); //row is exclusive to requesting cache
            // cout << "EXCLUSIVE=========================================" << endl;

            // if (!foundInOtherCache) {  
            //     requestingCache.setRow(1, tagBits, SHARED, targetRow); 
            //     transfers--; 
            // } else {
            //     requestingCache.setRow(1, tagBits, EXCLUSIVE, targetRow);
            //     transfers++; 
            // }
        }

        void printCaches() {
            if (TRACE_OUTPUT_CACHE_PROTOCOL) {
                for (int i = 0; i < allCaches.size(); i++) {
                    allCaches[i].printCache();
                }
            }
        }
};
