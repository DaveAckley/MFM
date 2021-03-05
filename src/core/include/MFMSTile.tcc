/* -*- C++ -*- */
#include "MDist.h"

#include "Util.h"

namespace MFM
{

  template <class EC>
  MFMSTile<EC>::~MFMSTile() {/* defined to avoid inline error */}

  template <class EC>
  bool MFMSTile<EC>::IsCacheSitePossibleEventCenter(const SPoint & location) const
  {
    MFM_API_ASSERT_ARG(this->IsInCache(location));
    THREEDIR cnCacheDirs;
    u32 count = this->CacheAt(location, cnCacheDirs, YESCHKCONNECT);
    bool isInANeighborsShared = false;
    for(u32 i = 0; i < count; i++)
      {
	Dir dir = cnCacheDirs[i];
	const CacheProcessor<EC>& cp = this->GetCacheProcessor(dir);
	SPoint remoteloc = cp.LocalToRemote(location);
	isInANeighborsShared |= ! this->IsInCache(remoteloc); //all tiles same size
      }
    return isInANeighborsShared;
  }

  template <class EC>
  bool MFMSTile<EC>::IsConnected(Dir dir) const
  {
    const CacheProcessor<EC> & cxn = this->GetCacheProcessor(dir);
    //return cxn.IsConnected();
    return !cxn.IsUnclaimed() && cxn.IsConnected();
  }


} /* namespace MFM */
