#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include "Cache.h"

class CacheManager {
  public:
    CacheManager(vector<string> coreIDs);
    vector<Cache> getCores() const { return cores; }
    int getCacheHits() const { return cacheHits; }
    int getCacheMisses() const { return cacheMisses; }
    int getWritebacks() const { return writebacks; }
    int getBroadcasts() const { return broadcasts; }
    int getTransfers() const { return cacheToCacheTransfers; }
    
    int findCoreIndex(string coreID) const;
    void invalidateOtherCores(int coreIndex, string tag);
    void transitionState(int coreIndex, string tag, char newState);
    void processRead(string coreID, string tag);
    void processWrite(string coreID, string tag);
    
    // Final behavior
    void printStatistics() const {
      cout << cacheHits << endl;
      cout << cacheMisses << endl;
      cout << writebacks << endl;
      cout << broadcasts << endl;
      cout << cacheToCacheTransfers;
    };
    
  private:
    vector<Cache> cores; // max 4 cores
    int cacheHits;
    int cacheMisses;
    int writebacks;
    int broadcasts;
    int cacheToCacheTransfers;
};

#endif